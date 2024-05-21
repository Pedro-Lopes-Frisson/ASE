idf_component_register(SRCS "tasksAndQueues.c"
    INCLUDE_DIRS "." REQUIRES TempSensorTC74 esp_timer driver lwip nvs_flash  wifiv2 mqtt_client_ase esp_wifi )
          