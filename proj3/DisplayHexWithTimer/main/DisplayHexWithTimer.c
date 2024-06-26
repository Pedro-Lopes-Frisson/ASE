#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "include/pinout.h"
#include "portmacro.h"
#include "sdkconfig.h"
#include <stdint.h>
#include <unistd.h>
#define TAG "info"

static int secs = 0, counter = 0;
static void configure_MOSFETS(void) {
  gpio_reset_pin(MOSFET_1);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(MOSFET_1, GPIO_MODE_OUTPUT);
  gpio_reset_pin(MOSFET_2);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(MOSFET_2, GPIO_MODE_OUTPUT);
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

  gpio_reset_pin(LED_DP);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(LED_DP, GPIO_MODE_OUTPUT);
}

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
static void turn_off_leds() {
  gpio_set_level(LED_A, 0);
  gpio_set_level(LED_B, 0);
  gpio_set_level(LED_C, 0);
  gpio_set_level(LED_D, 0);
  gpio_set_level(LED_E, 0);
  gpio_set_level(LED_F, 0);
  gpio_set_level(LED_G, 0);
  gpio_set_level(LED_DP, 0);
}
static void turn_off_mosftet() {
  gpio_set_level(MOSFET_1, 0);
  gpio_set_level(MOSFET_2, 0);
}

void update_displays() {
  if (counter % 2 == 0) {
    gpio_set_level(MOSFET_1, 0);
    gpio_set_level(LED_A, table[secs % 16][0]);
    gpio_set_level(LED_B, table[secs % 16][1]);
    gpio_set_level(LED_C, table[secs % 16][2]);
    gpio_set_level(LED_D, table[secs % 16][3]);
    gpio_set_level(LED_E, table[secs % 16][4]);
    gpio_set_level(LED_F, table[secs % 16][5]);
    gpio_set_level(LED_G, table[secs % 16][6]);
    gpio_set_level(LED_DP, table[secs % 16][7]);
    gpio_set_level(MOSFET_2, 1);
  } else {
    gpio_set_level(MOSFET_2, 0);
    gpio_set_level(LED_A, table[(int)secs / 16][0]);
    gpio_set_level(LED_B, table[(int)secs / 16][1]);
    gpio_set_level(LED_C, table[(int)secs / 16][2]);
    gpio_set_level(LED_D, table[(int)secs / 16][3]);
    gpio_set_level(LED_E, table[(int)secs / 16][4]);
    gpio_set_level(LED_F, table[(int)secs / 16][5]);
    gpio_set_level(LED_G, table[(int)secs / 16][6]);
    gpio_set_level(LED_DP, table[(int)secs / 16][7]);
    gpio_set_level(MOSFET_1, 1);
  }
  counter++;
}

static void increment_seconds() { secs = (secs + 1) % (16 * 16); }

void app_main() {
  configure_MOSFETS();
  configure_LEDS();

  turn_off_mosftet();
  turn_off_leds();

  const esp_timer_create_args_t increment_seconds_args = {
      .callback = &increment_seconds, .name = "IncrementSeconds"};

  esp_timer_handle_t increment_seconds_timer;
  ESP_ERROR_CHECK(
      esp_timer_create(&increment_seconds_args, &increment_seconds_timer));
  /* The timer has been created but is not running yet */

  /* Start timer */
  ESP_ERROR_CHECK(esp_timer_start_periodic(increment_seconds_timer, 1e6));

  const esp_timer_create_args_t update_displays_args = {
      .callback = &update_displays, .name = "update_displays"};

  esp_timer_handle_t update_displays_timer;
  ESP_ERROR_CHECK(
      esp_timer_create(&update_displays_args, &update_displays_timer));
  /* The timer has been created but is not running yet */

  /* Start timer */
  ESP_ERROR_CHECK(esp_timer_start_periodic(update_displays_timer, 1e4));
}
