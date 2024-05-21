idf_component_register(SRCS "wifi_mqtt_app.c"
                    INCLUDE_DIRS "." REQUIRES esp_wifi lwip WiFi nvs_flash TempSensorTC74 mqttclientase)
