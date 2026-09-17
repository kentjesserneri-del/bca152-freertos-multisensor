# BCA152 FreeRTOS Multisensor

An ESP32 room monitoring system built on FreeRTOS. It reads temperature, humidity, and light level, shows the readings on a small OLED screen that the user can flip through with a rotary encoder, sounds a buzzer when the temperature leaves a safe range, and turns the display off automatically when the room has been empty for a while.

The point of this project wasn't just to make a working sensor box. It was to actually use FreeRTOS the way it's meant to be used — separate tasks doing separate jobs, queues carrying data between them, an event group for signaling state changes, and a mutex protecting the one shared resource that more than one task touches.

## What it does

- Reads temperature and humidity from a DHT22 and light level from an LDR, both on a 2-second cycle.
- Shows the readings on a 128x64 OLED, one value at a time, and lets you cycle between Temperature, Humidity, Light, and Motion with a rotary encoder.
- Sounds a buzzer and marks the display with a `*` if the temperature goes above 30°C or below 18°C.
- Uses a PIR motion sensor to detect whether the room is occupied. If nothing moves for 15 seconds, the OLED turns off. Sensing keeps running quietly in the background, and the display comes back the moment motion is detected again.

## Hardware

- ESP32 Dev Module
- DHT22 temperature and humidity sensor
- LDR (photoresistor) for light level
- SSD1306 128x64 OLED display (I2C)
- KY-040 rotary encoder
- Passive buzzer
- PIR motion sensor

## Circuit

![Wokwi circuit](docs/images/wokwi-circuit.png)

The DHT22 uses a single data pin on GPIO4. The LDR sits on GPIO36, one of the ESP32's ADC1 input-only pins — the correct choice for reading an analog sensor. The OLED talks over I2C on GPIO18 (SDA) and GPIO19 (SCL). The rotary encoder uses GPIO32 and GPIO33 for CLK and DT, kept separate from the I2C pins on purpose after running into a pin conflict earlier in development. The buzzer is driven with LEDC PWM on GPIO25 instead of a plain digital pin, since a passive buzzer needs an oscillating signal to make sound. The PIR sensor sits on GPIO26.

## Architecture

The system is built around five FreeRTOS tasks that never talk to each other directly. Everything passes through a queue or an event group instead.

![Architecture diagram](docs/architecture-diagram.svg)

**SensorTask** (priority 2) reads the DHT22 and LDR every 2 seconds and pushes the combined reading into two separate queues — one for the display, one for the alarm logic — since a FreeRTOS queue delivers each item to a single receiver.

**DisplayTask** (priority 1) owns the OLED. No other task writes to the screen directly. It pulls from the sensor queue, checks which display mode is currently selected, and checks the event group to know whether the system should be showing anything at all.

**InputTask** (priority 2) reads the rotary encoder through an interrupt handler and pushes the current display mode into a single-item queue using `xQueueOverwrite`, since only the latest selection matters.

**AlarmTask** (priority 3, the highest in the system) watches the temperature and drives the buzzer. It's given top priority because it does very little work per wake-up, so the cost of that priority is almost nothing — but a real temperature problem gets handled immediately instead of waiting behind a display refresh.

**MotionTask** (priority 2) polls the PIR every 500ms and decides whether the system should be considered ACTIVE or INACTIVE, setting and clearing bits in a shared event group that other tasks can read without needing direct access to MotionTask itself.

A mutex (`serialMutex`) protects the shared serial output, since DisplayTask, AlarmTask, and MotionTask can all print at different times, and printing is not naturally safe to do from multiple tasks at once.

## System state

![State machine diagram](docs/state-machine-diagram.svg)

The system has two states, ACTIVE and INACTIVE, tracked entirely by MotionTask. PIR motion keeps the system in ACTIVE and resets the idle timer. If 15 seconds pass with no motion, the system moves to INACTIVE and the OLED goes dark. As soon as the PIR fires again, it goes straight back to ACTIVE. Only PIR motion counts toward this — turning the encoder does not reset the timer, since the spec defines activity in terms of room occupancy, not user input.

## Repository layout

```
bca152-freertos-multisensor/
├── include/              header files
├── lib/dht22/            custom ESP-IDF DHT22 bit-bang driver
├── src/                  task and driver source files
│   ├── main.cpp
│   ├── display.cpp
│   ├── input.cpp
│   ├── alarm.cpp
│   ├── motion.cpp
│   ├── rtos_objects.cpp
│   ├── temperature_logic.cpp
│   ├── display_logic.cpp
│   └── motion_logic.cpp
├── test/                 native unit tests (Unity framework)
│   ├── test_temperature/
│   ├── test_display/
│   └── test_motion/
├── docs/
│   ├── functional-verification.md
│   ├── fault-experiments.md
│   └── images/
├── diagram.json          Wokwi circuit
├── wokwi.toml            Wokwi configuration
├── platformio.ini        PlatformIO configuration
└── README.md
```

## Screenshots

![OLED showing a temperature reading](docs/images/oled-temperature.png)

![OLED showing the Motion page](docs/images/oled-motion.png)

![Terminal output during a normal run](docs/images/terminal-normal.png)

![Terminal output showing an alarm state change](docs/images/terminal-alarm.png)

## Building and running

This project uses PlatformIO with the ESP-IDF framework.

Build for the ESP32:

```
pio run -e esp32dev
```

Run the native unit tests (no hardware or simulator needed):

```
pio test -e native
```

Run static analysis:

```
pio check -e esp32dev
```

To simulate the circuit, open the project in VS Code with the Wokwi extension installed, then start the simulator from `diagram.json`.

## Unit tests

Logic that doesn't touch hardware lives in separate `*_logic.cpp` files so it can be tested on the host machine. There are 18 tests total, covering:

- `evaluateTemperature()` — 7 tests, boundary behavior at 18°C and 30°C
- `nextDisplayMode()` / `previousDisplayMode()` — 6 tests, forward/reverse wraparound
- `evaluateSystemState()` — 5 tests, idle timeout and motion override

Run them with `pio test -e native`.

## Static analysis

`pio check -e esp32dev` uses cppcheck with `check_skip_packages = yes` so it only inspects the project's own source, not the ESP-IDF toolchain headers. The final run reports no defects.

## Functional verification

A full verification record — including observed behavior for 20 tests covering boot, sensors, display, encoder navigation, alarm behavior, motion state, and concurrency — is in [`docs/functional-verification.md`](docs/functional-verification.md).

## Fault experiments

Three deliberate faults were injected and observed — removing a blocking delay, over-prioritizing a task, and removing the serial mutex. Results and analysis are in [`docs/fault-experiments.md`](docs/fault-experiments.md).

## Known limitations

1. Wokwi prints a recurring `GPIO 18 is not usable, maybe conflict with others` warning from the I2C master driver. It showed up on every pin pair tested for the OLED (21/22, 15/16, 25/26, 18/19), and the OLED renders correctly in every case — so it looks like a Wokwi quirk under ESP-IDF v6.0.1, not a wiring or code issue.

2. The hand-written DHT22 bit-bang driver can occasionally produce a transient bad reading when the simulated sensor value is changed very quickly (for example, dragging a slider in Wokwi).

3. A passive buzzer needs an oscillating signal to produce sound. A plain `gpio_set_level` didn't animate in the simulator, so the buzzer is driven with LEDC PWM at 2 kHz — which is also the correct approach for real hardware.

4. Rotating the encoder does not reset the inactivity timeout. Only PIR motion counts as activity, which matches the spec.

5. If a DHT22 read fails, the code falls back to 0°C rather than skipping that reading, which will incorrectly trigger `LOW_TEMPERATURE`. A more careful implementation would skip the alarm check when the read is marked invalid.

6. There's up to ~700 ms of delay between a real motion event and the display reacting, since MotionTask polls the PIR every 500 ms and DisplayTask checks the event group every 200 ms. Not instant, but not noticeable in practice.

7. Wokwi reports 2 MB flash while the real ESP32 Dev Module has 4 MB. Purely a simulator default mismatch — no functional impact.

## Future improvements

- Skip the alarm pipeline on invalid DHT reads instead of substituting 0°C.
- Add a median-of-3 filter on the DHT22 driver to smooth out transient outliers.
- Treat encoder rotation as activity, so turning the knob wakes the display.
- Add persistence (NVS) so the last-selected display mode and thresholds survive a reboot.
- Add WiFi + MQTT for remote monitoring and cloud logging.

## References

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [Wokwi Simulator Docs](https://docs.wokwi.com/)
- [PlatformIO Docs](https://docs.platformio.org/)
- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [DHT22 Datasheet](https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf)

## Acknowledgments

Built for BCA152 Microcontrollers at Mindanao State University – Iligan Institute of Technology, College of Computer Studies, Department of Computer Applications.