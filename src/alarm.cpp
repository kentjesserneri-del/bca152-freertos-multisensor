#include "alarm.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/task.h"
#include <cstdio>

#define BUZZER_PIN GPIO_NUM_25
#define BUZZER_FREQUENCY_HZ 2000

static constexpr ledc_timer_t buzzerTimer = LEDC_TIMER_0;
static constexpr ledc_channel_t buzzerChannel = LEDC_CHANNEL_0;

static QueueHandle_t alarmQueue;

AlarmState evaluateTemperature(float temperature) {
    if (temperature < LOW_TEMPERATURE_LIMIT)  return AlarmState::LOW_TEMPERATURE;
    if (temperature > HIGH_TEMPERATURE_LIMIT) return AlarmState::HIGH_TEMPERATURE;
    return AlarmState::NORMAL;
}

static void configure_buzzer_gpio(void) {
    ledc_timer_config_t timerConfig = {};
    timerConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    timerConfig.timer_num = buzzerTimer;
    timerConfig.duty_resolution = LEDC_TIMER_10_BIT;
    timerConfig.freq_hz = BUZZER_FREQUENCY_HZ;
    timerConfig.clk_cfg = LEDC_AUTO_CLK;
    ledc_timer_config(&timerConfig);

    ledc_channel_config_t channelConfig = {};
    channelConfig.gpio_num = BUZZER_PIN;
    channelConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    channelConfig.channel = buzzerChannel;
    channelConfig.intr_type = LEDC_INTR_DISABLE;
    channelConfig.timer_sel = buzzerTimer;
    channelConfig.duty = 0;
    channelConfig.hpoint = 0;
    ledc_channel_config(&channelConfig);
}

static void set_buzzer_enabled(bool enabled) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, buzzerChannel, enabled ? 512 : 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, buzzerChannel);
}

void alarm_init(QueueHandle_t alarmQueueHandle) {
    alarmQueue = alarmQueueHandle;
    configure_buzzer_gpio();
    set_buzzer_enabled(false);
}

void AlarmTask(void *pvParameters) {
    SensorData data;
    AlarmState lastState = AlarmState::NORMAL;

    while (true) {
        if (xQueueReceive(alarmQueue, &data, portMAX_DELAY) == pdTRUE) {
            AlarmState state = evaluateTemperature(data.temperature);

            if (state != lastState) {
                printf("[AlarmTask] Temp %.1f C -> state changed to %s\n",
                       data.temperature,
                       state == AlarmState::NORMAL ? "NORMAL" :
                       state == AlarmState::LOW_TEMPERATURE ? "LOW_TEMPERATURE" : "HIGH_TEMPERATURE");
                lastState = state;
            }

            set_buzzer_enabled(state != AlarmState::NORMAL);
        }
    }
}