#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"

#include "ssd1306.h"
#include "inference.h"
#include "pico/platform.h"

// Define os pinos padrão da BitDogLab para I2C
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

// Inicializa UART0 e I2C0
void init_peripherals() {
    stdio_init_all();

    // UART0 - comunicação com a Pi 3
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);  // TX
    gpio_set_function(1, GPIO_FUNC_UART);  // RX

    // I2C0 - display OLED
    i2c_init(i2c0, 400 * 1000); // 400kHz
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Inicializa display OLED
    ssd1306_init(i2c0);
}

int main() {
    init_peripherals();

    // Mensagem inicial no OLED
    ssd1306_clear();
    ssd1306_draw_text(0, 0, "Aguardando Pi 3...");
    ssd1306_show();

    // Loop principal que escuta UART e dispara inferência
    monitor_uart_and_infer();

    return 0;
}
