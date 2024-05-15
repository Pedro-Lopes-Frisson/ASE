#include "TempSensorTC74.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>
#define TC74ADDR 0x49
void app_main(void) {
  i2c_master_bus_handle_t pBusHandle;
  i2c_master_dev_handle_t pSensorHandle;
  tc74_init(&pBusHandle, &pSensorHandle, TC74ADDR, 1, 0, 100000);

  // test standby
  while (0) {
    tc74_standy(pSensorHandle);
    vTaskDelay(10);
  }

  // test wakeup
  while (0) {
    tc74_wakeup(pSensorHandle);
    vTaskDelay(10);
  }

  // test is ready
  tc74_wakeup(pSensorHandle);
  while (0) {
    printf("\rData is Ready ? %d", tc74_is_temperature_ready(pSensorHandle));
    vTaskDelay(10);
  }

  // test wake_up_and_read_temp
  uint8_t pTemp;
  while (0) {
    tc74_wakeup_and_read_temp(pSensorHandle, &pTemp);
    printf("\r%u C", pTemp);
    vTaskDelay(10);
  }

  // test read_temp_after_cfg
  tc74_wakeup(pSensorHandle);
  while (0) {
    tc74_read_temp_after_cfg(pSensorHandle, &pTemp);
    printf("\r%u C", pTemp);
    vTaskDelay(10);
  }

  // test read_temp_after_temp
  tc74_wakeup_and_read_temp(pSensorHandle, &pTemp);
  while (1) {
    tc74_read_temp_after_temp(pSensorHandle, &pTemp);
    printf("\r%u C", pTemp);
    vTaskDelay(10);
  }
  tc_74_free(pBusHandle, pSensorHandle);
}
