// pico_inference_project.cpp
// Recebe imagem via UDP (raw lwIP API). Protocol: header 4 bytes (idx(2), total(2)) + payload
// Envia ACK por chunk: "ACK"+idx(2). Quando todos os chunks recebidos, chama run_inference_from_buffer().

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

#include "model_settings.h" // kNumRows, kNumCols, kNumChannels, kMaxImageSize
#include "inference.h"      // run_inference_from_buffer(uint8_t*, size_t, int, int)
#include "wifi_config.h"    // seu wifi_config.h (NÃO versionar)

#define UDP_PORT 5005
#define CHUNK_PAYLOAD_MAX 1000
#define IMAGE_BUFFER_MAX (64*1024)

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

    // On first chunk initialize bookkeeping
    if (bytes_received == 0 && idx == 0) {
        expected_total_chunks = total;
        if (chunks_bitmap) { free(chunks_bitmap); chunks_bitmap = NULL; }
        if (expected_total_chunks > 0) {
            size_t bsize = (expected_total_chunks + 7) / 8;
            chunks_bitmap = (uint8_t*)malloc(bsize);
            if (chunks_bitmap) memset(chunks_bitmap, 0, bsize);
            else {
                printf("[PICO] Aviso: falha ao alocar chunks_bitmap (%u bytes)\n", (unsigned)bsize);
            }
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

    // Copy payload skipping the 4 header bytes directly into image_buffer at offset
    size_t copied = pbuf_copy_partial(p, image_buffer + offset, (u16_t)payload_len, 4);
    if (copied != payload_len) {
        printf("[PICO] Aviso: copiados %u/%u bytes do chunk %u\n", (unsigned)copied, (unsigned)payload_len, idx);
    }

    // mark and account bytes only once per chunk
    if (!get_chunk_bit(idx)) {
        set_chunk_bit(idx);
        bytes_received += (uint32_t)copied;
    }

    // send ACK for this chunk
    send_ack(pcb, addr, port, idx);

    // check completion
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

        // chama a função de inferência (deve ser implementada em inference.cpp)
        run_inference_from_buffer(image_buffer, bytes_received, kNumCols, kNumRows);

        // enviar DONE
        const char done[] = "TRANSFER_DONE";
        struct pbuf *pdone = pbuf_alloc(PBUF_TRANSPORT, sizeof(done) - 1, PBUF_RAM);
        if (pdone) {
            memcpy(pdone->payload, done, sizeof(done) - 1);
            udp_sendto(pcb, pdone, addr, port);
            pbuf_free(pdone);
        }

        // cleanup
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

int main(void) {
    stdio_init_all();
    while (!stdio_usb_connected()) sleep_ms(100);
    sleep_ms(200);

    printf("\n--- Pico W UDP Image Receiver ---\n");

    if (cyw43_arch_init()) {
        printf("cyw43_arch_init failed\n");
        return -1;
    }
    cyw43_arch_enable_sta_mode();

    printf("Conectando a SSID '%s'...\n", WIFI_SSID);
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0) {
        printf("Falha ao conectar WiFi\n");
        return -1;
    }
    printf("WiFi OK. IP: %s\n", ipaddr_ntoa(netif_ip4_addr(netif_default)));

    if (!init_udp_server(UDP_PORT)) return -1;

    while (true) {
        cyw43_arch_poll(); // processa stack WiFi/lwIP
        sleep_ms(10);
    }
    return 0;
}
