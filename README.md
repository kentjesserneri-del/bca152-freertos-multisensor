# BCA152 FreeRTOS Multisensor

ESP32 multisensor system built with FreeRTOS (via ESP-IDF) and simulated in Wokwi. This is my lab project for BCA152 — the goal is a multitasking embedded system that reads temperature/humidity, light, and motion, displays readings on an OLED, and raises alarms when temperature goes out of range.

## Status

Work in progress. Currently done:

- [x] PlatformIO + ESP-IDF project set up and building
- [x] Wokwi simulation configured, ESP32 booting with serial output
- [ ] Sensor tasks (DHT22, LDR)
- [ ] Queue-based communication between tasks
- [ ] OLED display + rotary encoder navigation
- [ ] Alarm logic (buzzer, temperature thresholds)
- [ ] Motion detection + active/inactive state machine
- [ ] Event group / mutex synchronization
- [ ] Unit tests
- [ ] Full documentation + report

## Hardware (simulated)

- ESP32 Dev Module
- DHT22 (temperature/humidity)
- LDR (light level)
- PIR motion sensor
- SSD1306 OLED display
- Rotary encoder
- Buzzer

All of the above are simulated in Wokwi — no physical hardware is used for this lab.

## Tech stack

- **Framework:** ESP-IDF (not Arduino), so all tasks use native FreeRTOS APIs
- **Build system:** PlatformIO
- **Simulation:** Wokwi (VS Code extension)

## Project structure

```
include/      header files
lib/          project-specific libraries
src/          source files (main.c and task modules)
test/         unit tests
docs/         lab report and diagrams
diagram.json  Wokwi circuit layout
wokwi.toml    Wokwi simulator config
```

## Building and running

1. Clone the repo and open it in VS Code with the PlatformIO extension installed.
2. Build:
   ```
   pio run
   ```
3. Open `diagram.json` and start the Wokwi simulation (Wokwi for VS Code extension), or run `pio run -t upload` if using real hardware.
4. Watch the serial monitor at 115200 baud for output.

## Why FreeRTOS

The whole point of this lab is learning to split a system into independent, concurrently running tasks instead of one big loop — sensor reading, display updates, input handling, and alarm logic all run as separate tasks, synchronized with queues, a mutex, and an event group where needed. Details on the task design and priority choices will go in `docs/laboratory-report.pdf` once that section is finished.

## Author

Kent Jesserneri — BCA152