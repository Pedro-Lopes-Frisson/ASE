#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

#define I2CADDR 0x49
#define TESTE3  1
#undef TESTE1
#undef TESTE2

void app_main(void) {
  i2c_master_bus_config_t i2c_mst_config = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .i2c_port = I2C_NUM_0,
      .scl_io_num = 0,
      .sda_io_num = 1,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };

  i2c_master_bus_handle_t bus_handle;
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = I2CADDR,
      .scl_speed_hz = 100000,
  };

  i2c_master_dev_handle_t dev_handle;
  ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

#ifdef TESTE1

  while (1) {
    uint8_t buffer[2] = {0x01, 0x80};
    ESP_ERROR_CHECK(
        i2c_master_transmit(dev_handle, buffer, sizeof(buffer), -1));
    vTaskDelay(10);
  }
#endif

#ifdef TESTE2

  while (1) {
    uint8_t buffer[2] = {0x01, 0x00};
    ESP_ERROR_CHECK(
        i2c_master_transmit(dev_handle, buffer, sizeof(buffer), -1));
    vTaskDelay(10);
  }

#endif
#ifdef TESTE3

  uint8_t buffer[2] = {0x01, 0x00};
  ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, buffer, sizeof(buffer), -1));

  while (1) {
    uint8_t txBuf[1] = {0x00};
    uint8_t rxBuf[1] = {'2'};

    ESP_ERROR_CHECK(i2c_master_transmit_receive(
        dev_handle, txBuf, sizeof(txBuf), rxBuf, sizeof(rxBuf), -1));
    vTaskDelay(10);
    printf("\r%u C", rxBuf[0]);
  }

#endif
}
