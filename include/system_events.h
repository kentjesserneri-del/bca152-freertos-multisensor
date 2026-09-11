#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define EVENT_ACTIVE (1 << 0)  // set = system ACTIVE, cleared = INACTIVE. Producer: MotionTask. Consumer: DisplayTask.
#define EVENT_MOTION (1 << 1)  // himuon while PIR currently read ug  HIGH. Producer: MotionTask. Consumer: SensorTask.
#define EVENT_ALARM  (1 << 2)  // reserve ni for phase 10

extern EventGroupHandle_t systemEvents;