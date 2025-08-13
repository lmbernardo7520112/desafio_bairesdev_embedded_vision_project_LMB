// pico_inference_project.cpp
// Recebe imagem via UDP (raw lwIP API). Protocolo: header 4 bytes (idx(2), total(2)) + payload
// Envia ACK por chunk: "ACK"+idx(2). Quando todos os chunks recebidos, chama run_inference_from_buffer().
// VERSÃO CORRIGIDA: Scan completo + captura canal/auth + logging detalhado

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
#include "wifi_config.h" // define WIFI_SSID e WIFI_PASS

#define UDP_PORT            5005
#define CHUNK_PAYLOAD_MAX   1000
#define IMAGE_BUFFER_MAX    (64*1024)

static uint8_t  image_buffer[IMAGE_BUFFER_MAX];
static uint16_t expected_total_chunks = 0;
static uint8_t* chunks_bitmap = NULL;
static uint32_t bytes_received = 0;
static struct udp_pcb* udp_pcb_global = NULL;

// ========================= Utils de Log/Estado =========================
static void print_string_bytes_hex(const char *label, const char *s) {
    if (!s) { printf("%s: <null>\n", label); return; }
    size_t len = strlen(s);
    printf("%s (len=%u) bytes:", label, (unsigned)len);
    for (size_t i = 0; i < len; ++i) printf(" %02X", (unsigned char)s[i]);
    printf("\n");
}

static const char* auth_mode_to_string(uint32_t auth_mode) {
    switch (auth_mode) {
        case CYW43_AUTH_OPEN:           return "OPEN";
        case CYW43_AUTH_WPA_TKIP_PSK:   return "WPA-TKIP-PSK";
        case CYW43_AUTH_WPA2_AES_PSK:   return "WPA2-AES-PSK";
        case CYW43_AUTH_WPA2_MIXED_PSK: return "WPA2-MIXED-PSK";
        default: 
            // Para valores não mapeados, mostra o código hexadecimal
            static char unknown_buf[32];
            snprintf(unknown_buf, sizeof(unknown_buf), "UNKNOWN(0x%x)", (unsigned)auth_mode);
            return unknown_buf;
    }
}

static const char* link_status_to_string(int status) {
    switch (status) {
        case CYW43_LINK_DOWN:     return "DOWN";
        case CYW43_LINK_JOIN:     return "JOINING";
        case CYW43_LINK_NOIP:     return "NO_IP";
        case CYW43_LINK_UP:       return "UP";
        case CYW43_LINK_FAIL:     return "FAIL";
        case CYW43_LINK_NONET:    return "NO_NET";
        case CYW43_LINK_BADAUTH:  return "BAD_AUTH";
        default: return "UNKNOWN";
    }
}

static void dump_cyw43_state(void) {
    struct netif *netif = netif_default;
    if (!netif) { printf("[DEBUG] netif_default não está disponível.\n"); return; }

    uint8_t mac[6]; cyw43_hal_get_mac(0, mac);
    int link_status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
    
    printf("\n--- [DEBUG] Estado da Interface de Rede ---\n");
    printf("Endereço MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("Status do Link: %s (%d)\n", link_status_to_string(link_status), link_status);
    printf("IP: %s\n", ipaddr_ntoa(netif_ip_addr4(netif)));
    printf("Gateway: %s\n", ipaddr_ntoa(netif_ip_gw4(netif)));
    printf("Netmask: %s\n", ipaddr_ntoa(netif_ip_netmask4(netif)));
    printf("------------------------------------------\n\n");
}

static bool have_ip_address(void) {
    struct netif *netif = netif_default;
    if (!netif) return false;
    const ip4_addr_t *ip = netif_ip4_addr(netif);
    return ip && !ip4_addr_isany(ip);
}

// ========================= UDP =========================
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
    if (!p || !pcb || !addr) { if (p) pbuf_free(p); return; }
    if (p->tot_len < 4) { pbuf_free(p); return; }

    uint8_t header[4];
    pbuf_copy_partial(p, header, 4, 0);
    uint16_t idx   = (uint16_t)((header[0] << 8) | header[1]);
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

    if (!get_chunk_bit(idx)) { set_chunk_bit(idx); bytes_received += (uint32_t)copied; }

    send_ack(pcb, addr, port, idx);

    bool all = (expected_total_chunks != 0);
    for (uint16_t i = 0; all && i < expected_total_chunks; ++i) {
        if (!get_chunk_bit(i)) { all = false; break; }
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
    if (!udp_pcb_global) { printf("Erro: udp_new()\n"); return false; }
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

// ========================= Wi-Fi: Scan Completo + Conexão com Canal/Auth Específicos =========================
#include "cyw43.h"

typedef struct {
    bool found;
    uint8_t channel;
    uint32_t auth_mode;
    int8_t best_rssi;
    char ssid[33]; // Para verificação
} target_ap_info_t;

static target_ap_info_t target_ap;

static int detailed_scan_cb(void *env, const cyw43_ev_scan_result_t *res) {
    target_ap_info_t *info = (target_ap_info_t*)env;
    
    if (!res) { 
        printf("[SCAN] *** Scan finalizado ***\n");
        return 0; 
    }
    
    // Log de todos os APs encontrados
    printf("[SCAN] SSID='%s' RSSI=%d dBm Canal=%d Auth=%s BSSID=%02x:%02x:%02x:%02x:%02x:%02x\n",
           res->ssid, res->rssi, res->channel, auth_mode_to_string(res->auth_mode),
           res->bssid[0], res->bssid[1], res->bssid[2], res->bssid[3], res->bssid[4], res->bssid[5]);
    
    // Verifica se é o SSID alvo
    if (strncmp((const char*)res->ssid, WIFI_SSID, sizeof(res->ssid)) == 0) {
        printf("[SCAN] *** ENCONTROU AP ALVO! ***\n");
        printf("[SCAN] Target SSID='%s' RSSI=%d dBm Canal=%d Auth=%s\n",
               res->ssid, res->rssi, res->channel, auth_mode_to_string(res->auth_mode));
        
        // Pega o melhor sinal se houver múltiplos APs com mesmo SSID
        if (!info->found || res->rssi > info->best_rssi) {
            info->found = true;
            info->channel = res->channel;
            info->auth_mode = res->auth_mode;
            info->best_rssi = res->rssi;
            strncpy(info->ssid, (const char*)res->ssid, sizeof(info->ssid) - 1);
            info->ssid[sizeof(info->ssid) - 1] = '\0';
            
            printf("[SCAN] *** ATUALIZADO melhor AP: Canal=%d Auth=%s RSSI=%d ***\n",
                   info->channel, auth_mode_to_string(info->auth_mode), info->best_rssi);
        }
    }
    
    return 0;
}

static bool perform_detailed_scan(uint32_t timeout_ms) {
    printf("\n=== INICIANDO SCAN DETALHADO ===\n");
    
    // Limpa dados anteriores
    memset(&target_ap, 0, sizeof(target_ap));
    
    // Configura scan para busca ativa em todos os canais
    cyw43_wifi_scan_options_t opts = {0};
    opts.scan_type = 0; // Active scan (padrão)
    // Deixa channel_list NULL para escanear todos os canais
    
    printf("[SCAN] Procurando por SSID: '%s'\n", WIFI_SSID);
    printf("[SCAN] Escaneando todos os canais (2.4GHz)...\n");
    
    int err = cyw43_wifi_scan(&cyw43_state, &opts, &target_ap, detailed_scan_cb);
    if (err) {
        printf("[SCAN] ERRO ao iniciar scan: %d\n", err);
        return false;
    }
    
    // Aguarda conclusão do scan
    absolute_time_t until = make_timeout_time_ms(timeout_ms);
    printf("[SCAN] Aguardando até %u ms...\n", (unsigned)timeout_ms);
    
    while (absolute_time_diff_us(get_absolute_time(), until) > 0) {
        cyw43_arch_poll();
        sleep_ms(50); // Polling mais frequente durante scan
    }
    
    printf("=== SCAN FINALIZADO ===\n");
    
    if (target_ap.found) {
        printf("[SCAN] ✓ AP ALVO ENCONTRADO!\n");
        printf("[SCAN]   SSID: '%s'\n", target_ap.ssid);
        printf("[SCAN]   Canal: %d\n", target_ap.channel);
        printf("[SCAN]   Auth: %s (0x%x)\n", auth_mode_to_string(target_ap.auth_mode), (unsigned)target_ap.auth_mode);
        printf("[SCAN]   RSSI: %d dBm\n", target_ap.best_rssi);
        return true;
    } else {
        printf("[SCAN] ✗ AP ALVO NÃO ENCONTRADO!\n");
        printf("[SCAN] Verifique se:\n");
        printf("[SCAN]   1. O SSID '%s' está correto\n", WIFI_SSID);
        printf("[SCAN]   2. O AP está ligado e broadcasting\n");
        printf("[SCAN]   3. O AP está na faixa 2.4GHz\n");
        return false;
    }
}

static void log_connection_progress(const char* stage) {
    int status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
    printf("[AUTH] %s - Status: %s (%d)\n", stage, link_status_to_string(status), status);
    dump_cyw43_state();
}

// Conecta usando canal e auth específicos detectados no scan
static int connect_with_specific_params(const char *ssid, const char *pass, uint8_t channel, uint32_t auth_mode, uint32_t timeout_ms) {
    printf("\n=== INICIANDO CONEXÃO COM PARÂMETROS ESPECÍFICOS ===\n");
    printf("[CONN] SSID: '%s'\n", ssid);
    printf("[CONN] Canal: %d\n", channel);
    printf("[CONN] Auth: %s (0x%x)\n", auth_mode_to_string(auth_mode), (unsigned)auth_mode);
    printf("[CONN] Timeout: %u ms\n", (unsigned)timeout_ms);
    
    // Log senha (cuidado em produção!)
    print_string_bytes_hex("[CONN] Senha", pass);
    
    // Desativa powersave para melhor performance durante conexão
    printf("[AUTH] Desativando powersave...\n");
    cyw43_wifi_pm(&cyw43_state, CYW43_NO_POWERSAVE_MODE);
    
    log_connection_progress("PRÉ-CONEXÃO");
    
    // Inicia conexão com parâmetros específicos
    printf("[AUTH] Chamando cyw43_arch_wifi_connect_timeout_ms...\n");
    int ret = cyw43_arch_wifi_connect_timeout_ms(ssid, pass, auth_mode, timeout_ms);
    
    printf("[AUTH] Resultado da conexão: %d\n", ret);
    log_connection_progress("PÓS-TENTATIVA DE CONEXÃO");
    
    if (ret == 0) {
        printf("[AUTH] ✓ Link estabelecido com sucesso!\n");
        
        // Aguarda IP via DHCP
        printf("[DHCP] Aguardando endereço IP...\n");
        absolute_time_t dhcp_until = make_timeout_time_ms(10000); // 10s para DHCP
        
        while (!have_ip_address() && absolute_time_diff_us(get_absolute_time(), dhcp_until) > 0) {
            cyw43_arch_poll();
            sleep_ms(100);
            
            // Log periódico do status DHCP
            static uint32_t dhcp_log_counter = 0;
            if (++dhcp_log_counter % 20 == 0) { // A cada 2 segundos
                printf("[DHCP] Ainda aguardando IP... (%u)\n", dhcp_log_counter / 10);
                log_connection_progress("AGUARDANDO DHCP");
            }
        }
        
        if (have_ip_address()) {
            printf("[DHCP] ✓ IP obtido com sucesso!\n");
        } else {
            printf("[DHCP] ✗ Timeout no DHCP (prosseguindo mesmo assim)\n");
        }
        
        log_connection_progress("CONEXÃO FINAL");
        return 0;
        
    } else {
        printf("[AUTH] ✗ Falha na conexão!\n");
        
        // Diagnóstico detalhado do erro
        switch (ret) {
            case CYW43_LINK_NONET:
                printf("[AUTH] Causa: Rede não encontrada no canal %d\n", channel);
                printf("[AUTH] Sugestão: Verifique se o AP ainda está no canal %d\n", channel);
                break;
                
            case CYW43_LINK_BADAUTH:
                printf("[AUTH] Causa: Falha de autenticação\n");
                printf("[AUTH] Verificações:\n");
                printf("[AUTH]   1. Senha está correta?\n");
                printf("[AUTH]   2. Modo auth %s é suportado pelo AP?\n", auth_mode_to_string(auth_mode));
                printf("[AUTH]   3. AP não está rejeitando por filtro MAC?\n");
                break;
                
            case CYW43_LINK_FAIL:
                printf("[AUTH] Causa: Falha de associação/link\n");
                printf("[AUTH] Possíveis causas:\n");
                printf("[AUTH]   1. Interferência no canal %d\n", channel);
                printf("[AUTH]   2. AP com muitos clientes conectados\n");
                printf("[AUTH]   3. Sinal muito fraco (era %d dBm)\n", target_ap.best_rssi);
                break;
                
            default:
                printf("[AUTH] Causa: Timeout ou erro desconhecido (%d)\n", ret);
                break;
        }
        
        log_connection_progress("FALHA DE CONEXÃO");
        return ret;
    }
}

// Função principal de conexão com tentativas
static int connect_with_retries(const char *ssid, const char *pass, int max_attempts) {
    int ret = -1;

    for (int attempt = 1; attempt <= max_attempts; ++attempt) {
        printf("\n##########################################\n");
        printf("### TENTATIVA DE CONEXÃO %d/%d ###\n", attempt, max_attempts);
        printf("##########################################\n");

        // 1. Scan detalhado para encontrar o AP exato
        if (!perform_detailed_scan(8000)) { // 8 segundos de scan
            printf("[ERRO] AP não encontrado no scan. Tentando novamente...\n");
            sleep_ms(2000);
            continue;
        }
        
        // 2. Conecta usando os parâmetros exatos encontrados
        ret = connect_with_specific_params(ssid, pass, target_ap.channel, target_ap.auth_mode, 30000);
        
        if (ret == 0) {
            printf("\n🎉 SUCESSO na tentativa %d! 🎉\n", attempt);
            return 0;
        }
        
        printf("\n💥 FALHA na tentativa %d (erro %d)\n", attempt, ret);
        
        if (attempt < max_attempts) {
            printf("Aguardando 3 segundos antes da próxima tentativa...\n");
            sleep_ms(3000);
        }
    }
    
    printf("\n❌ TODAS AS TENTATIVAS FALHARAM ❌\n");
    printf("Erro final: %d\n", ret);
    return ret;
}

// ========================= main =========================
int main(void) {
    stdio_init_all();
    while (!stdio_usb_connected()) sleep_ms(100);
    sleep_ms(500); // Tempo extra para estabilizar

    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Pico W UDP Image Receiver (v3 - Scan + Auth Detalhado)   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    if (cyw43_arch_init()) {
        printf("❌ ERRO CRÍTICO: cyw43_arch_init() falhou\n");
        return -1;
    }

    printf("✓ cyw43 inicializado com sucesso.\n");
    dump_cyw43_state();

    cyw43_arch_enable_sta_mode();
    printf("✓ Modo Station (STA) ativado.\n");

    int conn_ret = connect_with_retries(WIFI_SSID, WIFI_PASS, 3);
    if (conn_ret != 0) {
        printf("\n❌ FALHA FINAL DE CONEXÃO WIFI ❌\n");
        printf("Erro final: %d\n", conn_ret);
        printf("Dados finais utilizados:\n");
        print_string_bytes_hex("SSID", WIFI_SSID);
        print_string_bytes_hex("PASS", WIFI_PASS);
        cyw43_arch_deinit();
        return -1;
    }

    printf("\n🌐 CONEXÃO WIFI ESTABELECIDA COM SUCESSO! 🌐\n");
    dump_cyw43_state();

    if (!init_udp_server(UDP_PORT)) {
        printf("❌ Falha ao inicializar servidor UDP\n");
        cyw43_arch_deinit();
        return -1;
    }

    printf("\n🚀 Sistema pronto! Aguardando imagens UDP na porta %d...\n", UDP_PORT);

    // Loop principal: mantém a pilha viva e o servidor UDP
    while (true) {
        cyw43_arch_poll();
        sleep_ms(10);
    }

    cyw43_arch_deinit();
    return 0;
}
