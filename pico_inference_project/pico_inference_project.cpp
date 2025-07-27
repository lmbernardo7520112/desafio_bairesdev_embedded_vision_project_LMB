// ======================================================================= 
//  ARQUIVO FINAL E FUNCIONAL (SEM I2C, MONITORADO VIA PUTTY)
// =======================================================================
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "inference.h" // Nossa lógica de inferência (sem display)

// --- CONFIGURAÇÃO DE PINOS ---
// UART para comunicação com a Raspberry Pi 3
#define PI_UART_INSTANCE   uart1
#define PI_UART_TX_PIN     4
#define PI_UART_RX_PIN     9
#define PI_UART_BAUD_RATE  115200

int main() {
    // Inicializa USB e aguarda conexão do host (evita perder prints)
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    sleep_ms(100); // Delay extra por segurança

    // Mensagem de boot que PRECISA aparecer no PuTTY
    printf("\n\n--- PICO W INFERENCE ENGINE (Build Estável) ---\n");
    printf("STATUS: USB (stdio) inicializado com sucesso.\n");

    // Inicializa a UART
    uart_init(PI_UART_INSTANCE, PI_UART_BAUD_RATE);
    gpio_set_function(PI_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(PI_UART_RX_PIN, GPIO_FUNC_UART);
    printf("STATUS: UART para Pi 3 inicializada nos pinos TX=%d, RX=%d.\n", PI_UART_TX_PIN, PI_UART_RX_PIN);

    // Sinal de pronto
    printf("\n>>> SISTEMA PRONTO. AGUARDANDO GATILHO DA PI 3... <<<\n");

    char uart_buffer[20];
    int buffer_index = 0;

    while (true) {
        if (uart_is_readable(PI_UART_INSTANCE)) {
            char c = uart_getc(PI_UART_INSTANCE);

            if (c == '\n' || buffer_index >= (sizeof(uart_buffer) - 1)) {
                uart_buffer[buffer_index] = '\0';

                if (strncmp(uart_buffer, "FOTO_OK", 7) == 0) {
                    printf("\n[PICO_W] GATILHO 'FOTO_OK' RECEBIDO!\n");
                    
                    run_inference(); 

                    printf("\n>>> SISTEMA PRONTO. AGUARDANDO PRÓXIMO GATILHO... <<<\n");
                }
                buffer_index = 0;
            } else if (c >= ' ') {
                uart_buffer[buffer_index++] = c;
            }
        }
    }

    return 0;
}

