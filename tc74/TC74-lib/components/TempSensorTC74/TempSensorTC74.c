#include "include/TempSensorTC74.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include <esp_log.h>

#define RTR 0x00
#define RWCR 0x01
#define STANDBY 0x80
#define WAKEUP 0x00

esp_err_t tc74_init(i2c_master_bus_handle_t *pBusHandle,
                    i2c_master_dev_handle_t *pSensorHandle, uint8_t sensorAddr,
                    int sdaPin, int sclPin, uint32_t clkSpeedHz) {

  i2c_master_bus_config_t i2c_mst_config = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = I2C_NUM_0,
      .scl_io_num = sclPin,
      .sda_io_num = sdaPin,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };
  esp_err_t err = i2c_new_master_bus(&i2c_mst_config, pBusHandle);
  if (err != ESP_OK) {
    return err;
  }

  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = sensorAddr,
      .scl_speed_hz = clkSpeedHz,
  };

  return i2c_master_bus_add_device(*pBusHandle, &dev_cfg, pSensorHandle);
}

esp_err_t tc_74_free(i2c_master_bus_handle_t busHandle,
                     i2c_master_dev_handle_t sensorHandle) {

  esp_err_t del_dev = i2c_master_bus_rm_device(sensorHandle);
  if (del_dev != ESP_OK) {
    return del_dev;
  }

  return i2c_del_master_bus(busHandle);
}

esp_err_t tc74_standy(i2c_master_dev_handle_t sensorHandle) {
  uint8_t buffer[2] = {RWCR, STANDBY};
  return i2c_master_transmit(sensorHandle, buffer, sizeof(buffer), -1);
}

esp_err_t tc74_wakeup(i2c_master_dev_handle_t sensorHandle) {
  uint8_t buffer[2] = {RWCR, WAKEUP};
  return i2c_master_transmit(sensorHandle, buffer, sizeof(buffer), -1);
}

bool tc74_is_temperature_ready(i2c_master_dev_handle_t sensorHandle) {
  uint8_t txBuf[2] = {RWCR};
  uint8_t rxBuf[1] = {'2'};
  esp_err_t err = i2c_master_transmit_receive(
      sensorHandle, txBuf, sizeof(txBuf), rxBuf, sizeof(rxBuf), -1);
  if (err == ESP_OK) {
    return rxBuf[0] & 0x40;
  }
  return false;
}

esp_err_t tc74_wakeup_and_read_temp(i2c_master_dev_handle_t sensorHandle,
                                    uint8_t *pTemp) {
  uint8_t buffer[2] = {RWCR, WAKEUP};
  esp_err_t err =
      i2c_master_transmit(sensorHandle, buffer, sizeof(buffer), -1);
  if (err != ESP_OK) {
    return err;
  }

  uint8_t txBuf[1] = {RTR};
  return i2c_master_transmit_receive(sensorHandle, txBuf, sizeof(txBuf), pTemp,
                                     sizeof(*pTemp), -1);
}

esp_err_t tc74_read_temp_after_cfg(i2c_master_dev_handle_t sensorHandle,
                                   uint8_t *pTemp) {
  uint8_t txBuf[1] = {RTR};
  return i2c_master_transmit_receive(sensorHandle, txBuf, sizeof(txBuf), pTemp,
                                     sizeof(*pTemp), -1);
}

esp_err_t tc74_read_temp_after_temp(i2c_master_dev_handle_t sensorHandle,
                                    uint8_t *pTemp) {
  return i2c_master_receive(sensorHandle, pTemp, sizeof(*pTemp), -1);
}
