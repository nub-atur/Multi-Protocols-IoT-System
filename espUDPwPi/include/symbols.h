#ifndef SSD1306_SYMBOLS
#define SSD1306_SYMBOLS

#include <main.h>
// custom symbols
static uint8_t degree_symbol[8] = {
    0x00,
    0x00,
    0x00,
    0x0C,
    0x12,
    0x12,
    0x0C,
    0x00,
};
static uint8_t up_arrow[8] = {
    0x00,
    0x04,
    0x06,
    0x7F,
    0x06,
    0x04,
    0x00,
    0x00,
};
static uint8_t down_arrow[8] = {
    0x00,
    0x10,
    0x30,
    0x7F,
    0x30,
    0x10,
    0x00,
    0x00,
};
static uint8_t fire_symbol[8] = {
    0x00,
    0x72,
    0xB9,
    0x0C,
    0xC7,
    0x7E,
    0x38,
    0x02,
};
static uint8_t snowflake_symbol[8] = {
    0x42,
    0xDB,
    0x24,
    0x52,
    0x4A,
    0x24,
    0xDB,
    0x42,
};

#endif // SSD1306_SYMBOLS