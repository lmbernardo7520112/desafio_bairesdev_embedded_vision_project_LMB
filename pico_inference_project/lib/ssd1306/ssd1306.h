#ifndef SSD1306_H
#define SSD1306_H

#include "hardware/i2c.h"
#include <stdint.h>
#include <stdbool.h>

#define SSD1306_I2C_ADDR 0x3C
#define SSD1306_WIDTH    128
#define SSD1306_HEIGHT   64

void ssd1306_init(i2c_inst_t *i2c);
void ssd1306_clear();
void ssd1306_show();
void ssd1306_draw_text(uint8_t row, uint8_t col, const char *text);

#endif
