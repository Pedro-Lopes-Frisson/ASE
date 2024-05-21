idf_component_register(SRCS "wifiAppv2.c"
                    INCLUDE_DIRS "." REQUIRES esp_wifi lwip nvs_flash mqtt_client_ase TempSensorTC74 wifiv2 esp_timer)
