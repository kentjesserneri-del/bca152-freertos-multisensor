#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dht22.h"

#define DHT_PIN GPIO_NUM_4

void SensorTask(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        dht22_reading_t reading = dht22_read(DHT_PIN);
        if (reading.valid) {
            printf("[SensorTask] Temp: %.1f C, Humidity: %.1f %%\n",
                   reading.temperature, reading.humidity);
        } else {
            printf("[SensorTask] Failed to read DHT22\n");
        }
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(2000));
    }
}

extern "C" void app_main(void)
{
    printf("BCA152 FreeRTOS Multisensor\n");
    printf("System starting...\n");
    xTaskCreate(SensorTask, "SensorTask", 4096, NULL, 2, NULL);
}