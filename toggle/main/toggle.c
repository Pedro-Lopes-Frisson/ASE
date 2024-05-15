#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define TAG "led toggle"
#define BLINK_GPIO 5
#define CONFIG_BLINK_PERIOD 50
static int s_led_state = 0;

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}
static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}


void app_main(void)
{

    /* Configure the peripheral according to the LED type */
    configure_led();

    while (1) {
        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == 0 ? "ON" : "OFF");
        blink_led();
        /* Toggle the LED state */
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        if(s_led_state == 0){
            s_led_state = 1;
        }
        else {
            s_led_state = 0;
        }
    }
}
