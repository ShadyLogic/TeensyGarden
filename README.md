# 🌿 Teensy Garden Irrigation Manager

This is a fully-featured, automated garden watering system built on the **Teensy 4.1** platform. It manages up to **four irrigation zones**, each with soil moisture sensors and solenoid valves, and supports multiple scheduling modes to ensure efficient watering.

## Features

- ✅ Supports up to 4 zones
- 🌱 Soil moisture monitoring
- 💧 Automated valve control
- ⏰ Flexible scheduling modes:
  - `NONE`: No automatic watering
  - `DOW`: Day-of-week scheduling
  - `INTERVAL`: Fixed interval watering
  - `SENSOR`: Moisture-based only
  - `INTERVAL_SENSOR`: Moisture + interval
- 💾 Persistent EEPROM storage for zone settings
- 📆 Real-time clock (RTC) support via Teensy 4.1
- 📄 Data logging to SD card
- 🖥️ Serial/BLE command menus for setup and monitoring

## Hardware Overview

| Component              | Description                                  |
|------------------------|----------------------------------------------|
| Microcontroller        | Teensy 4.1                                   |
| Valves (per zone)      | 2 GPIO pins per solenoid valve (4 zones max) |
| Soil Moisture Sensors  | Analog pins A6–A9 (Pins 20–23)               |
| RTC                    | Teensy built-in RTC                          |
| SD Card                | Uses `BUILTIN_SDCARD`                        |

**Pin Assignments:**

| Zone | Solenoid Pin 1 | Solenoid Pin 2 | Moisture Sensor |
|------|----------------|----------------|-----------------|
| 1    | D2             | D3             | A6 (Pin 20)     |
| 2    | D4             | D5             | A7 (Pin 21)     |
| 3    | D6             | D7             | A8 (Pin 22)     |
| 4    | D8             | D9             | A9 (Pin 23)     |

## Getting Started

### 1. Wiring

Connect your valves and sensors according to the pin assignments above. Insert a microSD card into the Teensy’s SD slot.

### 2. Upload Code

Use the Arduino IDE or PlatformIO to upload the project to your Teensy 4.1. Required libraries:

- Maltbie_Helper
- Maltbie_Menu
- TimeLib
- SD
- SPI

### 3. Interact via Serial

Open the Serial Monitor (baud rate 115200) to view the menu and interact:

- View status of all zones
- Set time
- Toggle or time-run valves
- Manage schedules

## EEPROM and Logging

- **Zone settings** are saved to EEPROM and automatically restored at boot.
- **Watering events** are timestamped and logged to `log.txt` on the SD card.
- At **12:00 PM daily**, all settings are auto-saved to EEPROM.

## File Structure

    .
    ├── GardenManager.h      # Zone and manager class definitions
    ├── GardenManager.cpp    # Implementation of watering logic
    ├── main.cpp             # Main setup loop and user interface

## Customization

You can easily extend or modify this project to:

- Add a web dashboard via ESP module or Ethernet
- Integrate weather forecasts
- Support additional sensors (e.g., rain or flow meters)

## Sample Output

    ***** Zone Info *****
    Tomatoes - Mode: INTERVAL/SENSOR
    Watering every 12 hrs
    Dry: 500 | Wet: 800
    Moisture Level: 480
    Valve is OPEN for 14m
    *********************

## Author

**Jacob Rogers**

If you found this project useful or would like to contribute improvements, feel free to fork or reach out!

## License

This project is open-source and free to use under the [MIT License](LICENSE).
