Functional Verification

Below is everything I tested in Wokwi to make sure the system actually works the way it's supposed to, not just that it compiles. I went through each feature and wrote down what I actually saw on screen or in the terminal.

About the I2C warnings: every single run prints repeating lines like W (8533) i2c.common: GPIO 18 is not usable, maybe conflict with others. I tried four different pin pairs for the OLED (21/22, 15/16, 25/26, and 18/19) and got the exact same warning every time regardless of pin. Since the OLED renders correctly no matter what, and the warning doesn't change based on which real pins are used, this looks like a quirk in how Wokwi's ESP-IDF v6.0.1 simulation logs I2C driver init, not an actual wiring or code problem.

Boot and sensor basics

Started the sim and the serial monitor printed "BCA152 FreeRTOS Multisensor" then "System starting..." right away, before anything sensor-related showed up. Good.

With the sim just running normally, DisplayTask kept logging lines like [DisplayTask] Temp: 24.0 C, Humidity: 40.0 %, Light: 24% about every 2 seconds, which is the sampling rate I set. That checks out.

For the LDR, I dragged the illumination slider down from 25119 lux and watched the light percentage change in real time — went Light: 80% → 26% → 2% in a couple seconds. So the ADC read is working and it's actually reflecting the slider movement, not just stuck on a default value.

On the queue side, I just watched whether the numbers SensorTask reads (temp, humidity, light) all end up together on the same DisplayTask line, and they did every time. That means SensorData is making it through sensorQueue in one piece rather than getting split up or corrupted somewhere.

OLED + encoder

Right when the sim starts, before I touch anything, the OLED sits on ROOM MONITOR / TEMPERATURE / 24.0C. That's the expected default state.

Turned the knob one click clockwise and it moved to ROOM MONITOR / HUMIDITY / 40.0% — so TEMPERATURE to HUMIDITY works like it should.

Kept turning it clockwise, four clicks total, and it cycled all the way through TEMPERATURE → HUMIDITY → LIGHT → MOTION and landed back on TEMPERATURE. Watched the OLED update at every single step to make sure nothing skipped or got stuck.

Then went the other way — turned counterclockwise starting from TEMPERATURE — and it jumped straight to MOTION / NO, which is the wraparound-backward behavior it's supposed to have instead of just stopping.

Alarm

Pushed the DHT22 temperature slider above 30°C and got [AlarmTask] Temp 31.0 C -> state changed to HIGH_TEMPERATURE in the log. OLED header switched to ROOM MONITOR * and the buzzer showed as active in the sim.

Dropped the temperature way down to 5.5°C, below the 18°C low limit, and the log showed Temp 5.5 C -> state changed to LOW_TEMPERATURE. OLED read ROOM MONITOR * / TEMPERATURE / 5.5C, buzzer active again.

Brought it back up into range at 22.5°C and the log flipped to Temp 22.5 C -> state changed to NORMAL. The * disappeared off the header and the buzzer stopped.

One thing I wanted to specifically check was whether the alarm would spam the log every 2-second sensor cycle while sitting in an alarm state, so I held the temperature at -12.6°C for well past 10 seconds. Only got the one log line for the transition — nothing repeated after that, which means the state != lastState check in AlarmTask is actually doing what it's supposed to.

Motion / system state

Left the PIR alone (no motion) for 15+ seconds and MotionTask logged State changed to INACTIVE at right around the 16-second mark. Screenshot confirms the OLED went completely black at that point.

Triggered the PIR again after that and got State changed to ACTIVE back, with the display coming back on and showing sensor data again like normal.

While the screen was blank, I specifically watched the serial output to make sure the rest of the system hadn't frozen too — and DisplayTask kept printing Temp: 18.2 C, Humidity: 40.0 %, Light: 24% lines the whole time the OLED was dark. So it's just the display that goes quiet, not the whole system.

For the MOTION page specifically: cycled the encoder over to it, and with no PIR activity it showed MOTION / NO. Triggered the PIR and within about 200ms it flipped to MOTION / YES, matching up with the State changed to ACTIVE line in the log at basically the same moment.

Serial output / mutex

To try to trip up the serial mutex, I triggered alarm changes and motion changes close together on purpose, hoping to catch two tasks writing to the terminal at nearly the same time. Everything came out clean — [DisplayTask], [AlarmTask], and [MotionTask] lines all printed with intact tags and no mixed-up characters anywhere. I'll be honest that this is more of a "didn't see it break" result than hard proof something worked, since there's no screenshot that really proves a race condition didn't happen — but across everything I ran, nothing ever looked garbled.

Build / tests / static analysis

pio run -e esp32dev — SUCCESS, using 4.2% of RAM and 17.5% of flash.

pio test -e native — all 18 tests passed (7 for temperature logic, 6 for display logic, 5 for motion logic).

pio check -e esp32dev — came back clean, "No defects found." The very first time I ran this it actually crashed partway through, because cppcheck was trying to parse a GCC toolchain header and hit a syntax error that had nothing to do with anything I wrote. Fixed it by adding check_skip_packages = yes to platformio.ini so cppcheck only looks at my own source instead of every ESP-IDF header it can find. After that it ran clean.