    /*+----------+
      |          |
      |   ESP    |----GPIO------->ledDataCount1{pixels[0], pixels[1], pixels[2]}---------------+
      |          |                                                                             |
      |          |             +---ledDataCount2{pixels[3], pixels[4], pixels[5]}<-------------+
      +----------+            |
                              +--->ledDataCount2{pixels[6], pixels[7], pixels[8]}-----------> ..........
  */
#include "mydata.h"
#include <stdio.h>
#include "main.h"
#include "TIM_RTOS.h"

#include "driver/gpio.h"
#include "driver/timer.h"

#include "freertos/queue.h"
#include <freertos/portmacro.h>

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_mac.h"
#include "esp_eth.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_http_client.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include <esp_wifi_types.h>

// #define ENABLE_LOGGING 0
#define QUEUE_LENGTH 10
#define ITEM_SIZE sizeof(uint8_t)
#define INPUT_PIN GPIO_NUM_34

#define PORT 8888
#define HOST_IP_ADDR "192.168.1.4"
static const char *TAG = "UDP SOCKET CLIENT";
static const char *payload = "M";

//----------------------------SOME TASKS-------------------------------------------
TaskHandle_t rainBow;
TaskHandle_t flop;
TaskHandle_t readInput;
QueueHandle_t myQueue;
TaskHandle_t UDP;

static void udp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1) {
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }

        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        ESP_LOGI(TAG, "Socket created, sending to %s:%d", host_ip, PORT);

        while (1) {

            int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Message sent");

            struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);
                if (strncmp(rx_buffer, "OK: ", 4) == 0) {
                    ESP_LOGI(TAG, "Received expected message, reconnecting");
                    break;
                }
            }
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    // vTaskDelete(NULL);
}

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting WIFI_EVENT_STA_START ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected WIFI_EVENT_STA_CONNECTED ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection WIFI_EVENT_STA_DISCONNECTED ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

void wifi_connection()
{
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_wifi_connect();
}

void vRainBow(void *pvParameter)
{
    uint8_t green;
    uint8_t red;
    uint8_t blue;
    uint16_t i;
    uint16_t j = 0;
    // uint32_t rxData;
    uint16_t res = 200;
	while(1)
	{  
        // if (xQueueReceive(myQueue, &rxData, portMAX_DELAY))
        // {
        //     res = (rxData > (uint16_t)(4095/2))? 1:1000;
        // }
        
        for (i = 0; i < 256; ++i) 
        {
            if (j == NUM_COLS*NUM_ROWS) j = 0; 

            // Red goes from 0 to 255
            red = i;
            // Green goes from 0 to 255
            green = (i + 85) % 256; // 85 is 1/3 of 255
            // Blue goes from 255 to 0
            blue = (256 - i) % 256;

            // Print color
            setLedColor(j, green, red, blue);
            led_act();

            j++;
            // Sleep for a short duration to control the speed of the effect
            vTaskDelay(res / portTICK_PERIOD_MS);
        }
        // vTaskDelay(res / portTICK_PERIOD_MS);
	}
}

void vFlop(void *pvParameter)
{
    uint16_t res = 1000;
    for(;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        vTaskSuspend(rainBow);

        for (uint8_t i=0; i< NUM_COLS*NUM_ROWS; i++)
        {
            setLedColor(i, 0, 0, 0);
            led_act();
        }
        vTaskDelay(res / portTICK_PERIOD_MS);
    }
}

void vReadLM393(void *pvParameter)
{
    uint32_t input;

    adc1_config_width(ADC_WIDTH_BIT_12);

    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); //attenuation

    for(;;)
    {
        input = adc1_get_raw(ADC1_CHANNEL_6);
        printf("%ld\n", input);
        xQueueSend(myQueue, &input, 10);
        vTaskResume(rainBow);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

//------------------------------------------------------------------------
void app_main() 
{   
    esp_log_level_set("wifi_init", ESP_LOG_NONE);
    esp_log_level_set("gpio", ESP_LOG_NONE);

    wifi_connection();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    
    Init_TIM_RTOS();

    bitbang_initialize(pins);

    myQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);

    xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 4, &UDP);
    xTaskCreate(vRainBow, "rainbow", 1024*3, NULL, 0, &rainBow);
    // xTaskCreate(vReadLM393, "read", 1024*3, NULL, 2, &readInput);
    // xTaskCreate(vFlop, "jumping", 1024*3, NULL, 1, &flop);
}






// #define ADC_SAMPLES_COUNT 1000
// int16_t abuf[ADC_SAMPLES_COUNT];
// int16_t abufPos = 0;
//---------------------------SamplingCalc---------------------------
// portMUX_TYPE DRAM_ATTR timerMux = portMUX_INITIALIZER_UNLOCKED; 
// TaskHandle_t complexHandlerTask;
// timer_t * adcTimer = NULL; // our timer

// void complexHandler(void *param) {
//   while (true) {
//     // Sleep until the ISR gives us something to do, or for 1 second
//     uint32_t tcount = ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(1000));  
//     if (check_for_work) {
//       // Do something complex and CPU-intensive
//     }
//   }
// }

// void IRAM_ATTR onTimer(void *args) {
//   portENTER_CRITICAL_ISR(&timerMux);

//   abuf[abufPos++] = local_adc1_read(ADC1_CHANNEL_0);
  
//   if (abufPos >= ADC_SAMPLES_COUNT) { 
//     abufPos = 0;

//     // Notify adcTask that the buffer is full.
//     BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//     vTaskNotifyGiveFromISR(flop, &xHigherPriorityTaskWoken);
//     if (xHigherPriorityTaskWoken) {
//       portYIELD_FROM_ISR();
//     }
//   }
//   portEXIT_CRITICAL_ISR(&timerMux);
// }

// int IRAM_ATTR local_adc1_read(int channel) {
//     uint16_t adc_value;
//     SENS.sar_meas_start1.sar1_en_pad = (1 << channel); // only one channel is selected
//     while (SENS.sar_slave_addr1.meas_status != 0);
//     SENS.sar_meas_start1.meas1_start_sar = 0;
//     SENS.sar_meas_start1.meas1_start_sar = 1;
//     while (SENS.sar_meas_start1.meas1_done_sar == 0);
//     adc_value = SENS.sar_meas_start1.meas1_data_sar;
//     return adc_value;
// }
