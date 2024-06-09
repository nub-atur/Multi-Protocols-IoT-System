#include "ws2812.h"
#include "main.h"

rmt_item32_t bitOn = {{{T1H, 1, T1L, 0}}};
rmt_item32_t bitOff = {{{T0H, 1, T0L, 0}}};

rmt_item32_t led_data_buffer[NUM_ROWS][LED_BUFFER_ITEMS];
rmt_config_t config[NUM_CHANS];

uint8_t pCount = 0;

uint8_t ledDataCount[NUM_ROWS*NUM_COLS][BYTES_PER_LED];   //Define the LED's data one by one.

uint8_t pixels[NUM_ROWS * NUM_COLS * BYTES_PER_LED]; //chatgpt is too dump cheap...

gpio_num_t pins[NUM_CHANS] = {LED_PINS};   //Maybe we have a bunch of used pins.

void ws2812_control_init(gpio_num_t *pins) {
    for (int chan = 0; chan < NUM_CHANS; chan++) {
		config[chan].rmt_mode = RMT_MODE_TX;
		config[chan].channel = (rmt_channel_t)(RMT_CHANNEL_0 + chan);
		config[chan].gpio_num = (gpio_num_t)pins[chan];
		config[chan].mem_block_num = 1;
		config[chan].tx_config.loop_en = false;
		config[chan].tx_config.carrier_en = false;
		config[chan].tx_config.idle_output_en = true;
		config[chan].tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
		config[chan].clk_div = 2;

		rmt_config(&config[chan]);
		rmt_driver_install(config[chan].channel, 0, 0);
	}
}

void setup_rmt_data_buffer(uint8_t *pixels) {
    // Prepare LED data buffer for RMT
    for (int row = 0; row < NUM_ROWS; row++) {
		int bit = 0;
		for (int col = 0; col < NUM_COLS; col++) {
			for (int b = 0; b < BYTES_PER_LED; b++) {
				uint8_t bits_to_send = *pixels++;
				uint8_t mask = 0x80;
				for (uint8_t i = 0; i < 8; i++) {
					led_data_buffer[row][bit++] = (bits_to_send & mask) ? bitOn : bitOff;
					mask >>= 1;
				}
			}
		}
	}
}

void ws2812_write_leds(uint8_t *pixels, uint16_t rowLen, gpio_num_t *pins) {
    // Write LED data to RMT peripheral
    int rem = NUM_ROWS;
	int off = 0;
	int cnt = NUM_ROWS < NUM_CHANS ? NUM_ROWS : NUM_CHANS;

	setup_rmt_data_buffer(pixels);

	while (rem > 0) {
		for (int chan = 0; chan < cnt; chan++) {
			int pin = pins[off + chan];
			//is there an rmt_clear_pin?
//			ESP_ERROR_CHECK(rmt_set_pin(RMT_CHANNEL_0 + chan, RMT_MODE_TX, pin));

			gpio_set_direction(pin, GPIO_MODE_OUTPUT);
		    rmt_set_gpio(RMT_CHANNEL_0+chan, RMT_MODE_TX, pin, 0);
		}
		for (int chan = 0; chan < cnt; chan++) {
			rmt_write_items((rmt_channel_t)(RMT_CHANNEL_0 + chan), led_data_buffer[off + chan], LED_BUFFER_ITEMS, false);
			rem--;
		}
		for (int chan = 0; chan < cnt; chan++) {
			rmt_wait_tx_done((rmt_channel_t)(RMT_CHANNEL_0 + chan), portMAX_DELAY);
		}
		for (int chan = 0; chan < cnt; chan++) {
			int pin = pins[off + chan];
		    gpio_reset_pin(pin);
		}
		off += cnt;
		cnt = rem < NUM_CHANS ? rem : NUM_CHANS;
	}
}

void bitbang_send_pixels_800(uint8_t* pixels, uint16_t rowLen, uint8_t numPins, gpio_num_t* pins) {
    ws2812_write_leds(pixels, rowLen, pins);
}

void bitbang_initialize(gpio_num_t *pins) {
    ws2812_control_init(pins);
}

void setLedColor(uint8_t index, uint8_t green, uint8_t red, uint8_t blue) { //my led is GRB, that's why i config color like this. you should change this following ur specifics
    if (index < NUM_COLS*NUM_ROWS) {
        ledDataCount[index][0] = green;
        ledDataCount[index][1] = red;
        ledDataCount[index][2] = blue;
    }
    pixels[pCount] = ledDataCount[index][0];
    pixels[pCount++] = ledDataCount[index][1];
    pixels[pCount++] = ledDataCount[index][2];

    pCount++;
    if (pCount == NUM_ROWS * NUM_COLS * BYTES_PER_LED) pCount = 0 ;
}

void led_act(){
     bitbang_send_pixels_800(pixels, NUM_ROWS, NUM_CHANS, pins);
}