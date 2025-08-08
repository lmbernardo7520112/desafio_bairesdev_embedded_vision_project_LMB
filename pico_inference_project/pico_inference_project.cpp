// =======================================================================
//  CÓDIGO UART SUPER ROBUSTO PARA PICO W
//  - Configuração precisa de UART
//  - Debug detalhado
//  - Tratamento de diferentes baud rates
// =======================================================================

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "inference.h"

#define PI_UART_INSTANCE   uart1
#define PI_UART_TX_PIN     4
#define PI_UART_RX_PIN     9

// Array de baud rates para teste
const uint32_t BAUD_RATES[] = {115200, 57600, 38400, 19200, 9600};
const int NUM_BAUD_RATES = sizeof(BAUD_RATES) / sizeof(BAUD_RATES[0]);

void test_baud_rate(uint32_t baud_rate) {
    printf("\n=== TESTANDO BAUD RATE: %d ===\n", baud_rate);
    
    // Reinicializar UART com novo baud rate
    uart_deinit(PI_UART_INSTANCE);
    uart_init(PI_UART_INSTANCE, baud_rate);
    
    // Reconfigurar pinos
    gpio_set_function(PI_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(PI_UART_RX_PIN, GPIO_FUNC_UART);
    
    printf("UART configurada para %d baud\n", baud_rate);
    printf("Aguardando dados por 5 segundos...\n");
    
    char uart_buffer[64];
    int buffer_index = 0;
    uint32_t timeout_ms = 5000;
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    
    while ((to_ms_since_boot(get_absolute_time()) - start_time) < timeout_ms) {
        while (uart_is_readable(PI_UART_INSTANCE)) {
            char c = uart_getc(PI_UART_INSTANCE);
            
            // Debug detalhado
            printf("[%d BAUD] Byte: 0x%02X (%c) ASCII:%d\n", 
                   baud_rate, (unsigned char)c, 
                   (c >= 32 && c <= 126) ? c : '?', c);
            
            // Buffer management
            if (buffer_index < (int)(sizeof(uart_buffer) - 1)) {
                uart_buffer[buffer_index++] = c;
            }
            
            // Detectar fim de linha
            if (c == '\n' || c == '\r') {
                uart_buffer[buffer_index - 1] = '\0';
                printf("[%d BAUD] MENSAGEM COMPLETA: '%s'\n", baud_rate, uart_buffer);
                
                // Verificar comandos
                if (strcmp(uart_buffer, "TESTE_UART") == 0) {
                    printf("🎯 SUCESSO! Teste UART reconhecido com %d baud!\n", baud_rate);
                    uart_puts(PI_UART_INSTANCE, "PICO_ACK_TESTE\n");
                } else if (strcmp(uart_buffer, "FOTO_OK") == 0) {
                    printf("🎯 SUCESSO! FOTO_OK reconhecido com %d baud!\n", baud_rate);
                    uart_puts(PI_UART_INSTANCE, "PICO_ACK_FOTO\n");
                    run_inference();
                }
                
                // Reset buffer
                buffer_index = 0;
                memset(uart_buffer, 0, sizeof(uart_buffer));
            }
        }
        sleep_ms(10);
    }
    
    printf("Timeout para %d baud\n", baud_rate);
}

void uart_comprehensive_test() {
    printf("\n🔍 INICIANDO TESTE ABRANGENTE DE UART\n");
    
    for (int i = 0; i < NUM_BAUD_RATES; i++) {
        test_baud_rate(BAUD_RATES[i]);
        sleep_ms(1000); // Pausa entre testes
    }
    
    printf("\n✅ TESTE ABRANGENTE FINALIZADO\n");
}

int main() {
    stdio_init_all();
    
    // Aguardar conexão USB
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    sleep_ms(500);

    printf("\n\n=== PICO W UART DIAGNOSTIC SYSTEM ===\n");
    printf("Pinos: TX=%d, RX=%d\n", PI_UART_TX_PIN, PI_UART_RX_PIN);
    
    // Teste inicial com 115200 (padrão)
    printf("\n--- FASE 1: TESTE PADRÃO (115200) ---\n");
    uart_init(PI_UART_INSTANCE, 115200);
    gpio_set_function(PI_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(PI_UART_RX_PIN, GPIO_FUNC_UART);
    
    char uart_buffer[64];
    int buffer_index = 0;
    uint32_t last_test_time = 0;
    
    printf("Sistema pronto. Enviando dados da Pi 3...\n");
    
    while (true) {
        // A cada 30 segundos, fazer teste abrangente se não receber dados válidos
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_test_time > 30000) {
            printf("\n⚠️  Sem dados válidos por 30s. Iniciando diagnóstico...\n");
            uart_comprehensive_test();
            last_test_time = current_time;
            
            // Voltar para configuração padrão
            uart_init(PI_UART_INSTANCE, 115200);
            gpio_set_function(PI_UART_TX_PIN, GPIO_FUNC_UART);
            gpio_set_function(PI_UART_RX_PIN, GPIO_FUNC_UART);
        }
        
        while (uart_is_readable(PI_UART_INSTANCE)) {
            char c = uart_getc(PI_UART_INSTANCE);
            
            // Log detalhado de cada byte
            printf("[UART] Byte: 0x%02X (%c)\n", 
                   (unsigned char)c, (c >= 32 && c <= 126) ? c : '.');
            
            if (buffer_index < (int)(sizeof(uart_buffer) - 1)) {
                uart_buffer[buffer_index++] = c;
            }
            
            if (c == '\n') {
                uart_buffer[buffer_index - 1] = '\0';
                printf("[UART] Mensagem: '%s'\n", uart_buffer);
                
                if (strcmp(uart_buffer, "TESTE_UART") == 0) {
                    printf("✅ TESTE_UART reconhecido!\n");
                    uart_puts(PI_UART_INSTANCE, "PICO_OK\n");
                } else if (strcmp(uart_buffer, "FOTO_OK") == 0) {
                    printf("📸 FOTO_OK reconhecido! Executando inferência...\n");
                    uart_puts(PI_UART_INSTANCE, "INFERENCE_START\n");
                    run_inference();
                    uart_puts(PI_UART_INSTANCE, "INFERENCE_DONE\n");
                } else {
                    printf("❓ Comando desconhecido: '%s'\n", uart_buffer);
                }
                
                buffer_index = 0;
                memset(uart_buffer, 0, sizeof(uart_buffer));
                last_test_time = current_time; // Reset timer após receber dados
            }
        }
        
        sleep_ms(10);
    }

    return 0;
}