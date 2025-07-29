// =======================================================================
//  ARQUIVO FINAL E ROBUSTO PARA COMUNICAÇÃO UART COM A PI 3
//  - Comunicação UART limpa, segura e compatível
//  - Detecta "FOTO_OK\n" com precisão
//  - Executa inferência ao receber comando correto
// =======================================================================

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "inference.h"

// UART config
#define PI_UART_INSTANCE   uart1
#define PI_UART_TX_PIN     4
#define PI_UART_RX_PIN     9
#define PI_UART_BAUD_RATE  115200

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    sleep_ms(200);

    printf("\n\n--- PICO W INFERENCE ENGINE (Build Estável) ---\n");
    printf("STATUS: USB (stdio) inicializado com sucesso.\n");

    uart_init(PI_UART_INSTANCE, PI_UART_BAUD_RATE);
    gpio_set_function(PI_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(PI_UART_RX_PIN, GPIO_FUNC_UART);
    printf("STATUS: UART para Pi 3 inicializada nos pinos TX=%d, RX=%d.\n", PI_UART_TX_PIN, PI_UART_RX_PIN);

    printf("\n>>> SISTEMA PRONTO. AGUARDANDO GATILHO DA PI 3... <<<\n");

    char uart_buffer[64];
    int buffer_index = 0;

    while (true) {
        while (uart_is_readable(PI_UART_INSTANCE)) {
            char c = uart_getc(PI_UART_INSTANCE);
            // printf("[UART DEBUG] Recebido: 0x%02X '%c'\n", c, c);

            if (c == '\n' || c == '\r') {
                if (buffer_index > 0) {
                    uart_buffer[buffer_index] = '\0';
                    printf("[UART] Comando recebido: '%s'\n", uart_buffer);

                    if (strcmp(uart_buffer, "TESTE_UART") == 0) {
                        printf("[PICO] Teste UART recebido com sucesso.\n");
                    } else if (strcmp(uart_buffer, "FOTO_OK") == 0) {
                        printf("[PICO] Gatilho 'FOTO_OK' reconhecido. Executando inferência...\n");
                        run_inference();
                        printf("[PICO] Inferência finalizada.\n");
                    } else {
                        printf("[PICO] Comando UART desconhecido: '%s'\n", uart_buffer);
                    }

                    buffer_index = 0;
                    memset(uart_buffer, 0, sizeof(uart_buffer));
                }
            } else if (c >= 32 && c <= 126) {
                if (buffer_index < (int)(sizeof(uart_buffer) - 1)) {
                    uart_buffer[buffer_index++] = c;
                }
            }
        }

        sleep_ms(10);
    }

    return 0;
}
