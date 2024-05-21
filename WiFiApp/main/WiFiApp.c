#include "WiFi.h"
#include "include/WiFi.h"
#include "include/mqttclientase.h"
#include <stdint.h>
#include <stdio.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static void default_handler1(esp_mqtt_event_handle_t event)
{
    printf("1TOPI11111111C=%.*s\r\n", event->topic_len, event->topic);
    printf("1DATA1=%.*s\r\n", event->data_len, event->data);
}
void app_main(void)
{

  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // wifi_init_sta((uint8_t *)"MEO-23AD5F", (uint8_t *)"C77265F64A");
  wifi_init_sta((uint8_t *)"A326E28", (uint8_t *)"password123!");
  
    struct Event_Handler e =
        {
            .data_callback = default_handler1,
        };

    printf("Connected\n stating mqtt\n");
    while (mqtt_app_start(&e) != ESP_OK)
    {
        vTaskDelay(1000);
    }
    printf("MQTT HAS STARTED\n");
    mqtt_subscribe("ola");

    while (1)
    {
        vTaskDelay(1000);
        mqtt_publish("ola", "ola esp");
    }
}
