#ifndef _WS2812_H_
#define _WS2812_H_

#include "driver/gpio.h"
#include "driver/rmt.h"
#include <driver/rmt_types_legacy.h>
#include <driver/adc_types_legacy.h>
#include <driver/adc.h>

#include <stdio.h>
#include <inttypes.h>
#include <main.h>

#define LED_PINS GPIO_NUM_5
#define SENSOR_PIN 5       //the sensor pin
#define NUM_ROWS 1
#define NUM_COLS 16
#define NUM_CHANS 1
#define BYTES_PER_LED 3
#define BITS_PER_LED 24
#define LED_BUFFER_ITEMS ((NUM_COLS * BITS_PER_LED))

#define T0H 16      // count times following datasheet of led
#define T1H 32      // 1 pulse time = 1/(40MHz) * count
#define T0L 34      // where setting the data pin at clock div 2, default source clock ESP32 = 80MHz
#define T1L 18

extern gpio_num_t pins[NUM_CHANS];

//--------------functions--------------------
void ws2812_control_init(gpio_num_t *pins);
void setup_rmt_data_buffer(uint8_t *pixels);
// void castArray(uint8_t ledDataCount[NUM_ROWS*NUM_COLS][BYTES_PER_LED]); 
void ws2812_write_leds(uint8_t *pixels, uint16_t rowLen, gpio_num_t *pins);
void bitbang_send_pixels_800(uint8_t* pixels, uint16_t rowLen, uint8_t numPins, gpio_num_t* pins); 
void bitbang_initialize(gpio_num_t *pins); 
void setLedColor(uint8_t index, uint8_t green, uint8_t red, uint8_t blue); //my led is GRB, that's why i config color like this. you should change this following ur specifics
void led_act();

#endif //_WS2812_H_