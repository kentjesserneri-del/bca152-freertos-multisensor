#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

enum class DisplayMode {
    TEMPERATURE,
    HUMIDITY,
    LIGHT,
    MOTION
};

DisplayMode nextDisplayMode(DisplayMode current);
DisplayMode previousDisplayMode(DisplayMode current);

void input_init(QueueHandle_t modeQueueHandle);
void InputTask(void *pvParameters);