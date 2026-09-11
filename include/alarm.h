#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "sensor_data.h"

#define LOW_TEMPERATURE_LIMIT  18.0f
#define HIGH_TEMPERATURE_LIMIT 30.0f

enum class AlarmState {
    NORMAL,
    LOW_TEMPERATURE,
    HIGH_TEMPERATURE
};

AlarmState evaluateTemperature(float temperature);

void alarm_init(QueueHandle_t alarmQueueHandle);
void AlarmTask(void *pvParameters);