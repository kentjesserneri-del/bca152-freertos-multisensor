#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Task 1: Runs every 1 second
void TaskOne(void *pvParameters)
{
    while (true) {
        printf("[TaskOne] Hello from Task One\n");
        vTaskDelay(pdMS_TO_TICKS(1000)); // Block for 1 second
    }
}

// Task 2: Runs every 2 seconds
void TaskTwo(void *pvParameters)
{
    while (true) {
        printf("[TaskTwo] Hello from Task Two\n");
        vTaskDelay(pdMS_TO_TICKS(2000)); // Block for 2 seconds
    }
}

// Main entry point
extern "C" void app_main(void)
{
    printf("BCA152 FreeRTOS Multisensor\n");
    printf("System starting...\n");

    // Create TaskOne with priority 1
    xTaskCreate(
        TaskOne,        // Task function
        "TaskOne",      // Task name
        2048,           // Stack size (words)
        NULL,           // Parameters
        1,              // Priority
        NULL            // Task handle (not needed)
    );

    // Create TaskTwo with priority 1
    xTaskCreate(
        TaskTwo,        // Task function
        "TaskTwo",      // Task name
        2048,           // Stack size (words)
        NULL,           // Parameters
        1,              // Priority
        NULL            // Task handle (not needed)
    );

    // app_main can exit - tasks will continue running
}