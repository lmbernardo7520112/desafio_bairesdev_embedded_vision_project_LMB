#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include "lwip/ip4_addr.h"  // Adicionado para ip4addr_ntoa
#include "inference.h"

// Defina a senha correta para a rede "moto" aqui
#define WIFI_PASSWORD "password"
#define UDP_PORT 5005
#define MAX_IMAGE_SIZE (48 * 48 * 1) // kNumRows * kNumCols * kNumChannels
#define CHUNK_PAYLOAD_MAX 1000
#define ACK_TIMEOUT_MS 2000

// Estrutura para armazenar informações de redes com SSID "moto"
typedef struct {
    char ssid[33];
    int rssi;
    int channel;
    uint8_t bssid[6];
    uint auth_mode;
} wifi_network_t;

// Array para armazenar redes encontradas com "moto" no SSID
#define MAX_MOTO_NETWORKS 10
wifi_network_t moto_networks[MAX_MOTO_NETWORKS];
int moto_network_count = 0;
bool connected = false;
static char connected_ssid[33] = "";

// Buffer para armazenar a imagem recebida
static uint8_t image_buffer[MAX_IMAGE_SIZE];
static size_t image_buffer_len = 0;
static uint16_t expected_total_chunks = 0;
static uint16_t received_chunks = 0;

// Função para mapear auth_mode
static uint32_t get_auth_mode(uint auth_mode) {
    printf("Mapping auth_mode: %u\n", auth_mode);
    switch (auth_mode) {
        case 0: return CYW43_AUTH_OPEN;
        case 1: return CYW43_AUTH_WPA_TKIP_PSK;
        case 2: return CYW43_AUTH_WPA2_AES_PSK;
        case 5:
            #ifdef CYW43_AUTH_WPA3_SAE
                return CYW43_AUTH_WPA3_SAE;
            #else
                return CYW43_AUTH_WPA2_AES_PSK;
            #endif
        default: return CYW43_AUTH_WPA2_AES_PSK;
    }
}

// Função para conectar à rede Wi-Fi
bool connect_to_wifi(const char *ssid, const char *password, uint auth_mode) {
    printf("\nTentando conectar à rede %s com auth_mode %u...\n", ssid, auth_mode);
    uint32_t sdk_auth_mode = get_auth_mode(auth_mode);
    int err = cyw43_arch_wifi_connect_timeout_ms(ssid, password, sdk_auth_mode, 10000);
    if (err == 0) {
        printf("Conexão bem-sucedida à rede %s!\n", ssid);
        strncpy(connected_ssid, ssid, 32);
        connected_ssid[32] = '\0';
        connected = true;

        // Adicionado: Imprimir o IP obtido
        const ip_addr_t *ip = &cyw43_state.netif[CYW43_ITF_STA].ip_addr;
        printf("Endereço IP obtido: %s\n", ip4addr_ntoa(ip));

        return true;
    } else {
        printf("Falha na conexão à rede %s: erro %d\n", ssid, err);
        return false;
    }
}

// Função para selecionar a melhor rede
void select_best_protocol(void) {
    if (moto_network_count == 0) {
        printf("Nenhuma rede com SSID contendo 'moto' encontrada.\n");
        return;
    }

    int best_index = 0;
    int best_rssi = -1000;
    int best_auth_score = 0;

    for (int i = 0; i < moto_network_count; i++) {
        int auth_score = (moto_networks[i].auth_mode == 0) ? 1 : (moto_networks[i].auth_mode == 5) ? 5 : 4;
        if (auth_score > best_auth_score || (auth_score == best_auth_score && moto_networks[i].rssi > best_rssi)) {
            best_index = i;
            best_auth_score = auth_score;
            best_rssi = moto_networks[i].rssi;
        }
    }

    printf("\nMelhor rede com 'moto' encontrada: %s (RSSI: %d dBm, Canal: %d)\n",
           moto_networks[best_index].ssid, moto_networks[best_index].rssi, moto_networks[best_index].channel);

    if (!connected) {
        connected = connect_to_wifi(moto_networks[best_index].ssid, WIFI_PASSWORD, moto_networks[best_index].auth_mode);
    }
}

// Callback de resultados do scan Wi-Fi
static int scan_result(void *env, const cyw43_ev_scan_result_t *result) {
    if (result && strstr((const char*)result->ssid, "moto") && moto_network_count < MAX_MOTO_NETWORKS) {
        strncpy(moto_networks[moto_network_count].ssid, (const char*)result->ssid, 32);
        moto_networks[moto_network_count].ssid[32] = '\0';
        moto_networks[moto_network_count].rssi = result->rssi;
        moto_networks[moto_network_count].channel = result->channel;
        memcpy(moto_networks[moto_network_count].bssid, result->bssid, 6);
        moto_networks[moto_network_count].auth_mode = result->auth_mode;
        moto_network_count++;
        printf("Rede com 'moto' encontrada: %s\n", result->ssid);
    }
    return 0;
}

// Worker para scan Wi-Fi
static void scan_worker_fn(async_context_t *context, async_at_time_worker_t *worker) {
    cyw43_wifi_scan_options_t scan_options = {0};
    int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_result);
    if (err == 0) {
        bool *scan_started = (bool *)worker->user_data;
        *scan_started = true;
        printf("\nPerforming wifi scan\n");
    } else {
        printf("Failed to start scan: %d\n", err);
    }
}

// Callback para recepção de pacotes UDP
static void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if (!p) return;

    // Processar pacote (cabeçalho: idx(2 bytes) + total(2 bytes))
    if (p->len < 4) {
        printf("[UDP] Pacote muito pequeno: %d bytes\n", p->len);
        pbuf_free(p);
        return;
    }

    uint8_t *data = (uint8_t *)p->payload;
    // Deserializar idx e total_chunks (big-endian, 2 bytes cada)
    uint16_t idx = (data[0] << 8) | data[1];
    uint16_t total_chunks = (data[2] << 8) | data[3];

    if (received_chunks == 0) {
        expected_total_chunks = total_chunks;
        image_buffer_len = 0;
        printf("[UDP] Iniciando nova transferência: %u chunks esperados\n", total_chunks);
    }

    if (idx >= expected_total_chunks) {
        printf("[UDP] Índice inválido: %u >= %u\n", idx, expected_total_chunks);
        pbuf_free(p);
        return;
    }

    // Copiar payload para o buffer
    size_t payload_len = p->len - 4;
    size_t offset = idx * CHUNK_PAYLOAD_MAX;
    if (offset + payload_len <= MAX_IMAGE_SIZE) {
        memcpy(image_buffer + offset, data + 4, payload_len);
        image_buffer_len = offset + payload_len > image_buffer_len ? offset + payload_len : image_buffer_len;
        received_chunks++;
        printf("[UDP] Recebido chunk %u/%u (%u bytes)\n", idx + 1, total_chunks, payload_len);
    } else {
        printf("[UDP] Buffer overflow: offset=%u, payload_len=%u\n", offset, payload_len);
        pbuf_free(p);
        return;
    }

    // Enviar ACK
    struct pbuf *ack = pbuf_alloc(PBUF_TRANSPORT, 5, PBUF_RAM);
    if (ack) {
        memcpy(ack->payload, "ACK", 3);
        // Serializar idx (big-endian, 2 bytes)
        ((uint8_t *)ack->payload)[3] = (idx >> 8) & 0xFF;
        ((uint8_t *)ack->payload)[4] = idx & 0xFF;
        udp_sendto(pcb, ack, addr, port);
        pbuf_free(ack);
        printf("[UDP] Enviado ACK para chunk %u\n", idx);
    }

    // Verificar se todos os chunks foram recebidos
    if (received_chunks == expected_total_chunks) {
        printf("[UDP] Imagem completa recebida: %u bytes\n", image_buffer_len);
        run_inference_from_buffer(image_buffer, image_buffer_len, 48, 48); // Ajuste dimensões
        received_chunks = 0;
        expected_total_chunks = 0;

        // Enviar TRANSFER_DONE
        struct pbuf *done = pbuf_alloc(PBUF_TRANSPORT, strlen("TRANSFER_DONE"), PBUF_RAM);
        if (done) {
            memcpy(done->payload, "TRANSFER_DONE", strlen("TRANSFER_DONE"));
            udp_sendto(pcb, done, addr, port);
            pbuf_free(done);
            printf("[UDP] Enviado TRANSFER_DONE\n");
        }
    }

    pbuf_free(p);
}

int main(void) {
    stdio_init_all();
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    bool scan_started = false;
    async_at_time_worker_t scan_worker = {
        .do_work = scan_worker_fn,
        .user_data = &scan_started
    };
    async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &scan_worker, 0);

    // Configurar UDP
    struct udp_pcb *udp = udp_new();
    if (!udp) {
        printf("Falha ao criar PCB UDP\n");
        cyw43_arch_deinit();
        return 1;
    }
    if (udp_bind(udp, IP_ADDR_ANY, UDP_PORT) != ERR_OK) {
        printf("Falha ao bindar UDP no porto %d\n", UDP_PORT);
        udp_remove(udp);
        cyw43_arch_deinit();
        return 1;
    }
    udp_recv(udp, udp_receive_callback, NULL);
    printf("[UDP] Aguardando pacotes na porta %d\n", UDP_PORT);

    uint32_t last_check = 0;
    while (true) {
        if (connected) {
            uint32_t now = to_ms_since_boot(get_absolute_time());
            if (now - last_check >= 5000) {
                int status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
                connected = (status >= 0);
                printf("Status da conexão: %s (status: %d)\n", connected ? "Conectado" : "Desconectado", status);
                last_check = now;
            }
        } else if (!cyw43_wifi_scan_active(&cyw43_state) && scan_started) {
            scan_started = false;
            select_best_protocol();
            async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &scan_worker, 10000);
        }

        cyw43_arch_poll();
        sleep_ms(10);
    }

    udp_remove(udp);
    cyw43_arch_deinit();
    return 0;
}