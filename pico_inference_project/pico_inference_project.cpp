#include <stdio.h>
#include "ssd1306.h"
#include "inference.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "image_provider.h"  // ✅ Necessário para usar FillImage()

// Display externo (definido em inference.cpp)
extern ssd1306_t display;

int main() {
    // Inicialização do sistema
    stdio_init_all();

    // Inicialização do display I2C (ajuste os pinos conforme necessário)
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    // Inicializar o display SSD1306
    // display.external_vcc = false;
    ssd1306_init(&display, 128, 64, 0x3C, i2c_default);

    // ✅ Gera dados de imagem simulada para teste
    FillImage();

    // Executar inferência
    run_inference();

    while (true) {
        sleep_ms(1000);  // Loop de manutenção
    }

    return 0;
}

