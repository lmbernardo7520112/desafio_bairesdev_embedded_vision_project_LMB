#ifndef SSD1306_H
#define SSD1306_H

#include "hardware/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t buffer[1024];
    uint8_t width;
    uint8_t height;
    uint8_t address;
    i2c_inst_t *i2c;
} ssd1306_t;

void ssd1306_init(ssd1306_t *dev, uint8_t width, uint8_t height, uint8_t address, i2c_inst_t *i2c);
void ssd1306_clear(ssd1306_t *dev);
void ssd1306_show(ssd1306_t *dev);
void ssd1306_draw_string(ssd1306_t *dev, uint32_t x, uint32_t y, uint32_t scale, const char *s);

#ifdef __cplusplus
}
#endif

#endif // SSD1306_H