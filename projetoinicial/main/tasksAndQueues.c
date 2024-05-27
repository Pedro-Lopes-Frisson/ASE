#include "TempSensorTC74.h"
#include "apps/esp_sntp.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/projdefs.h"
#include "include/pinout.h"
#include "portmacro.h"
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include "include/mqtt_client_ase.h"
#include "include/wifiv2.h"
#include "esp_sntp.h"
#include "esp_netif_sntp.h"
#include "nvs_flash.h"
// #include "cJSON.h"

#define TC74ADDR 0x49
#define TAG "ERROR"
#define MAX_TEMPS 10
#define MODE 1
#define MAX_T 30
#define MIN_T 20

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY \
  (4000) // Frequency in Hertz. Set frequency at 4 kHz
         //

struct Temp
{
  uint8_t temp;
  int64_t timestamp;
} Temp;

struct Save_temps_args
{
  QueueHandle_t handler;
} Save_temps_args;

static int w_ptr = 0;
static uint8_t temps[MAX_TEMPS];
static uint8_t latest_temp;
static i2c_master_bus_handle_t pBusHandle;
static i2c_master_dev_handle_t pSensorHandle;
void check_temp(void);
void save_temps(void *pvParameters);
void write_temp_to_console(void);
void update_display(void);
static QueueHandle_t save_temps_queue;
static int counter = 0;
void subscriber_handler(esp_mqtt_event_handle_t event)
{
  // cJSON *json = cJSON_ParseWithLength(event->data, event->data_len);
  // const cJSON *command = NULL;
  // command = cJSON_GetObjectItemCaseSensitive(monitor_json, "command");
  // if (cJSON_IsString(name) && (name->valuestring != NULL))
  // {
  //   printf("Checking monitor \"%s\"\n", name->valuestring);
  // }

  // if (strncmp(name, "set", 3) == 0)
  // {
  //   command = cJSON_GetObjectItemCaseSensitive(monitor_json, "command");
  // }

  printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
  printf("DATA=%.*s\r\n", event->data_len, event->data);
  // if (json == NULL)
  // {
  //   const char *error_ptr = cJSON_GetErrorPtr();
  //   if (error_ptr != NULL)
  //   {
  //     fprintf(stderr, "Error before: %s\n", error_ptr);
  //   }
  //   status = 0;
  // }
  // name = cJSON_GetObjectItemCaseSensitive(monitor_json, "name");
  // if (cJSON_IsString(name) && (name->valuestring != NULL))
  // {
  //   printf("Checking monitor \"%s\"\n", name->valuestring);
  // }
  // cJSON_Delete(monitor_json);
  // return status;
}

void check_temp(void)
{
  struct Temp pTemp;
  tc74_read_temp_after_temp(pSensorHandle, &(pTemp.temp));
  struct timeval tv_now;
  gettimeofday(&tv_now, NULL);
  pTemp.timestamp = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
  if (save_temps_queue != 0)
  {
    printf("New temp at timestamp %lld value detected was %u\n\r",
           (long long int)pTemp.timestamp, (uint8_t)pTemp.temp);
    // Send the structure to the queue
    if (xQueueSend(save_temps_queue, &pTemp, 0) != pdPASS)
    {
      ESP_LOGE(TAG, "Failed to send temp to queue");
    }
  }
  else
  {
    ESP_LOGE(TAG, "No Queue");
  }
}

void save_temps(void *pvParameters)
{
  struct Temp receivedTemp;

  while (1)
  {
    // Wait indefinitely until data is received
    if (xQueueReceive(save_temps_queue, &receivedTemp, portMAX_DELAY) == pdPASS)
    {
      char msg[80];
      snprintf(&msg, 80, "{\"Timestamp\": %lld ,\"Temperature\":%u}", (long long int)receivedTemp.timestamp, receivedTemp.temp);
      mqtt_publish("sensor/tc74", msg);

      if (receivedTemp.temp != latest_temp)
      {
        latest_temp = receivedTemp.temp;
        printf("New temp at timestamp %lld value detected was %u\r",
               (long long int)receivedTemp.timestamp, (uint8_t)receivedTemp.temp);
      }

      temps[w_ptr] = receivedTemp.temp;
      w_ptr = (w_ptr + 1) % MAX_TEMPS; // Ensure proper circular buffer increment
    }
  }
}
#define STACK_SIZE 4096 * 2
static void setup_sntp(void)
{
  esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
  esp_netif_sntp_init(&config);

  time_t now;
  char strftime_buf[64];
  struct tm timeinfo;

  time(&now);
  setenv("TZ", "WET0WEST,M3.5.0/1,M10.5.0", 1);
  tzset();

  esp_netif_sntp_start();
}

void app_main(void)
{
  uint8_t pTemp;
  struct Save_temps_args pvParametersSaveTemps;
  tc74_init(&pBusHandle, &pSensorHandle, TC74ADDR, 1, 0, 100000);

  ESP_ERROR_CHECK(nvs_flash_init());
  initialise_wifi();

  struct Event_Handler e =
      {
          .data_callback = subscriber_handler,
      };

  while (isWifiConnected() != 1)
  {
    vTaskDelay(1000);
  }
  setup_sntp();
  printf("Connected\n stating mqtt\n");
  while (mqtt_app_start(&e) != ESP_OK)
  {
    vTaskDelay(1000);
  }

  printf("MQTT HAS STARTED\n");
  mqtt_subscribe("esp32c3/ase");

  // test read_temp_after_temp
  tc74_wakeup_and_read_temp(pSensorHandle, &pTemp);
  printf("\rInitial Temperature was %u C\n", pTemp);
  latest_temp = pTemp;
  pvParametersSaveTemps.handler = save_temps_queue = xQueueCreate(10, sizeof(struct Temp));

  const esp_timer_create_args_t check_temp_args = {.callback = &check_temp,
                                                   .name = "Check Temp"};

  esp_timer_handle_t check_temp_timer;
  ESP_ERROR_CHECK(esp_timer_create(&check_temp_args, &check_temp_timer));
  /* The timer has been created but is not running yet */

  /* Start timer */
  ESP_ERROR_CHECK(esp_timer_start_periodic(check_temp_timer, 5e6 * 60));

  /* Tarefa */
  /* Start timer */
  TaskHandle_t xHandle;
  xTaskCreate(save_temps, "Save updated", STACK_SIZE,
              (void *)&pvParametersSaveTemps, 0, &xHandle);

  while (true)
  {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
