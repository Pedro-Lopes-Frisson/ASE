#ifndef __SENSOR_BME280_H__INCLUDED__
#define __SENSOR_BME280_H__INCLUDED__


#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
esp_err_t bm280_init(i2c_master_bus_handle_t* pBusHandle,
					i2c_master_dev_handle_t* pSensorHandle,
					uint8_t sensorAddr, int sdaPin, int sclPin, uint32_t clkSpeedHz);

esp_err_t bm280_free(i2c_master_bus_handle_t busHandle,
					 i2c_master_dev_handle_t sensorHandle);

esp_err_t bm280_standy(i2c_master_dev_handle_t sensorHandle);

esp_err_t bm280_wakeup(i2c_master_dev_handle_t sensorHandle);

bool bm280_is_temperature_ready(i2c_master_dev_handle_t sensorHandle);

esp_err_t bm280_wakeup_and_read_temp(i2c_master_dev_handle_t sensorHandle, uint8_t* pTemp);

esp_err_t bm280_read_temp_after_cfg(i2c_master_dev_handle_t sensorHandle, uint8_t* pTemp);

esp_err_t bm280_read_temp_after_temp(i2c_master_dev_handle_t sensorHandle, uint8_t* pTemp);
#endif // __SENSOR_BME280_H__INCLUDED__
