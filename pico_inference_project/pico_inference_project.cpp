// =======================================================================
//  ARQUIVO FINAL E FUNCIONAL - VERSÃO UDP (SEM I2C, MONITORADO VIA PUTTY)
// =======================================================================

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/udp.h"
#include "inference.h" // Nossa lógica de inferência (sem display)

// ================= CONFIG WiFi =================
#define WIFI_SSID     "Leonardo e Victor 5G"
#define WIFI_PASS     "3546792024"
#define UDP_PORT      8080

// ================= VARIÁVEIS =================
static struct udp_pcb *udp_pcb_global = NULL;

// =======================================================================
// Callback para tratar mensagens recebidas via UDP
// =======================================================================
void udp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                       const ip_addr_t *addr, u16_t port) {
    if (!p) return;

    char msg[64];
    size_t len = (p->len < sizeof(msg) - 1) ? p->len : sizeof(msg) - 1;
    memcpy(msg, p->payload, len);
    msg[len] = '\0';

    printf("📩 Recebido de %s:%d -> '%s'\n", ipaddr_ntoa(addr), port, msg);

    // Verifica se é FOTO_OK
    if (strncmp(msg, "FOTO_OK", 7) == 0) {
        printf("\n[PICO_W] GATILHO 'FOTO_OK' RECEBIDO!\n");

        run_inference();

        printf("\n>>> SISTEMA PRONTO. AGUARDANDO PRÓXIMO GATILHO... <<<\n");
    } else {
        printf("[PICO_W] Comando UDP desconhecido: '%s'\n", msg);
    }

    pbuf_free(p);
}

// =======================================================================
// Inicializa WiFi
// =======================================================================
bool init_wifi() {
    printf("📡 Inicializando WiFi...\n");
    if (cyw43_arch_init()) {
        printf("❌ Erro ao inicializar WiFi\n");
        return false;
    }
    cyw43_arch_enable_sta_mode();

    printf("🔗 Conectando-se à rede '%s'...\n", WIFI_SSID);
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS,
                                           CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("❌ Falha na conexão WiFi\n");
        return false;
    }
    printf("✅ Conectado! IP: %s\n", ipaddr_ntoa(netif_ip4_addr(netif_default)));
    return true;
}

// =======================================================================
// Inicializa servidor UDP
// =======================================================================
bool init_udp() {
    udp_pcb_global = udp_new();
    if (!udp_pcb_global) {
        printf("❌ Erro ao criar PCB UDP\n");
        return false;
    }

    if (udp_bind(udp_pcb_global, IP_ADDR_ANY, UDP_PORT) != ERR_OK) {
        printf("❌ Erro ao bind UDP\n");
        udp_remove(udp_pcb_global);
        return false;
    }

    udp_recv(udp_pcb_global, udp_recv_callback, NULL);
    printf("✅ Servidor UDP escutando na porta %d\n", UDP_PORT);
    return true;
}

// =======================================================================
// MAIN
// =======================================================================
int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    sleep_ms(100);

    printf("\n\n--- PICO W INFERENCE ENGINE (UDP) ---\n");
    printf("STATUS: USB (stdio) inicializado com sucesso.\n");

    if (!init_wifi()) {
        printf("💥 Erro crítico: não foi possível conectar ao WiFi.\n");
        while (1) sleep_ms(1000);
    }

    if (!init_udp()) {
        printf("💥 Erro crítico: não foi possível iniciar servidor UDP.\n");
        while (1) sleep_ms(1000);
    }

    printf("\n>>> SISTEMA PRONTO. AGUARDANDO GATILHO VIA UDP... <<<\n");

    // Loop principal — apenas mantém a pilha lwIP rodando
    while (true) {
        cyw43_arch_poll();
        sleep_ms(5);
    }

    return 0;
}
