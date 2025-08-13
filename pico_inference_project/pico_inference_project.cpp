// pico_inference_project.cpp
// Recebe imagem via UDP (raw lwIP API). Protocol: header 4 bytes (idx(2), total(2)) + payload
// Envia ACK por chunk: "ACK"+idx(2). Quando todos os chunks recebidos, chama run_inference_from_buffer().

// pico_inference_project.cpp - versão com logs de depuração de rede aprimorados
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"

#include "model_settings.h"
#include "inference.h"
#include "wifi_config.h" // Certifique-se que WIFI_SSID e WIFI_PASS estão aqui

#define UDP_PORT 5005
#define CHUNK_PAYLOAD_MAX 1000
#define IMAGE_BUFFER_MAX (64*1024)

// ... (o restante do código de recebimento UDP permanece o mesmo) ...
static uint8_t image_buffer[IMAGE_BUFFER_MAX];
static uint16_t expected_total_chunks = 0;
static uint8_t *chunks_bitmap = NULL;
static uint32_t bytes_received = 0;
static struct udp_pcb *udp_pcb_global = NULL;

static void send_ack(struct udp_pcb *pcb, const ip_addr_t *addr, u16_t port, uint16_t idx) {
    uint8_t buf[5];
    memcpy(buf, "ACK", 3);
    buf[3] = (uint8_t)((idx >> 8) & 0xFF);
    buf[4] = (uint8_t)(idx & 0xFF);
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(buf), PBUF_RAM);
    if (!p) return;
    memcpy(p->payload, buf, sizeof(buf));
    udp_sendto(pcb, p, addr, port);
    pbuf_free(p);
}

static inline void set_chunk_bit(uint16_t idx) {
    if (!chunks_bitmap) return;
    chunks_bitmap[idx / 8] |= (1u << (idx % 8));
}
static inline bool get_chunk_bit(uint16_t idx) {
    if (!chunks_bitmap) return false;
    return (chunks_bitmap[idx / 8] & (1u << (idx % 8))) != 0;
}

static void udp_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                        const ip_addr_t *addr, u16_t port) {
    if (!p || !pcb || !addr) {
        if (p) pbuf_free(p);
        return;
    }

    if (p->tot_len < 4) {
        pbuf_free(p);
        return;
    }

    uint8_t header[4];
    pbuf_copy_partial(p, header, 4, 0);
    uint16_t idx = (uint16_t)((header[0] << 8) | header[1]);
    uint16_t total = (uint16_t)((header[2] << 8) | header[3]);

    if (bytes_received == 0 && idx == 0) {
        expected_total_chunks = total;
        if (chunks_bitmap) { free(chunks_bitmap); chunks_bitmap = NULL; }
        if (expected_total_chunks > 0) {
            size_t bsize = (expected_total_chunks + 7) / 8;
            chunks_bitmap = (uint8_t*)malloc(bsize);
            if (chunks_bitmap) memset(chunks_bitmap, 0, bsize);
            else printf("[PICO] Aviso: falha ao alocar chunks_bitmap (%u bytes)\n", (unsigned)bsize);
        }
        bytes_received = 0;
        printf("[PICO] Nova transferência: %u chunks esperados\n", expected_total_chunks);
    }

    size_t payload_len = p->tot_len - 4;
    uint32_t offset = (uint32_t)idx * CHUNK_PAYLOAD_MAX;
    if (offset + payload_len > IMAGE_BUFFER_MAX) {
        printf("[PICO] Erro: chunk fora do buffer idx=%u len=%u (offset %u, max %u)\n",
               idx, (unsigned)payload_len, (unsigned)offset, (unsigned)IMAGE_BUFFER_MAX);
        pbuf_free(p);
        return;
    }

    size_t copied = pbuf_copy_partial(p, image_buffer + offset, (u16_t)payload_len, 4);
    if (copied != payload_len) {
        printf("[PICO] Aviso: copiados %u/%u bytes do chunk %u\n", (unsigned)copied, (unsigned)payload_len, idx);
    }

    if (!get_chunk_bit(idx)) {
        set_chunk_bit(idx);
        bytes_received += (uint32_t)copied;
    }

    send_ack(pcb, addr, port, idx);

    bool all = true;
    if (expected_total_chunks == 0) all = false;
    else {
        for (uint16_t i = 0; i < expected_total_chunks; ++i) {
            if (!get_chunk_bit(i)) { all = false; break; }
        }
    }

    if (all) {
        printf("[PICO] Transferencia completa: chunks=%u bytes=%u. Rodando inferencia...\n",
               expected_total_chunks, (unsigned)bytes_received);

        run_inference_from_buffer(image_buffer, bytes_received, kNumCols, kNumRows);

        const char done[] = "TRANSFER_DONE";
        struct pbuf *pdone = pbuf_alloc(PBUF_TRANSPORT, sizeof(done) - 1, PBUF_RAM);
        if (pdone) {
            memcpy(pdone->payload, done, sizeof(done) - 1);
            udp_sendto(pcb, pdone, addr, port);
            pbuf_free(pdone);
        }

        if (chunks_bitmap) { free(chunks_bitmap); chunks_bitmap = NULL; }
        expected_total_chunks = 0;
        bytes_received = 0;
    }

    pbuf_free(p);
}

static bool init_udp_server(uint16_t port) {
    udp_pcb_global = udp_new();
    if (!udp_pcb_global) {
        printf("Erro: udp_new()\n");
        return false;
    }
    if (udp_bind(udp_pcb_global, IP_ADDR_ANY, port) != ERR_OK) {
        printf("Erro: udp_bind()\n");
        udp_remove(udp_pcb_global);
        udp_pcb_global = NULL;
        return false;
    }
    udp_recv(udp_pcb_global, udp_recv_cb, NULL);
    printf("UDP server pronto na porta %u\n", port);
    return true;
}

static void print_string_bytes_hex(const char *label, const char *s) {
    if (!s) {
        printf("%s: <null>\n", label);
        return;
    }
    size_t len = strlen(s);
    printf("%s (len=%u) bytes:", label, (unsigned)len);
    for (size_t i = 0; i < len; ++i) {
        printf(" %02X", (unsigned char)s[i]);
    }
    printf("\n");
}

// NOVO: Função para imprimir o estado detalhado da interface de rede
static void dump_cyw43_state() {
    struct netif *netif = netif_default;
    if (!netif) {
        printf("[DEBUG] netif_default não está disponível.\n");
        return;
    }
    
    uint8_t mac[6];
    cyw43_hal_get_mac(0, mac);

    printf("\n--- [DEBUG] Estado da Interface de Rede ---\n");
    printf("Endereço MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("Status do Link: %s\n", cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP ? "UP" : "DOWN");
    printf("IP: %s\n", ipaddr_ntoa(netif_ip_addr4(netif)));
    printf("Gateway: %s\n", ipaddr_ntoa(netif_ip_gw4(netif)));
    printf("Netmask: %s\n", ipaddr_ntoa(netif_ip_netmask4(netif)));
    printf("------------------------------------------\n\n");
}

// MODIFICADO: Função de conexão com mais logs
// MODIFICADO: Função de conexão com logs e constantes de erro corrigidas
static int connect_with_retries(const char *ssid, const char *pass, int max_attempts) {
    int ret = -1;
    for (int attempt = 1; attempt <= max_attempts; ++attempt) {
        printf("Conectando a SSID '%s'... tentativa %d/%d\n", ssid, attempt, max_attempts);
        
        // Tenta a conexão com um timeout de 20 segundos
        ret = cyw43_arch_wifi_connect_timeout_ms(
            ssid,
            pass,
            CYW43_AUTH_WPA2_AES_PSK, // Usar WPA2-AES é uma boa prática
            20000
        );
        
        if (ret == 0) {
            printf("Conectado com sucesso na tentativa %d\n", attempt);
            return 0; // Sucesso
        }

        // CORRIGIDO: Switch-case ajustado para usar constantes válidas do SDK
        printf("Falha ao conectar WiFi (ret=%d). ", ret);
        switch(ret) {
            case -1: /* CYW43_LINK_FAIL */    printf("Causa: Falha de Link (SSID/Senha incorretos?)\n"); break;
            case -2: /* CYW43_LINK_NONET */   printf("Causa: Rede não encontrada (Fora de alcance?)\n"); break;
            case -3: /* CYW43_LINK_BADAUTH */ printf("Causa: Autenticação falhou (Senha errada? Tipo de segurança?)\n"); break;
            default:                          printf("Causa: Erro desconhecido ou Timeout (Sinal fraco? Roteador não responde?)\n"); break;
        }

        if (attempt == 1) { // Mostrar detalhes apenas na primeira tentativa
            print_string_bytes_hex("SSID", ssid);
            print_string_bytes_hex("PASS", pass);
            printf("Sugestões: 1) Verifique se o hotspot está em 2.4 GHz.\n"
                   "2) Certifique-se de que o SSID e a senha não contenham caracteres especiais invisíveis.\n"
                   "3) Configure segurança WPA2-PSK (não WPA3). 4) Aumente limite de clientes do hotspot.\n");
        }
        
        dump_cyw43_state();

        sleep_ms(2000); // Espera 2 segundos antes de tentar novamente
    }
    return ret; // Retorna o último erro após todas as tentativas
}

int main(void) {
    stdio_init_all();
    // Espera o USB ser conectado para não perder os logs iniciais
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    sleep_ms(500); // Um tempo extra para o terminal serial estabilizar

    printf("\n--- Pico W UDP Image Receiver (v2 com Debug Logs) ---\n");

    if (cyw43_arch_init()) {
        printf("Falha crítica: cyw43_arch_init() falhou\n");
        return -1;
    }
    
    // NOVO: Imprime o estado inicial
    printf("cyw43 inicializado.\n");
    dump_cyw43_state();

    cyw43_arch_enable_sta_mode();
    printf("Modo Station (STA) ativado.\n");

    int conn_ret = connect_with_retries(WIFI_SSID, WIFI_PASS, 3);
    if (conn_ret != 0) {
        printf("\nConexão WiFi falhou após todas as tentativas. Erro final: %d\n", conn_ret);
        print_string_bytes_hex("SSID_final", WIFI_SSID);
        print_string_bytes_hex("PASS_final", WIFI_PASS);
        cyw43_arch_deinit();
        return -1;
    }

    // NOVO: Imprime o estado final após conexão bem-sucedida
    printf("\nConexão WiFi estabelecida com sucesso!\n");
    dump_cyw43_state();

    if (!init_udp_server(UDP_PORT)) {
        cyw43_arch_deinit();
        return -1;
    }

    while (true) {
        cyw43_arch_poll();
        sleep_ms(10);
    }
    
    cyw43_arch_deinit();
    return 0;
}
