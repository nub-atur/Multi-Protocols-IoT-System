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
 * @file ssd1306.c
 * @brief implementations of functions for setting up, interacting with and sending commands to an ssd1306 driven screen via spi or i2c
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "ssd1306.h"
#include "font8x8_basic.h"
#include "font8x8_readable.h"
#include "font8x8_readable_thin.h"
#include "font8x8_space.h"

#include <string.h>

#define tag CONFIG_SSD1306_TAG ///< tag for logging library

int center, top, bottom; ///< positions for oled

/**
 * initialise the screen device
 */
void ssd1306_init(SSD1306_t *dev)
{
#if CONFIG_I2C_INTERFACE
    i2c_master_init(dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
#endif // CONFIG_I2C_INTERFACE

#if CONFIG_SPI_INTERFACE
    spi_master_init(dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
#endif // CONFIG_SPI_INTERFACE
#if CONFIG_FLIP
    dev._flip = true;
#endif
    int width = 128;
    int height = 64;
#if CONFIG_SSD1306_128x64
    height = 64;
#endif // CONFIG_SSD1306_128x64
#if CONFIG_SSD1306_128x32
    height = 32;
#endif // CONFIG_SSD1306_128x32
    if (dev->_address == SPIAddress)
    {
        spi_init(dev, width, height);
    }
    else
    {
        i2c_init(dev, width, height);
    }
    ssd1306_clear_screen(dev, false);
    ssd1306_contrast(dev, 0xff);
#if CONFIG_SSD1306_128x64
    top = 2;
    center = 3;
    bottom = 8;
#endif // CONFIG_SSD1306_128x64
#if CONFIG_SSD1306_128x32
    top = 1;
    center = 1;
    bottom = 4;
#endif // CONFIG_SSD1306_128x32
    ssd1306_clear_screen(dev, false);
    ssd1306_contrast(dev, 0xff);
}
/**
 * de-initialise the screen device and free resources
 * @todo make sure to free more resources and de-init the actual spi/i2c session
 */
void ssd1306_deinit(SSD1306_t *dev)
{
    /// @todo
}
/**
 * display some text on the screen
 * @param dev the screen device to interact with
 * @param page the line to display the text on
 * @param text the text data
 * @param text_len the text length
 * @param invert whether or not to invert the text and background colours
 */
void ssd1306_display_text(SSD1306_t *dev, int page, char *text, int text_len, bool invert)
{
    if (page >= dev->_pages)
        return;
    int _text_len = text_len;
    if (_text_len > 16)
        _text_len = 16;

    uint8_t seg = 0;
    uint8_t image[8];
    for (uint8_t i = 0; i < _text_len; i++)
    {
        memcpy(image, font8x8[(uint8_t)text[i]], 8);
        if (invert)
            ssd1306_invert(image, 8);
        if (dev->_flip)
            ssd1306_flip(image, 8);
        if (dev->_address == SPIAddress)
        {
            spi_display_image(dev, page, seg, image, 8);
        }
        else
        {
            i2c_display_image(dev, page, seg, image, 8);
        }
        seg = seg + 8;
    }
}
/**
 * display some text on the screen
 * @param dev the screen device to interact with
 * @param line the line to display the text on
 * @param text the text data
 */
void ssd1306_wrapped_display_text(SSD1306_t *dev, int line, char *text)
{
    ssd1306_clear_line(dev, line, false);
    ssd1306_display_text(dev, line, text, strlen(text), false);
}
/**
 * display an image on the screen
 * @param dev the screen device to interact with
 * @param page the line to display the image on
 * @param seg the segment of the image
 * @param images the image data to display
 * @param width the width of the image
 */
void ssd1306_display_image(SSD1306_t *dev, int page, int seg, uint8_t *images, int width)
{
    if (dev->_address == SPIAddress)
    {
        spi_display_image(dev, page, seg, images, width);
    }
    else
    {
        i2c_display_image(dev, page, seg, images, width);
    }
}
/**
 * clear the screen (fills the screen with spaces)
 * @param dev the screen device to interact with
 * @param invert whether or not the cleared screen should be inverted
 */
void ssd1306_clear_screen(SSD1306_t *dev, bool invert)
{
    char space[16];
    memset(space, 0x20, sizeof(space));
    for (int page = 0; page < dev->_pages; page++)
    {
        ssd1306_display_text(dev, page, space, sizeof(space), invert);
    }
}
/**
 * clear a specific line (fills the line with spaces)
 * @param dev the screen device to interact with
 * @param page the line to clear
 * @param invert whether or not the cleared line should be inverted
 */
void ssd1306_clear_line(SSD1306_t *dev, int page, bool invert)
{
    char space[16];
    memset(space, 0x20, sizeof(space));
    ssd1306_display_text(dev, page, space, sizeof(space), invert);
}
/**
 * set the contrast of the screen
 * @param dev the screen device to interact with
 * @param contrast the level of contrast
 */
void ssd1306_contrast(SSD1306_t *dev, int contrast)
{
    if (dev->_address == SPIAddress)
    {
        spi_contrast(dev, contrast);
    }
    else
    {
        i2c_contrast(dev, contrast);
    }
}
/**
 * start scrolling the screen via software
 * @param dev the screen device to interact with
 * @param start the start of the scroll
 * @param end the end of the scroll
 */
void ssd1306_software_scroll(SSD1306_t *dev, int start, int end)
{
    ESP_LOGI(tag, "software_scroll start=%d end=%d _pages=%d", start, end, dev->_pages);
    if (start < 0 || end < 0)
    {
        dev->_scEnable = false;
    }
    else if (start >= dev->_pages || end >= dev->_pages)
    {
        dev->_scEnable = false;
    }
    else
    {
        dev->_scEnable = true;
        dev->_scStart = start;
        dev->_scEnd = end;
        dev->_scDirection = 1;
        if (start > end)
            dev->_scDirection = -1;
        for (int i = 0; i < dev->_pages; i++)
        {
            dev->_page[i]._valid = false;
            dev->_page[i]._segLen = 0;
        }
    }
}
/**
 * display some scrolling text on the screen
 * @param dev the screen device to interact with
 * @param text the text data
 * @param text_len the text length
 * @param invert whether or not the scrolling text should be inverted
 */
void ssd1306_scroll_text(SSD1306_t *dev, char *text, int text_len, bool invert)
{
    ESP_LOGI(tag, "dev->_scEnable=%d", dev->_scEnable);
    if (dev->_scEnable == false)
        return;

    void (*func)(SSD1306_t * dev, int page, int seg, uint8_t *images, int width);
    if (dev->_address == SPIAddress)
    {
        func = spi_display_image;
    }
    else
    {
        func = i2c_display_image;
    }

    int srcIndex = dev->_scEnd - dev->_scDirection;
    while (1)
    {
        int dstIndex = srcIndex + dev->_scDirection;
        ESP_LOGD(tag, "srcIndex=%d dstIndex=%d", srcIndex, dstIndex);
        dev->_page[dstIndex]._valid = dev->_page[srcIndex]._valid;
        dev->_page[dstIndex]._segLen = dev->_page[srcIndex]._segLen;
        for (int seg = 0; seg < dev->_width; seg++)
        {
            dev->_page[dstIndex]._segs[seg] = dev->_page[srcIndex]._segs[seg];
        }
        ESP_LOGD(tag, "_valid=%d", dev->_page[dstIndex]._valid);
        if (dev->_page[dstIndex]._valid)
            (*func)(dev, dstIndex, 0, dev->_page[dstIndex]._segs, dev->_page[srcIndex]._segLen);
        if (srcIndex == dev->_scStart)
            break;
        srcIndex = srcIndex - dev->_scDirection;
    }

    int _text_len = text_len;
    if (_text_len > 16)
        _text_len = 16;

    uint8_t seg = 0;
    uint8_t image[8];
    for (uint8_t i = 0; i < _text_len; i++)
    {
        memcpy(image, font8x8[(uint8_t)text[i]], 8);
        if (invert)
            ssd1306_invert(image, 8);
        if (dev->_flip)
            ssd1306_flip(image, 8);
        (*func)(dev, srcIndex, seg, image, 8);
        for (int j = 0; j < 8; j++)
            dev->_page[srcIndex]._segs[seg + j] = image[j];
        seg = seg + 8;
    }
    dev->_page[srcIndex]._valid = true;
    dev->_page[srcIndex]._segLen = seg;
}
/**
 * stop scrolling the screen via software
 * @param dev the screen device to interact with
 */
void ssd1306_scroll_clear(SSD1306_t *dev)
{
    ESP_LOGD(tag, "dev->_scEnable=%d", dev->_scEnable);
    if (dev->_scEnable == false)
        return;

    int srcIndex = dev->_scEnd - dev->_scDirection;
    while (1)
    {
        int dstIndex = srcIndex + dev->_scDirection;
        ESP_LOGD(tag, "srcIndex=%d dstIndex=%d", srcIndex, dstIndex);
        ssd1306_clear_line(dev, dstIndex, false);
        dev->_page[dstIndex]._valid = false;
        if (dstIndex == dev->_scStart)
            break;
        srcIndex = srcIndex - dev->_scDirection;
    }
}
/**
 * start scrolling the screen via hardware
 * @param dev the screen device to interact with
 * @param scroll the direction of the scroll
 */
void ssd1306_hardware_scroll(SSD1306_t *dev, ssd1306_scroll_type_t scroll)
{
    if (dev->_address == SPIAddress)
    {
        spi_hardware_scroll(dev, scroll);
    }
    else
    {
        i2c_hardware_scroll(dev, scroll);
    }
}
/**
 * start scrolling the screen via hardware
 * @param dev the screen device to interact with
 * @param page the specific page to scroll
 * @param scroll the direction of the scroll
 */
void ssd1306_hardware_scroll_line(SSD1306_t *dev, int page, ssd1306_scroll_type_t scroll)
{
    if (dev->_address == SPIAddress)
    {
        spi_hardware_scroll_line(dev, page, scroll);
    }
    else
    {
        i2c_hardware_scroll_line(dev, page, scroll);
    }
}
/**
 * invert a buffer of data
 * @param buf buffer to invert
 * @param blen the length of the buffer
 */
void ssd1306_invert(uint8_t *buf, size_t blen)
{
    uint8_t wk;
    for (int i = 0; i < blen; i++)
    {
        wk = buf[i];
        buf[i] = ~wk;
    }
}

// Flip upside down
/**
 * flip a buffer of data
 * @param buf buffer to flip
 * @param blen the length of the buffer
 */
void ssd1306_flip(uint8_t *buf, size_t blen)
{
    for (int i = 0; i < blen; i++)
    {
        buf[i] = ssd1306_rotate(buf[i]);
    }
}

// Rotate 8-bit data
// 0x12-->0x48
/**
 * rotate a buffer of data
 * @param buf buffer to rotate
 * @param blen the length of the buffer
 */
uint8_t ssd1306_rotate(uint8_t ch1)
{
    uint8_t ch2 = 0;
    for (int j = 0; j < 8; j++)
    {
        ch2 = (ch2 << 1) + (ch1 & 0x01);
        ch1 = ch1 >> 1;
    }
    return ch2;
}
/**
 * fadeout the screen
 * @param dev the screen device to interact with
 */
void ssd1306_fadeout(SSD1306_t *dev)
{
    void (*func)(SSD1306_t * dev, int page, int seg, uint8_t *images, int width);
    if (dev->_address == SPIAddress)
    {
        func = spi_display_image;
    }
    else
    {
        func = i2c_display_image;
    }

    uint8_t image[1];
    for (int page = 0; page < dev->_pages; page++)
    {
        image[0] = 0xFF;
        for (int line = 0; line < 8; line++)
        {
            if (dev->_flip)
            {
                image[0] = image[0] >> 1;
            }
            else
            {
                image[0] = image[0] << 1;
            }
            for (int seg = 0; seg < 128; seg++)
            {
                (*func)(dev, page, seg, image, 1);
            }
        }
    }
}
/**
 * dump the screen data
 * @param dev the screen device to interact with
 */
void ssd1306_dump(SSD1306_t dev)
{
    printf("_address=%x\n", dev._address);
    printf("_width=%x\n", dev._width);
    printf("_height=%x\n", dev._height);
    printf("_pages=%x\n", dev._pages);
}