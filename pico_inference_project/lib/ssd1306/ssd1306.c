#include "ssd1306.h"
#include "font8x8_basic.h"
#include "pico/stdlib.h"
#include <string.h>

static void ssd1306_send_cmd(ssd1306_t *dev, uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    i2c_write_blocking(dev->i2c, dev->address, buf, 2, false);
}

void ssd1306_init(ssd1306_t *dev, uint8_t width, uint8_t height, uint8_t address, i2c_inst_t *i2c) {
    dev->width = width;
    dev->height = height;
    dev->address = address;
    dev->i2c = i2c;

    sleep_ms(100);

    ssd1306_send_cmd(dev, 0xAE);
    ssd1306_send_cmd(dev, 0x20); ssd1306_send_cmd(dev, 0x00);
    ssd1306_send_cmd(dev, 0xB0);
    ssd1306_send_cmd(dev, 0xC8);
    ssd1306_send_cmd(dev, 0x00);
    ssd1306_send_cmd(dev, 0x10);
    ssd1306_send_cmd(dev, 0x40);
    ssd1306_send_cmd(dev, 0x81); ssd1306_send_cmd(dev, 0x7F);
    ssd1306_send_cmd(dev, 0xA1);
    ssd1306_send_cmd(dev, 0xA6);
    ssd1306_send_cmd(dev, 0xA8); ssd1306_send_cmd(dev, 0x3F);
    ssd1306_send_cmd(dev, 0xA4);
    ssd1306_send_cmd(dev, 0xD3); ssd1306_send_cmd(dev, 0x00);
    ssd1306_send_cmd(dev, 0xD5); ssd1306_send_cmd(dev, 0x80);
    ssd1306_send_cmd(dev, 0xD9); ssd1306_send_cmd(dev, 0xF1);
    ssd1306_send_cmd(dev, 0xDA); ssd1306_send_cmd(dev, 0x12);
    ssd1306_send_cmd(dev, 0xDB); ssd1306_send_cmd(dev, 0x40);
    ssd1306_send_cmd(dev, 0x8D); ssd1306_send_cmd(dev, 0x14);
    ssd1306_send_cmd(dev, 0xAF);

    ssd1306_clear(dev);
    ssd1306_show(dev);
}

void ssd1306_clear(ssd1306_t *dev) {
    for (int i = 0; i < sizeof(dev->buffer); ++i) {
        dev->buffer[i] = 0x00;
    }
}

void ssd1306_show(ssd1306_t *dev) {
    for (uint8_t page = 0; page < 8; page++) {
        ssd1306_send_cmd(dev, 0xB0 + page);
        ssd1306_send_cmd(dev, 0x00);
        ssd1306_send_cmd(dev, 0x10);

        uint8_t buf[129];
        buf[0] = 0x40;
        memcpy(&buf[1], &dev->buffer[page * 128], 128);
        i2c_write_blocking(dev->i2c, dev->address, buf, 129, false);
    }
}

void ssd1306_draw_string(ssd1306_t *dev, uint32_t x, uint32_t y, uint32_t scale, const char *s) {
    while (*s) {
        if (x + 8 >= dev->width) {
            x = 0;
            y += 8;
        }
        for (int i = 0; i < 8; ++i) {
            dev->buffer[x + (y / 8) * dev->width + i] = font8x8_basic[(uint8_t)*s][i];
        }
        x += 8;
        s++;
    }
}