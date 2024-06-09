#include <stdio.h>
#include <stdint.h>
#include <main.h>   
#include <esp_log.h>
#include <esp_wifi.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "freertos/event_groups.h"
#include <freertos/portmacro.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/i2c.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include <ssd1306.h>

#define BUF_SIZE 1024
#define QUEUE_LENGTH 5
#define ITEM_SIZE sizeof(char[32])

volatile uint8_t ackFlag = ACK_DEFAULT;

SemaphoreHandle_t cast;
TaskHandle_t rainBow;
TaskHandle_t flop;
TaskHandle_t udpComm;
TaskHandle_t handleACK;
QueueHandle_t myQueue;

//-----------------------------------TASKS & QUEUES------------------------
void vHandleACK(void *pvParamater){
    int addr_family = 0;
    int ip_protocol = 0;

    while(1){
        if (xSemaphoreTake(cast, portMAX_DELAY) == pdPASS){
            vTaskSuspend(udpComm);
            struct sockaddr_in dest_addr;
            dest_addr.sin_addr.s_addr = inet_addr(UDP_HOST_ID);
            dest_addr.sin_family = AF_INET;
            dest_addr.sin_port = htons(UDP_PORT);
            addr_family = AF_INET;
            ip_protocol = IPPROTO_IP;

            int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
            if (sock < 0) {
                ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
                break;
            }

            // char* res = returnACK(ackFlag, myQueue);
            // Allocate memory for the result
            char* result = malloc(32);  // Adjust size based on expected usage

            char rxData[32];
            if (xQueueReceive(myQueue, &rxData, portMAX_DELAY) == pdPASS) {
                strcpy(result, rxData);  
            }

            // Set the initial value of result
            strcat(result, (ackFlag == ACK_OK) ? "OK" : "KO");

            int err = sendto(sock, result, strlen(result), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            }
            ESP_LOGI(TAG, "Message ACK sent");
            vTaskResume(udpComm);
            vTaskDelete(NULL);
        }
    }
}

void vUdpComm (void *pvParameter){
    char rx_buffer[32];
    char host_ip[] = UDP_HOST_ID;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1) {
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(UDP_HOST_ID);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(UDP_PORT);
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

        // ESP_LOGI(TAG, "Socket created, sending to %s:%d", host_ip, UDP_PORT);

        while (1) {
            char buf[100] = "UDP_count = ";
            char tick_str[20];
            uint32_t ticks = (float)xTaskGetTickCount()/configTICK_RATE_HZ;
            snprintf(tick_str, sizeof(tick_str), "%lu", ticks);
            strcat(buf, tick_str);

            int err = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
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
                xQueueSend(myQueue, rx_buffer, 0);

                int err = sendto(sock, "OK", strlen("OK"), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                }
                ESP_LOGI(TAG, "Message ACK sent");

                break;
            }
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

void vFlop(void *pvParameter)
{
    char rxData[32];
    SSD1306_t dev;
    // PAGE_t pages;
    ssd1306_init(&dev);
    i2c_master_init(&dev, GPIO_NUM_21, GPIO_NUM_22, -1);
    i2c_init(&dev, 128, 64);
    
    char *welcome_text = "Hello, ESP32!";
    ssd1306_display_text(&dev, 0, welcome_text, strlen(welcome_text), false);

    // Clear the screen
    ssd1306_clear_screen(&dev, false);

    while(1) {
        uint32_t ticks = (float)xTaskGetTickCount()/configTICK_RATE_HZ;
        char str[10]; // Enough to hold 8 digits + null terminator for uint32_t
    
        // Convert uint32_t to string
        sprintf(str, "%lu", ticks);

        ssd1306_display_text(&dev, 5, "LINE 5", strlen("LINE 5"), false);
        // vTaskDelay(500 / portTICK_PERIOD_MS);
        // ssd1306_clear_screen(&dev, false);

        ssd1306_display_text(&dev, 2, "UDP count: ", strlen("UDP count: "), false);
        ssd1306_display_text(&dev, 3, str, strlen(str), false);

        // vTaskDelay(500 / portTICK_PERIOD_MS);
        // ssd1306_clear_screen(&dev, false);
        
        if (xQueueReceive(myQueue, &rxData, portMAX_DELAY) == pdPASS)
        {
            ssd1306_clear_screen(&dev, false);
            ssd1306_display_text(&dev, 0, rxData, strlen(rxData), false);
            // ackFlag = ACK_OK; 
            // xSemaphoreGive(cast);
            // vTaskDelay(500/portTICK_PERIOD_MS);
        } 
        vTaskDelay(50 / portTICK_PERIOD_MS);
	}
	ssd1306_deinit(&dev);
}

//-----------------------------------FUNCTIONS-----------------------------
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
            .ssid = WIFI_ID,
            .password = WIFI_PASS}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_wifi_connect();
}


// char* returnACK(uint8_t flag, QueueHandle_t myQueue){
//     // Allocate memory for the result
//     char* result = malloc(32);  // Adjust size based on expected usage
//     if (result == NULL) {
//         // Handle malloc failure
//         return NULL;
//     }

//     char rxData[32];
//     if (xQueueReceive(myQueue, &rxData, portMAX_DELAY) == pdPASS) {
//         strcpy(result, rxData);  
//     }

//     // Set the initial value of result
//     strcat(result, (flag == ACK_OK) ? "OK" : "KO");

//     return result;
// }

//==============================================================================================================================================

void app_main() {
        wifi_connection();

        // cast = xSemaphoreCreateBinary();
        // if (cast == NULL) {
        //     ESP_LOGE("Semaphore", "Semaphore creation failed");
        //     return;
        // }

        // esp_log_level_set("gpio", ESP_LOG_NONE);

        // bitbang_initialize(pins);

        myQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);

        // xTaskCreate(vRainBow, "rainbow", 1024*3, NULL, 1, &rainBow);
        // xTaskCreate(vHandleACK, "ACK", 1024*3, NULL, 0, &handleACK);
        xTaskCreate(vFlop, "jumping", 1024*4, NULL, 1, &flop);
        xTaskCreate(vUdpComm, "udp_client", 4096, NULL, 0, &udpComm);
}

