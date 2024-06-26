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
#define LEDC_FREQUENCY                                                         \
  (4000) // Frequency in Hertz. Set frequency at 4 kHz
         //

float map_value_to_percentage(float value) {
  float t = ((value - MIN_T) / (MAX_T - MIN_T));
  if (t < 0)
    return 0;
  if (t > 1)
    return 1;
  return t;
}

struct Temp {
  uint8_t temp;
  int timestamp;
} Temp;

struct Save_temps_args {
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
QueueHandle_t save_temps_queue;
static int counter = 0;

// display - digit - Led states
static int table[16][8] = {
    // a b  c  d  e  f  g  dp
    {1, 1, 1, 1, 1, 1, 0, 1}, //  0
    {0, 1, 1, 0, 0, 0, 0, 1}, //  1
    {1, 1, 0, 1, 1, 0, 1, 1}, //  2
    {1, 1, 1, 1, 0, 0, 1, 1}, //  3
    {0, 1, 1, 0, 0, 1, 1, 1}, //  4
    {1, 0, 1, 1, 0, 1, 1, 1}, //  5
    {1, 0, 1, 1, 1, 1, 1, 1}, //  6
    {1, 1, 1, 0, 0, 0, 0, 1}, //  7
    {1, 1, 1, 1, 1, 1, 1, 1}, //  8
    {1, 1, 1, 1, 0, 1, 1, 1}, //  9
    {1, 1, 1, 0, 1, 1, 1, 1}, //  a
    {0, 0, 1, 1, 1, 1, 1, 1}, //  b
    {1, 0, 0, 1, 1, 1, 0, 1}, //  c
    {0, 1, 1, 1, 1, 0, 1, 1}, //  d
    {1, 0, 0, 1, 1, 1, 1, 1}, //  e
    {1, 0, 0, 0, 1, 1, 1, 1}, //  f
};

static void configure_MOSFETS(void) {
  gpio_reset_pin(MOSFET_2);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(MOSFET_2, GPIO_MODE_OUTPUT);
  gpio_set_level(MOSFET_2, 0);
}
static void configure_LEDS(void) {
  gpio_reset_pin(LED_A);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(LED_A, GPIO_MODE_OUTPUT);

  gpio_reset_pin(LED_B);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(LED_B, GPIO_MODE_OUTPUT);

  gpio_reset_pin(LED_C);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(LED_C, GPIO_MODE_OUTPUT);

  gpio_reset_pin(LED_D);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(LED_D, GPIO_MODE_OUTPUT);

  gpio_reset_pin(LED_E);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(LED_E, GPIO_MODE_OUTPUT);

  gpio_reset_pin(LED_F);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(LED_F, GPIO_MODE_OUTPUT);

  gpio_reset_pin(LED_G);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(LED_G, GPIO_MODE_OUTPUT);

  gpio_set_level(LED_A, 0);
  gpio_set_level(LED_B, 0);
  gpio_set_level(LED_C, 0);
  gpio_set_level(LED_D, 0);
  gpio_set_level(LED_E, 0);
  gpio_set_level(LED_F, 0);
  gpio_set_level(LED_G, 0);
}

void check_temp(void) {
  struct Temp pTemp;
  BaseType_t b;
  tc74_read_temp_after_temp(pSensorHandle, &(pTemp.temp));
  pTemp.timestamp = (int)time(NULL);
  if (save_temps_queue != 0) {

    printf("Sending temp at timestamp %d value detected was %u\n",
           (int)pTemp.timestamp, (uint8_t)pTemp.temp);
    b = xQueueGenericSend(save_temps_queue, (void *)&pTemp, (TickType_t)0,
                          queueSEND_TO_BACK);
    if (b != pdPASS) {
      ESP_LOGE(TAG, "Failed to send message to Queue");
    }
  } else {
    ESP_LOGE(TAG, "No Queue");
  }
}

void save_temps(void *pvParameters) {
  struct Temp temp;
  QueueHandle_t queue =
      (QueueHandle_t)((struct Save_temps_args *)pvParameters)->handler;
  while (true) {
    printf("ola\n");
    if (queue != 0) {
      while (uxQueueMessagesWaiting(queue) != 0) {
        if (xQueueReceive(queue, (void *)&(temp), (TickType_t)10) == pdPASS) {
          if (temp.temp != latest_temp) {
            latest_temp = temp.temp;
            printf("New temp at timestamp %d value detected was %u\n",
                   (int)temp.timestamp, (uint8_t)temp.temp);
          }
          temps[w_ptr] = temp.temp;
          w_ptr = w_ptr + 1 % MAX_TEMPS;
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1000 * 2));
  }
}

static int m_state = 1;
void toggle_mosfet() {
  gpio_set_level(MOSFET_2, m_state);
  m_state = (m_state + 1) % 2;
}

void update_display() {
  if (counter % 2 == 0) {
    toggle_mosfet();
    gpio_set_level(LED_A, table[latest_temp % 10][0]);
    gpio_set_level(LED_B, table[latest_temp % 10][1]);
    gpio_set_level(LED_C, table[latest_temp % 10][2]);
    gpio_set_level(LED_D, table[latest_temp % 10][3]);
    gpio_set_level(LED_E, table[latest_temp % 10][4]);
    gpio_set_level(LED_F, table[latest_temp % 10][5]);
    gpio_set_level(LED_G, table[latest_temp % 10][6]);
  } else {
    toggle_mosfet();
    gpio_set_level(LED_A, table[(int)latest_temp / 10][0]);
    gpio_set_level(LED_B, table[(int)latest_temp / 10][1]);
    gpio_set_level(LED_C, table[(int)latest_temp / 10][2]);
    gpio_set_level(LED_D, table[(int)latest_temp / 10][3]);
    gpio_set_level(LED_E, table[(int)latest_temp / 10][4]);
    gpio_set_level(LED_F, table[(int)latest_temp / 10][5]);
    gpio_set_level(LED_G, table[(int)latest_temp / 10][6]);
  }
  counter++;
}

static void example_ledc_init(void) {
  // Prepare and then apply the LEDC PWM timer configuration
  ledc_timer_config_t ledc_timer = {
      .speed_mode = LEDC_MODE,
      .timer_num = LEDC_TIMER,
      .duty_resolution = LEDC_DUTY_RES,
      .freq_hz = LEDC_FREQUENCY, // Set output frequency at 4 kHz
      .clk_cfg = LEDC_AUTO_CLK};
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  // Prepare and then apply the LEDC PWM channel configuration
  ledc_channel_config_t ledc_channel = {.speed_mode = LEDC_MODE,
                                        .channel = LEDC_CHANNEL,
                                        .timer_sel = LEDC_TIMER,
                                        .intr_type = LEDC_INTR_DISABLE,
                                        .gpio_num = LED_PWM,
                                        .duty = 0, // Set duty to 0%
                                        .hpoint = 0};
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void update_pwm() {
  // Set duty to 50%
  float t = map_value_to_percentage((float)latest_temp);
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, pow(2, 13) * t));
  // Update duty to apply the new value
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

#define STACK_SIZE 4224

static void setup_wifi(void) {

}
static void setup_sntp(void) {
  esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
  esp_netif_sntp_init(&config);
}

void app_main(void) {
  uint8_t pTemp;
  struct Save_temps_args pvParametersSaveTemps;
  tc74_init(&pBusHandle, &pSensorHandle, TC74ADDR, 1, 0, 100000);

  configure_LEDS();
  configure_MOSFETS();

  // test read_temp_after_temp
  tc74_wakeup_and_read_temp(pSensorHandle, &pTemp);
  printf("\rInitial Temperature was %u C\n", pTemp);

  pvParametersSaveTemps.handler = xQueueCreate(10, sizeof(struct Temp *));
  save_temps_queue = pvParametersSaveTemps.handler;

  const esp_timer_create_args_t check_temp_args = {.callback = &check_temp,
                                                   .name = "Check Temp"};

  esp_timer_handle_t check_temp_timer;
  ESP_ERROR_CHECK(esp_timer_create(&check_temp_args, &check_temp_timer));
  /* The timer has been created but is not running yet */

  /* Start timer */
  ESP_ERROR_CHECK(esp_timer_start_periodic(check_temp_timer, 5e6));

  /* Tarefa */
  /* Start timer */
  TaskHandle_t xHandle;
  xTaskCreate(save_temps, "Save updated", STACK_SIZE,
              (void *)&pvParametersSaveTemps, 0, &xHandle);
#if MODE == 1
  const esp_timer_create_args_t update_display_arg = {
      .callback = &update_display, .name = "Update Display"};

  esp_timer_handle_t update_display_timer;
  ESP_ERROR_CHECK(esp_timer_create(&update_display_arg, &update_display_timer));
  /* The timer has been created but is not running yet */

  /* Start timer */
  ESP_ERROR_CHECK(esp_timer_start_periodic(update_display_timer, 1e3));

#endif

#if MODE == 2
  const esp_timer_create_args_t update_pwm_arg = {.callback = &update_pwm,
                                                  .name = "Update LEDC"};
  esp_timer_handle_t update_pwm_timer;
  ESP_ERROR_CHECK(esp_timer_create(&update_pwm_arg, &update_pwm_timer));
  /* The timer has been created but is not running yet */
  /* Start timer */
  ESP_ERROR_CHECK(esp_timer_start_periodic(update_pwm_timer, 1e3));
  // Set the LEDC peripheral configuration
  example_ledc_init();
  // Set duty to 50%
  ESP_ERROR_CHECK(
      ledc_set_duty(LEDC_MODE, LEDC_CHANNEL,
                    pow(2, 13) * map_value_to_percentage((float)pTemp)));

  // Update duty to apply the new value
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
#endif

  while (true) {
    vTaskDelay(100000000 / portTICK_PERIOD_MS);
  }
}
