#include <stdio.h>
#include "TIM_RTOS.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "sdkconfig.h"

// #define LED_TOGGLE_STACK 1000 // 1kB Stack size
// #define LED_GPIO1 25 // Using GPIO25
// #define LED_GPIO2 26 // Using GPIO26
// #define LED_TOGGLE_PRIO 5 // Priority one
// #define LED_DELAY1 500 // 500 ms delay between toggles
#define LED_DELAY2 5000  // 100 ms delay between toggles

// void LED_Toggle_wDelay(void *args) // Led Toggle task
// {
//     gpio_set_level(LED_GPIO1, !gpio_get_level(LED_GPIO1)); // Toggle LED
//     vTaskDelay(LED_DELAY1/portTICK_PERIOD_MS); // 100 ms delay between toggles
// }

static void LED_Toggle_wTimer(TimerHandle_t mypxTimer) // Timer callback handler
{
    // gpio_set_level(LED_GPIO2, !gpio_get_level(LED_GPIO2)); // Toggle LED

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(flop, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void Init_TIM_RTOS(void)
{
    // gpio_set_direction(LED_GPIO1, GPIO_MODE_INPUT_OUTPUT); // Setting GPIO25 as both input and output
    // gpio_set_direction(LED_GPIO2, GPIO_MODE_INPUT_OUTPUT); // Setting GPIO26 as both input and output
    // Setting up timers with given frequencies and handler
    // xTaskCreate(LED_Toggle_wDelay, "Toggle LED", LED_TOGGLE_STACK, NULL, LED_TOGGLE_PRIO, NULL);
    TimerHandle_t wTimer = xTimerCreate("Led Timer", (LED_DELAY2/portTICK_PERIOD_MS) , pdTRUE, ( void * ) 1, LED_Toggle_wTimer);
    xTimerStart(wTimer, 0); // Initiating timer
}
