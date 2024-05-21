#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
#include <esp_log.h>
#include <sys/time.h>

#define TAG "QueueExample"
#define QUEUE_SIZE 10

// Define a structure to hold temperature data and a timestamp
struct Temp {
    uint8_t temp;
    int64_t timestamp;
};

// Queue handle
static QueueHandle_t tempQueue;

// Task to receive data from the queue
void tempReceiverTask(void *pvParameters) {
    struct Temp receivedTemp;

    while (1) {
        // Wait indefinitely until data is received
        if (xQueueReceive(tempQueue, &receivedTemp, portMAX_DELAY) == pdPASS) {
            ESP_LOGI(TAG, "Received temp: %u, timestamp: %lld", receivedTemp.temp, (long long int)receivedTemp.timestamp);
        }
    }
}

// Timer callback to produce data and send it to the queue
void tempTimerCallback(TimerHandle_t xTimer) {
    struct Temp newTemp;
    struct timeval tv_now;
    
    // Simulate reading a temperature value
    
    // Get current timestamp
    gettimeofday(&tv_now, NULL);
    newTemp.timestamp = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
    newTemp.temp = (uint8_t)(newTemp.timestamp % 100);

    // Send the structure to the queue
    if (xQueueSend(tempQueue, &newTemp, 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to send temp to queue");
    }
}

void app_main(void) {
    // Create the queue
    tempQueue = xQueueCreate(QUEUE_SIZE, sizeof(struct Temp));
    if (tempQueue == NULL) {
        ESP_LOGE(TAG, "Failed to create queue");
        return;
    }

    // Create the temperature receiver task
    xTaskCreate(tempReceiverTask, "TempReceiverTask", 2048, NULL, 5, NULL);

    // Create the timer to send data every second
    TimerHandle_t tempTimer = xTimerCreate("TempTimer", pdMS_TO_TICKS(1000), pdTRUE, NULL, tempTimerCallback);
    if (tempTimer == NULL) {
        ESP_LOGE(TAG, "Failed to create timer");
        return;
    }

    // Start the timer
    if (xTimerStart(tempTimer, 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to start timer");
        return;
    }
}
