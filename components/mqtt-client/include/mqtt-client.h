#ifndef ESP_MQTT_CLIES_ASE_2024___
#define ESP_MQTT_CLIES_ASE_2024___
#include "esp_event.h"

#include "mqtt_client.h"

/* MQTT client */
static esp_mqtt_client_handle_t client;
// flag indicating if the client is or isn't connected to the broker
static int isConnected = 0;

struct Event_Handlers{
    void (*data_handler(esp_mqtt_event_handle_t));
} Event_Handlers;

/**
 * @brief mqtt client initialization
 * This function connects and starts the mqtt client and enables the publishing of messages
 */
void mqtt_app_start(void);

/**
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
/**
 * @brief mqtt message publish wrapper
 *
 * @param mqttTopic topic to publish the message to
 * @param mqttMessage Message to be published
 */
void mqtt_publish(const char *mqttTopic, const char *mqttMessage);
#endif