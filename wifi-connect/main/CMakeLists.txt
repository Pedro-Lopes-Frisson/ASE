idf_component_register(SRCS "wifi-connect.c"
    INCLUDE_DIRS "." REQUIRES lwip esp_wifi nvs_flash TempSensorTC74 mqtt)
