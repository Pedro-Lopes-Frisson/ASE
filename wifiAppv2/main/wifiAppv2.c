#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_eap_client.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "include/mqtt_client_ase.h"
#include "TempSensorTC74.h"
#include "include/wifiv2.h"
#include "esp_timer.h"
#include <strings.h>
#define TC74ADDR 0x49

static uint8_t pTemp;
static i2c_master_bus_handle_t pBusHandle;
static i2c_master_dev_handle_t pSensorHandle;

static void default_handler1(esp_mqtt_event_handle_t event)
{
    printf("1TOPI1C=%.*s\r\n", event->topic_len, event->topic);
    printf("1DATA1=%.*s\r\n", event->data_len, event->data);
}
void check_temp(void)
{
    tc74_read_temp_after_temp(pSensorHandle, &pTemp);
    printf("\rTemperature was %u C", pTemp);
    char msg[80];
    snprintf(&msg, 80, "{\"Timestamp\": %lld ,\"Temperature\":%u}",(long long int)10, pTemp);
    mqtt_publish("esp32c3", (char*)&msg);
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    initialise_wifi();

    struct Event_Handler e =
        {
            .data_callback = default_handler1,
        };

    while (isWifiConnected() != 1)
    {
        vTaskDelay(1000);
    }

    printf("Connected\n stating mqtt\n");
    while (mqtt_app_start(&e) != ESP_OK)
    {
        vTaskDelay(1000);
    }

    printf("MQTT HAS STARTED\n");
    mqtt_subscribe("ola");

    tc74_init(&pBusHandle, &pSensorHandle, TC74ADDR, 1, 0, 100000);
    // test read_temp_after_temp
    tc74_wakeup_and_read_temp(pSensorHandle, &pTemp);
    printf("\rInitial Temperature was %u C\n", pTemp);

    const esp_timer_create_args_t check_temp_args = {
        .callback = &check_temp, .name = "Check Temp"};

    esp_timer_handle_t check_temp_timer;
    ESP_ERROR_CHECK(
        esp_timer_create(&check_temp_args, &check_temp_timer));
    /* The timer has been created but is not running yet */

    /* Start timer */
    ESP_ERROR_CHECK(esp_timer_start_periodic(check_temp_timer, 1e6));
}