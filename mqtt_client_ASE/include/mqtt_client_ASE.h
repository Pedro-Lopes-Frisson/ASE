#ifndef __MQTT_CLIENT_H__ASE__2024
#define __MQTT_CLIENT_H__ASE__2024

#include "mqtt_client.h"
#include "esp_event.h"

esp_mqtt_client_handle_t client ;

struct Event_Handler
{
    void (*data_callback)(esp_mqtt_event_handle_t)
};
void default_data_handler(esp_mqtt_event_handle_t event);
void log_error_if_nonzero(const char *message, int error_code);
/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data);

void mqtt_app_start(Event_Handler event_data);
#endif