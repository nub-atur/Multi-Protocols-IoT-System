/*
 * MIT License
 *
 * Copyright (c) 2020 nopnop2002
 * Copyright (c) 2021 wolffshots
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file ssd1306_spi.c
 * @brief implementations of functions for setting up, interacting with and sending commands to an ssd1306 driven screen via spi
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "ssd1306.h"

#define tag CONFIG_SSD1306_SPI_TAG ///< tag for logging library

#ifdef CONFIG_IDF_TARGET_ESP32
#define LCD_HOST HSPI_HOST
#define DMA_CHAN 2
#elif defined CONFIG_IDF_TARGET_ESP32S2
#define LCD_HOST SPI2_HOST
#define DMA_CHAN LCD_HOST
#endif

static const int SPI_Command_Mode = 0;
static const int SPI_Data_Mode = 1;
static const int SPI_Frequency = 1000000;

/**
 * initialise an spi device
 * @param dev the device to interact with
 * @param GPIO_MOSI the pin to use for GPIO_MOSI
 * @param GPIO_SCLK the pin to use for GPIO_SCLK
 * @param GPIO_CS the pin to use for GPIO_CS
 * @param GPIO_DC the pin to use for GPIO_DC
 * @param GPIO_RESET the pin to use for GPIO_RESET
 */
void spi_master_init(SSD1306_t *dev, int16_t GPIO_MOSI, int16_t GPIO_SCLK, int16_t GPIO_CS, int16_t GPIO_DC, int16_t GPIO_RESET)
{
	esp_err_t ret;

	esp_rom_gpio_pad_select_gpio(GPIO_CS);
	gpio_set_direction(GPIO_CS, GPIO_MODE_OUTPUT);
	gpio_set_level(GPIO_CS, 0);

	esp_rom_gpio_pad_select_gpio(GPIO_DC);
	gpio_set_direction(GPIO_DC, GPIO_MODE_OUTPUT);
	gpio_set_level(GPIO_DC, 0);

	if (GPIO_RESET >= 0)
	{
		esp_rom_gpio_pad_select_gpio(GPIO_RESET);
		gpio_set_direction(GPIO_RESET, GPIO_MODE_OUTPUT);
		gpio_set_level(GPIO_RESET, 0);
		vTaskDelay(pdMS_TO_TICKS(100));
		gpio_set_level(GPIO_RESET, 1);
	}

	spi_bus_config_t spi_bus_config = {
		.sclk_io_num = GPIO_SCLK,
		.mosi_io_num = GPIO_MOSI,
		.miso_io_num = -1,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1};

	ret = spi_bus_initialize(LCD_HOST, &spi_bus_config, DMA_CHAN);
	ESP_LOGI(tag, "spi_bus_initialize=%d", ret);
	assert(ret == ESP_OK);

	spi_device_interface_config_t devcfg;
	memset(&devcfg, 0, sizeof(spi_device_interface_config_t));
	devcfg.clock_speed_hz = SPI_Frequency;
	devcfg.spics_io_num = GPIO_CS;
	devcfg.queue_size = 1;

	spi_device_handle_t handle;
	ret = spi_bus_add_device(LCD_HOST, &devcfg, &handle);
	ESP_LOGI(tag, "spi_bus_add_device=%d", ret);
	assert(ret == ESP_OK);
	dev->_dc = GPIO_DC;
	dev->_SPIHandle = handle;
	dev->_address = SPIAddress;
	dev->_flip = false;
}

/**
 * write a byte to the spi device
 * @param SPIHandle the spi device to interact with
 * @param Data the data being sent
 * @param DataLength the length of the data being sent
 */
bool spi_master_write_byte(spi_device_handle_t SPIHandle, const uint8_t *Data, size_t DataLength)
{
	spi_transaction_t SPITransaction;

	if (DataLength > 0)
	{
		memset(&SPITransaction, 0, sizeof(spi_transaction_t));
		SPITransaction.length = DataLength * 8;
		SPITransaction.tx_buffer = Data;
		spi_device_transmit(SPIHandle, &SPITransaction);
	}

	return true;
}

/**
 * write a command to the screen
 * @param dev the screen device to interact with
 * @param Command the command being sent
 */
bool spi_master_write_command(SSD1306_t *dev, uint8_t Command)
{
	static uint8_t CommandByte = 0;
	CommandByte = Command;
	gpio_set_level(dev->_dc, SPI_Command_Mode);
	return spi_master_write_byte(dev->_SPIHandle, &CommandByte, 1);
}

/**
 * write data to the screen
 * @param dev the screen device to interact with
 * @param Data the data being sent
 * @param DataLength the length of the data being sent
 */
bool spi_master_write_data(SSD1306_t *dev, const uint8_t *Data, size_t DataLength)
{
	gpio_set_level(dev->_dc, SPI_Data_Mode);
	return spi_master_write_byte(dev->_SPIHandle, Data, DataLength);
}

/**
 * initialise the screen spi session
 * @param dev the screen device to interact with
 * @param width the width of the screen
 * @param height the height of the screen
 */
void spi_init(SSD1306_t *dev, int width, int height)
{
	dev->_width = width;
	dev->_height = height;
	dev->_pages = 8;
	if (dev->_height == 32)
		dev->_pages = 4;

	spi_master_write_command(dev, OLED_CMD_DISPLAY_OFF);   // AE
	spi_master_write_command(dev, OLED_CMD_SET_MUX_RATIO); // A8
	if (dev->_height == 64)
		spi_master_write_command(dev, 0x3F);
	if (dev->_height == 32)
		spi_master_write_command(dev, 0x1F);
	spi_master_write_command(dev, OLED_CMD_SET_DISPLAY_OFFSET); // D3
	spi_master_write_command(dev, 0x00);
	spi_master_write_command(dev, OLED_CONTROL_BYTE_DATA_STREAM); // 40
	if (dev->_flip)
	{
		spi_master_write_command(dev, OLED_CMD_SET_SEGMENT_REMAP_0); // A0
	}
	else
	{
		spi_master_write_command(dev, OLED_CMD_SET_SEGMENT_REMAP_1); // A1
	}
	//spi_master_write_command(dev, OLED_CMD_SET_SEGMENT_REMAP);		// A1
	spi_master_write_command(dev, OLED_CMD_SET_COM_SCAN_MODE);	 // C8
	spi_master_write_command(dev, OLED_CMD_SET_DISPLAY_CLK_DIV); // D5
	spi_master_write_command(dev, 0x80);
	spi_master_write_command(dev, OLED_CMD_SET_COM_PIN_MAP); // DA
	if (dev->_height == 64)
		spi_master_write_command(dev, 0x12);
	if (dev->_height == 32)
		spi_master_write_command(dev, 0x02);
	spi_master_write_command(dev, OLED_CMD_SET_CONTRAST); // 81
	spi_master_write_command(dev, 0xFF);
	spi_master_write_command(dev, OLED_CMD_DISPLAY_RAM);	   // A4
	spi_master_write_command(dev, OLED_CMD_SET_VCOMH_DESELCT); // DB
	spi_master_write_command(dev, 0x40);
	spi_master_write_command(dev, OLED_CMD_SET_MEMORY_ADDR_MODE); // 20
	//spi_master_write_command(dev, OLED_CMD_SET_HORI_ADDR_MODE);	// 00
	spi_master_write_command(dev, OLED_CMD_SET_PAGE_ADDR_MODE); // 02
	// Set Lower Column Start Address for Page Addressing Mode
	spi_master_write_command(dev, 0x00);
	// Set Higher Column Start Address for Page Addressing Mode
	spi_master_write_command(dev, 0x10);
	spi_master_write_command(dev, OLED_CMD_SET_CHARGE_PUMP); // 8D
	spi_master_write_command(dev, 0x14);
	spi_master_write_command(dev, OLED_CMD_DEACTIVE_SCROLL); // 2E
	spi_master_write_command(dev, OLED_CMD_DISPLAY_NORMAL);	 // A6
	spi_master_write_command(dev, OLED_CMD_DISPLAY_ON);		 // AF
}

/**
 * display an image on the screen
 * @param dev the screen device to interact with
 * @param page the line to display the image on
 * @param seg the segment of the image
 * @param images the image data to display
 * @param width the width of the image
 */
void spi_display_image(SSD1306_t *dev, int page, int seg, uint8_t *images, int width)
{
	if (page >= dev->_pages)
		return;
	if (seg >= dev->_width)
		return;

	int _seg = seg + CONFIG_OFFSETX;
	uint8_t columLow = _seg & 0x0F;
	uint8_t columHigh = (_seg >> 4) & 0x0F;

	int _page = page;
	if (dev->_flip)
	{
		_page = (dev->_pages - page) - 1;
	}

	// Set Lower Column Start Address for Page Addressing Mode
	spi_master_write_command(dev, (0x00 + columLow));
	// Set Higher Column Start Address for Page Addressing Mode
	spi_master_write_command(dev, (0x10 + columHigh));
	// Set Page Start Address for Page Addressing Mode
	spi_master_write_command(dev, 0xB0 | _page);

	spi_master_write_data(dev, images, width);
}

/**
 * set the contrast of the screen
 * @param dev the screen device to interact with
 * @param contrast the level of contrast
 */
void spi_contrast(SSD1306_t *dev, int contrast)
{
	int _contrast = contrast;
	if (contrast < 0x0)
		_contrast = 0;
	if (contrast > 0xFF)
		_contrast = 0xFF;

	spi_master_write_command(dev, OLED_CMD_SET_CONTRAST); // 81
	spi_master_write_command(dev, _contrast);
}

/**
 * send the command for a continues scroll via hardware
 * @param dev the screen device to interact with
 * @param scroll the direction of the scroll
 */
void spi_hardware_scroll(SSD1306_t *dev, ssd1306_scroll_type_t scroll)
{

	if (scroll == SCROLL_RIGHT)
	{
		spi_master_write_command(dev, OLED_CMD_HORIZONTAL_RIGHT); // 26
		spi_master_write_command(dev, 0x00);					  // Dummy byte
		spi_master_write_command(dev, 0x00);					  // Define start page address
		spi_master_write_command(dev, CONFIG_SSD1306_FRAME_FREQ); // Frame frequency
		spi_master_write_command(dev, 0x07);					  // Define end page address
		spi_master_write_command(dev, 0x00);					  //
		spi_master_write_command(dev, 0xFF);					  //
		spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL);	  // 2F
	}

	if (scroll == SCROLL_LEFT)
	{
		spi_master_write_command(dev, OLED_CMD_HORIZONTAL_LEFT);  // 27
		spi_master_write_command(dev, 0x00);					  // Dummy byte
		spi_master_write_command(dev, 0x00);					  // Define start page address
		spi_master_write_command(dev, CONFIG_SSD1306_FRAME_FREQ); // Frame frequency
		spi_master_write_command(dev, 0x07);					  // Define end page address
		spi_master_write_command(dev, 0x00);					  //
		spi_master_write_command(dev, 0xFF);					  //
		spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL);	  // 2F
	}

	if (scroll == SCROLL_DOWN)
	{
		spi_master_write_command(dev, OLED_CMD_CONTINUOUS_SCROLL); // 29
		spi_master_write_command(dev, 0x00);					   // Dummy byte
		spi_master_write_command(dev, 0x00);					   // Define start page address
		spi_master_write_command(dev, CONFIG_SSD1306_FRAME_FREQ);  // Frame frequency
		//spi_master_write_command(dev, 0x01); // Define end page address
		spi_master_write_command(dev, 0x00); // Define end page address
		spi_master_write_command(dev, 0x3F); // Vertical scrolling offset

		spi_master_write_command(dev, OLED_CMD_VERTICAL); // A3
		spi_master_write_command(dev, 0x00);
		if (dev->_height == 64)
			spi_master_write_command(dev, 0x40);
		if (dev->_height == 32)
			spi_master_write_command(dev, 0x20);
		spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL); // 2F
	}

	if (scroll == SCROLL_UP)
	{
		spi_master_write_command(dev, OLED_CMD_CONTINUOUS_SCROLL); // 29
		spi_master_write_command(dev, 0x00);					   // Dummy byte
		spi_master_write_command(dev, 0x00);					   // Define start page address
		spi_master_write_command(dev, CONFIG_SSD1306_FRAME_FREQ);  // Frame frequency
		//spi_master_write_command(dev, 0x01); // Define end page address
		spi_master_write_command(dev, 0x00); // Define end page address
		spi_master_write_command(dev, 0x01); // Vertical scrolling offset

		spi_master_write_command(dev, OLED_CMD_VERTICAL); // A3
		spi_master_write_command(dev, 0x00);
		if (dev->_height == 64)
			spi_master_write_command(dev, 0x40);
		if (dev->_height == 32)
			spi_master_write_command(dev, 0x20);
		spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL); // 2F
	}

	if (scroll == SCROLL_STOP)
	{
		spi_master_write_command(dev, OLED_CMD_DEACTIVE_SCROLL); // 2E
	}
}

/**
 * send the command for a continues scroll via hardware
 * @param dev the screen device to interact with
 * @param page the specific page to scroll
 * @param scroll the direction of the scroll
 */
void spi_hardware_scroll_line(SSD1306_t *dev, int page, ssd1306_scroll_type_t scroll)
{
	if (scroll == SCROLL_RIGHT)
	{
		spi_master_write_command(dev, OLED_CMD_HORIZONTAL_RIGHT); // 26
		spi_master_write_command(dev, 0x00);					  // Dummy byte
		spi_master_write_command(dev, page);					  // Define start page address
		spi_master_write_command(dev, CONFIG_SSD1306_FRAME_FREQ); // Frame frequency
		spi_master_write_command(dev, page);					  // Define end page address
		spi_master_write_command(dev, 0x00);					  //
		spi_master_write_command(dev, 0xFF);					  //
		spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL);	  // 2F
	}

	if (scroll == SCROLL_LEFT)
	{
		spi_master_write_command(dev, OLED_CMD_HORIZONTAL_LEFT);  // 27
		spi_master_write_command(dev, 0x00);					  // Dummy byte
		spi_master_write_command(dev, page);					  // Define start page address
		spi_master_write_command(dev, CONFIG_SSD1306_FRAME_FREQ); // Frame frequency
		spi_master_write_command(dev, page);					  // Define end page address
		spi_master_write_command(dev, 0x00);					  //
		spi_master_write_command(dev, 0xFF);					  //
		spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL);	  // 2F
	}

	if (scroll == SCROLL_STOP)
	{
		spi_master_write_command(dev, OLED_CMD_DEACTIVE_SCROLL); // 2E
	}
}