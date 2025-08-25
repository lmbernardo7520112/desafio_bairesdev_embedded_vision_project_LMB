#include <stdio.h>
#include <string.h>
// #include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/async_context.h"
#include "pico/cyw43_arch.h"
#include "inference.h"

// Defina a senha correta para a rede "moto" aqui
#define WIFI_PASSWORD "password"

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
bool connected = false; // Flag para verificar se está conectado
static char connected_ssid[33] = ""; // Armazena o SSID conectado

// Função para mapear auth_mode para constantes do SDK
static uint32_t get_auth_mode(uint auth_mode) {
    printf("Mapping auth_mode: %u\n", auth_mode);
    switch (auth_mode) {
        case 0: // Aberto
            printf("Auth mode mapped to CYW43_AUTH_OPEN (0)\n");
            return CYW43_AUTH_OPEN;
        case 1: // WPA-PSK
            printf("Auth mode mapped to CYW43_AUTH_WPA_TKIP_PSK (1)\n");
            return CYW43_AUTH_WPA_TKIP_PSK;
        case 2: // WPA2-PSK
            printf("Auth mode mapped to CYW43_AUTH_WPA2_AES_PSK (3)\n");
            return CYW43_AUTH_WPA2_AES_PSK;
        case 5: // WPA3-SAE or WPA2-PSK fallback
            #ifdef CYW43_AUTH_WPA3_SAE
                printf("Auth mode mapped to CYW43_AUTH_WPA3_SAE\n");
                return CYW43_AUTH_WPA3_SAE;
            #else
                printf("Auth mode mapped to CYW43_AUTH_WPA2_AES_PSK (3) as fallback\n");
                return CYW43_AUTH_WPA2_AES_PSK;
            #endif
        default:
            printf("Unknown auth mode %u, defaulting to CYW43_AUTH_WPA2_AES_PSK (3)\n", auth_mode);
            return CYW43_AUTH_WPA2_AES_PSK;
    }
}

// Função para verificar o status da conexão
bool check_connection_status(void) {
    int status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
    if (status >= 0) {
        printf("Conexão mantida à rede %s (status: %d)\n", connected_ssid, status);
        return true;
    } else {
        printf("Conexão perdida (status: %d)\n", status);
        connected = false; // Marca como desconectado para tentar reconectar
        return false;
    }
}

// Função para tentar conectar à rede Wi-Fi
bool connect_to_wifi(const char *ssid, const char *password, uint auth_mode) {
    printf("\nTentando conectar à rede %s com auth_mode %u...\n", ssid, auth_mode);
    uint32_t sdk_auth_mode = get_auth_mode(auth_mode);
    if (sdk_auth_mode > 0xFFFF) { // Workaround para auth mode inválido
        printf("Invalid auth mode %u, forcing CYW43_AUTH_WPA2_AES_PSK (3)\n", sdk_auth_mode);
        sdk_auth_mode = CYW43_AUTH_WPA2_AES_PSK;
    }
    printf("SDK auth mode: %u\n", sdk_auth_mode);
    printf("CYW43 state before connect: %08x\n", cyw43_state.netif[CYW43_ITF_STA].state);
    int err = cyw43_arch_wifi_connect_timeout_ms(ssid, password, sdk_auth_mode, 10000);
    if (err == 0) {
        printf("Conexão bem-sucedida à rede %s!\n", ssid);
        strncpy(connected_ssid, ssid, 32);
        connected_ssid[32] = '\0';
        connected = true;
        printf("CYW43 state after connect: %08x\n", cyw43_state.netif[CYW43_ITF_STA].state);
        return true;
    } else {
        printf("Falha na conexão à rede %s: erro %d\n", ssid, err);
        printf("SSID (len=%u) bytes:", (unsigned)strlen(ssid));
        for (size_t i = 0; i < strlen(ssid); i++) printf(" %02X", (unsigned char)ssid[i]);
        printf("\nPASS (len=%u) bytes:", (unsigned)strlen(password));
        for (size_t i = 0; i < strlen(password); i++) printf(" %02X", (unsigned char)password[i]);
        printf("\nCYW43 state after fail: %08x\n", cyw43_state.netif[CYW43_ITF_STA].state);
        return false;
    }
}

// Função para determinar o melhor protocolo e conectar
void select_best_protocol(void) {
    if (moto_network_count == 0) {
        printf("Nenhuma rede com SSID contendo 'moto' encontrada.\n");
        return;
    }

    int best_index = 0;
    int best_rssi = -1000; // Valor inicial baixo para comparação
    int best_auth_score = 0; // Pontuação para o modo de autenticação

    for (int i = 0; i < moto_network_count; i++) {
        int auth_score = 0;
        switch (moto_networks[i].auth_mode) {
            case 0: // Aberto
                auth_score = 1; // Menor prioridade
                break;
            case 5: // WPA3-SAE
                auth_score = 5; // Melhor segurança
                break;
            default: // WPA-PSK, WPA2-PSK, ou outros
                auth_score = 4; // Boa segurança
                break;
        }

        // Comparar com base no auth_score e RSSI
        if (auth_score > best_auth_score || 
            (auth_score == best_auth_score && moto_networks[i].rssi > best_rssi)) {
            best_index = i;
            best_auth_score = auth_score;
            best_rssi = moto_networks[i].rssi;
        }
    }

    // Exibir detalhes da melhor rede
    printf("\nMelhor rede com 'moto' encontrada:\n");
    printf("SSID: %s\n", moto_networks[best_index].ssid);
    printf("RSSI: %d dBm\n", moto_networks[best_index].rssi);
    printf("Canal: %d\n", moto_networks[best_index].channel);
    printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           moto_networks[best_index].bssid[0], moto_networks[best_index].bssid[1],
           moto_networks[best_index].bssid[2], moto_networks[best_index].bssid[3],
           moto_networks[best_index].bssid[4], moto_networks[best_index].bssid[5]);
    
    // Determinar e exibir o protocolo recomendado
    switch (moto_networks[best_index].auth_mode) {
        case 0:
            printf("Protocolo recomendado: Nenhum (rede aberta, menos segura)\n");
            break;
        case 5:
            #ifdef CYW43_AUTH_WPA3_SAE
                printf("Protocolo recomendado: WPA3-SAE (máxima segurança)\n");
            #else
                printf("Protocolo recomendado: WPA2-PSK (usado como fallback, WPA3 não suportado)\n");
            #endif
            break;
        default:
            printf("Protocolo recomendado: WPA2-PSK (seguro e compatível)\n");
            break;
    }

    // Tentar conectar à melhor rede, se ainda não estiver conectado
    if (!connected) {
        connected = connect_to_wifi(moto_networks[best_index].ssid, WIFI_PASSWORD, moto_networks[best_index].auth_mode);
    }
}

// Callback de resultados do scan Wi-Fi
static int scan_result(void *env, const cyw43_ev_scan_result_t *result) {
    if (result) {
        // Verifica se o SSID contém "moto" (case-insensitive)
        char ssid_lower[33];
        strncpy(ssid_lower, (const char*)result->ssid, 32);
        ssid_lower[32] = '\0';
        for (int i = 0; ssid_lower[i]; i++) {
            if (ssid_lower[i] >= 'A' && ssid_lower[i] <= 'Z') {
                ssid_lower[i] += 32; // Converte para minúscula
            }
        }
        
        if (strstr(ssid_lower, "moto") != NULL && moto_network_count < MAX_MOTO_NETWORKS) {
            // Armazena informações da rede
            strncpy(moto_networks[moto_network_count].ssid, (const char*)result->ssid, 32);
            moto_networks[moto_network_count].ssid[32] = '\0';
            moto_networks[moto_network_count].rssi = result->rssi;
            moto_networks[moto_network_count].channel = result->channel;
            for (int i = 0; i < 6; i++) {
                moto_networks[moto_network_count].bssid[i] = result->bssid[i];
            }
            moto_networks[moto_network_count].auth_mode = result->auth_mode;
            moto_network_count++;
            
            printf("Rede com 'moto' encontrada: %s\n", result->ssid);
        }
        
        // Imprime todos os resultados do scan
        printf("ssid: %-32s rssi: %4d chan: %3d mac: %02x:%02x:%02x:%02x:%02x:%02x sec: %u\n",
               result->ssid, result->rssi, result->channel,
               result->bssid[0], result->bssid[1], result->bssid[2],
               result->bssid[3], result->bssid[4], result->bssid[5],
               result->auth_mode);
    }
    return 0;
}

// Worker que dispara o scan
static void scan_worker_fn(async_context_t *context, async_at_time_worker_t *worker) {
    cyw43_wifi_scan_options_t scan_options = {0}; // scan passivo padrão
    int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_result);
    if (err == 0) {
        bool *scan_started = (bool *)worker->user_data;
        *scan_started = true;
        printf("\nPerforming wifi scan\n");
    } else {
        printf("Failed to start scan: %d\n", err);
    }
}

int main(void) {
    stdio_init_all();
    
    printf("Inicializando CYW43...\n");
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }
    
    printf("CYW43 initial state: %08x\n", cyw43_state.netif[CYW43_ITF_STA].state);
    cyw43_arch_enable_sta_mode();
    printf("Press 'q' to quit\n");
    
    bool scan_started = false;
    async_at_time_worker_t scan_worker = {
        .do_work = scan_worker_fn,
        .user_data = &scan_started
    };
    
    // dispara imediatamente o primeiro scan
    hard_assert(async_context_add_at_time_worker_in_ms(
        cyw43_arch_async_context(), &scan_worker, 0));
    
    bool exit = false;
    uint32_t last_check = 0; // Última verificação de status
    while (!exit) {
        int key = getchar_timeout_us(0);
        if (key == 'q' || key == 'Q') {
            exit = true;
        }
        
        // Se conectado, verifica o status da conexão a cada 5 segundos
        if (connected) {
            uint32_t now = to_ms_since_boot(get_absolute_time());
            if (now - last_check >= 5000) {
                check_connection_status();
                last_check = now;
            }
        }
        // Se não conectado e terminou um scan, agenda o próximo em 10 s
        else if (!cyw43_wifi_scan_active(&cyw43_state) && scan_started) {
            scan_started = false;
            select_best_protocol(); // Seleciona, exibe e tenta conectar
            hard_assert(async_context_add_at_time_worker_in_ms(
                cyw43_arch_async_context(), &scan_worker, 10000));
        }
        
        #if PICO_CYW43_ARCH_POLL
            cyw43_arch_poll();
            cyw43_arch_wait_for_work_until(at_the_end_of_time);
        #else
            sleep_ms(1000);
        #endif
    }
    
    cyw43_arch_deinit();
    return 0;
}