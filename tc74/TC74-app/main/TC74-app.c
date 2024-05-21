#include "TempSensorTC74.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>
#include "esp_timer.h"
#define TC74ADDR 0x49


static uint8_t pTemp;
static i2c_master_bus_handle_t pBusHandle;
static i2c_master_dev_handle_t pSensorHandle;

void check_temp(void){

  tc74_read_temp_after_temp(pSensorHandle, &pTemp);
  printf("\rInitial Temperature was %u C\n", pTemp);
}


void app_main(void)
{

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
