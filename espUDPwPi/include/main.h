#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ws2812.h>

#define ACK_DEFAULT 0
#define ACK_OK      1
#define ACK_KO      1

#define WIFI_ID     "Mi 9T"
#define WIFI_PASS   "12345678"
#define UDP_HOST_ID "192.168.43.200"
#define UDP_PORT    8888
#define TAG         "UDP"

#define CONFIG_SSID_FONT_BASIC
#define CONFIG_SSD1306_I2C_TAG "es"

#define CONFIG_SSD1306_SPI_TAG "es"
#define CONFIG_SSD1306_TAG "es"
#define CONFIG_OFFSETX	0

#define CONFIG_SSD1306_FRAME_FREQ CONFIG_ESP32_XTAL_FREQ


#ifdef __cplusplus
}
#endif

#endif