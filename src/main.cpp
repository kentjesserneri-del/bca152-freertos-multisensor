#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_adc/adc_oneshot.h"
#include "dht22.h"
#include "sensor_data.h"
#include "display.h"

#define DHT_PIN GPIO_NUM_4

static QueueHandle_t sensorQueue;
static adc_oneshot_unit_handle_t adc1_handle;

void SensorTask(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        SensorData data = {};

        dht22_reading_t dht = dht22_read(DHT_PIN);
        data.temperature = dht.valid ? dht.temperature : 0;
        data.humidity    = dht.valid ? dht.humidity    : 0;

        int raw = 0;
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw);
        data.lightLevel = (raw * 100) / 4095;

        data.motionDetected = false;

        xQueueSend(sensorQueue, &data, portMAX_DELAY);
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(2000));
    }
}

void DisplayTask(void *pvParameters)
{
    SensorData received;

    while (true) {
        if (xQueueReceive(sensorQueue, &received, portMAX_DELAY) == pdTRUE) {

            printf("[DisplayTask] Temp: %.1f C, Humidity: %.1f %%, Light: %d%%\n",
                   received.temperature, received.humidity, received.lightLevel);

            char line2[17];
            snprintf(line2, sizeof(line2), "%d.%dC L:%d%%",
                     (int)received.temperature,
                     (int)(received.temperature * 10) % 10,
                     received.lightLevel);

            display_draw_string(0, 0, "ROOM MONITOR");
            display_draw_string(2, 0, "TEMPERATURE");
            display_draw_string(4, 0, line2);
        }
    }
}

extern "C" void app_main(void)
{
    printf("BCA152 FreeRTOS Multisensor\n");
    printf("System starting...\n");

    // ADC init (LDR)
    adc_oneshot_unit_init_cfg_t init_config = {};
    init_config.unit_id = ADC_UNIT_1;
    init_config.clk_src = ADC_RTC_CLK_SRC_DEFAULT;
    init_config.ulp_mode = ADC_ULP_MODE_DISABLE;
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    adc_oneshot_chan_cfg_t chan_config = {};
    chan_config.atten = ADC_ATTEN_DB_12;
    chan_config.bitwidth = ADC_BITWIDTH_DEFAULT;
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &chan_config);

    // OLED init (with power-up delay)
    vTaskDelay(pdMS_TO_TICKS(150));
    display_init();
    display_clear();

    // Queue + tasks
    sensorQueue = xQueueCreate(5, sizeof(SensorData));

    xTaskCreate(SensorTask,  "SensorTask",  4096, NULL, 2, NULL);
    xTaskCreate(DisplayTask, "DisplayTask", 4096, NULL, 1, NULL);
}