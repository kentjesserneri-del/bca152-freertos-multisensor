# Fault Experiments

For these three I deliberately broke something that was already working correctly, watched what actually happened, and then put it back. The point was to actually see the failure modes instead of just describing what they would theoretically look like.

## Experiment 1: Removing the blocking delay

I commented out the line `vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(2000));` in SensorTask. So instead of sampling once every 2 seconds, it just looped as fast as the CPU would let it.

The effect was immediate and pretty obvious. DisplayTask's output went from one line every couple seconds to dozens of lines per second flooding the terminal. AlarmTask started flipping between LOW_TEMPERATURE and NORMAL rapidly too, since it was getting fed transient or noisy DHT readings as fast as SensorTask could read them. The OLED started visibly flickering because DisplayTask was redrawing constantly instead of on a steady cadence. The sim did not freeze completely, but it was clearly struggling. Everything felt sluggish and unresponsive.

This makes sense given the priorities. SensorTask sits at priority 2, DisplayTask at priority 1. On a single core, once a task stops yielding, it basically hogs the CPU until something of equal or higher priority needs to run. Since SensorTask never blocks anymore, DisplayTask only gets whatever tiny scraps of CPU time slip through. That is why its output got worse instead of better, even though it is technically running "more often" in wall clock terms. This is a classic case of CPU starvation.

I put the delay back afterward and rebuilt. Result was SUCCESS, no warnings, back to normal.

## Experiment 2: Giving a task an unnecessarily high priority

I bumped InputTask from priority 2 up to priority 5, which is higher than every other task in the system, including AlarmTask at priority 3.

Honestly, nothing much changed. I pushed the DHT22 temperature out of range several times (33.1°C, 1.2°C, 11.8°C) and AlarmTask still logged the state changes right away and the buzzer still kicked in immediately. The encoder was still responsive too. It was basically indistinguishable from the correctly prioritized version.

The reason is that InputTask spends almost all its time blocked on `xQueueReceive(encoderQueue, ...)`. It only wakes up when the encoder ISR actually fires an event. A task that is sitting blocked is not using any CPU, so it does not matter how high its priority is set. It cannot starve anything if it is not running. This was actually a useful thing to see, because it shows that priority misconfiguration is really only dangerous when the high priority task is doing continuous work and not yielding. An event driven task like this one stays safe even if its priority is set "wrong" on paper. I do not want to overstate this as if nothing bad can ever happen from bad priorities. It is specifically that InputTask's design, sitting blocked most of the time, is what kept it safe here, not that priority does not matter in general.

I set InputTask back to priority 2 and rebuilt. Result was SUCCESS.

## Experiment 3: Removing the mutex

I commented out every `xSemaphoreTake(serialMutex, ...)` and `xSemaphoreGive(serialMutex)` pair around the printf calls in DisplayTask, AlarmTask, and MotionTask. All three at once, since I needed at least two tasks writing around the same time to actually get interleaving.

I tried to force a collision on purpose. I rapidly pushed the DHT22 temperature back and forth across the alarm thresholds while also triggering the PIR, for about 20 seconds straight, trying to get all three tasks printing within the same few milliseconds of each other. Output stayed completely clean the whole time. Every line still had its correct tag, `[DisplayTask]`, `[AlarmTask]`, `[MotionTask]`. Nothing was garbled or mixed together.

My guess is that ESP-IDF's printf goes through the UART driver, and that driver holds its own internal lock while it is actually writing to the hardware. So even without my own serialMutex, writes were probably still getting serialized at a lower level in this default blocking UART setup. That does not mean the mutex I added is pointless. It documents the shared resource explicitly instead of relying on an implicit guarantee from a driver I do not control, and it would actually matter on hardware using non blocking UART or direct register writes, where that automatic serialization would not be there. So this is an honest "I could not get it to break in this environment" result, not "the mutex does not do anything."

I uncommented all three mutex pairs again and rebuilt. Result was SUCCESS, back to protected.

## Overall

Removing the blocking delay was the one that actually broke something visibly. Clear starvation, flooded terminal, flickering display. The priority experiment and the mutex experiment both came back clean, but for different and specific reasons. The warning about the unused lastWake variable was a key clue that the delay function was never actually running, since that function was the only thing that needed the variable. One because the task in question stays blocked most of the time, the other because the UART driver seems to already serialize writes underneath the application layer. Neither result means those mechanisms do not matter in general. All three were restored to their correct state and the final build came back SUCCESS.