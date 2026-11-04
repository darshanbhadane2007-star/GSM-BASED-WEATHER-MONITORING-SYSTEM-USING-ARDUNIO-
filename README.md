# GSM-Based Weather Monitoring System

![Platform](https://img.shields.io/badge/platform-Arduino%20Uno-00979D?logo=arduino&logoColor=white)
![Language](https://img.shields.io/badge/language-C%2B%2B%20(Arduino)-blue)
![Connectivity](https://img.shields.io/badge/connectivity-GSM%20SMS-green)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

A low-cost, internet-free weather station built on Arduino Uno. It measures temperature, humidity, pressure, rain and light, displays the data on a scrolling 16x2 LCD, raises a buzzer alert on rain, and replies with a full weather report by SMS when requested.

## Table of Contents
- [Features](#features)
- [System Architecture](#system-architecture)
- [Hardware Requirements](#hardware-requirements)
- [Circuit Connections](#circuit-connections)
- [Software Requirements](#software-requirements)
- [Installation and Setup](#installation-and-setup)
- [Configuration](#configuration)
- [Usage](#usage)
- [Weather Logic](#weather-logic)
- [Code Overview](#code-overview)
- [Troubleshooting](#troubleshooting)
- [Known Limitations](#known-limitations)
- [Future Improvements](#future-improvements)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)
- [Author](#author)

## Features
- Temperature and humidity monitoring (DHT11)
- Barometric pressure and altitude sensing (BMP180)
- Rain detection with analog and digital outputs
- Ambient light detection (LDR)
- Scrolling 16x2 I2C LCD display
- Buzzer alert when rain is detected
- On-demand SMS weather report (text `HELLO` to the SIM number)
- Rule-based condition and short-term rain prediction
- Works anywhere with GSM coverage, no Wi-Fi or internet required

## System Architecture
```
 +----------+   +----------+   +-----------+   +-------+
 |  DHT11   |   |  BMP180  |   | Rain / LDR|   |Buzzer |
 | Temp/Hum |   | Pressure |   |  Sensors  |   | Alert |
 +----+-----+   +----+-----+   +-----+-----+   +---^---+
      |              | I2C           |               |
      +--------------+---------------+---------------+
                         |
                  +------v-------+        +------------------+
                  | Arduino Uno  |<------>|  GSM Module      |
                  | (processing, |  UART  | (SIM800L/SIM900A)|
                  |  prediction) |        +--------+---------+
                  +------+-------+                 |
                         | I2C                     v
                  +------v-------+          SMS to/from phone
                  | 16x2 I2C LCD |
                  +--------------+
```

## Hardware Requirements
| Component | Qty | Notes |
|---|---|---|
| Arduino Uno | 1 | ATmega328P, 5 V logic |
| GSM module (SIM800L / SIM900A) | 1 | With active 2G-enabled SIM |
| DHT11 sensor | 1 | Temperature and humidity |
| BMP180 (BMP085-compatible) | 1 | I2C, address `0x77` |
| Rain sensor module | 1 | Provides AO and DO |
| LDR + 10 kΩ resistor | 1 | Voltage divider |
| 16x2 LCD with I2C backpack | 1 | Address `0x27` or `0x3F` |
| Active buzzer | 1 | 5 V |
| External power supply | 1 | 2 A minimum for the GSM module |
| Breadboard, jumper wires | - | |

> Check that your network still supports 2G. Some regions have shut it down, in which case the module will not register.

## Circuit Connections
| Device | Pin | Arduino |
|---|---|---|
| DHT11 | DATA | D8 |
| Rain sensor | AO | A0 |
| Rain sensor | DO | D7 |
| LDR divider | Output | A1 |
| Buzzer | + | D13 |
| GSM module | TX | D11 (Arduino RX) |
| GSM module | RX | D12 (Arduino TX) |
| LCD (I2C) | SDA / SCL | A4 / A5 |
| BMP180 (I2C) | SDA / SCL | A4 / A5 |

**LDR divider:** 5 V → LDR → A1 → 10 kΩ → GND.

**Wiring notes**
- Power the GSM module from its own supply and connect its GND to Arduino GND. Do not power it from the Arduino 5 V pin. It draws current bursts of up to 2 A.
- SIM800L runs at 3.4 to 4.4 V (use about 4 V). SIM900A runs at 5 V.
- SIM800L RX is not 5 V tolerant. Put a voltage divider (for example 1 kΩ / 2 kΩ) between Arduino D12 and the module RX.
- LCD and BMP180 share the I2C bus without conflict (`0x27` vs `0x77`). Check your BMP180 breakout's supply rating (3.3 V or 5 V).
- Add a 10 kΩ pull-up on the DHT11 data line if you use the bare 3-pin sensor.

## Software Requirements
- [Arduino IDE](https://www.arduino.cc/en/software) 1.8.x or 2.x
- Libraries (Library Manager):

| Library | Author |
|---|---|
| DHT sensor library | Adafruit |
| Adafruit Unified Sensor | Adafruit |
| Adafruit BMP085 Library | Adafruit |
| LiquidCrystal I2C | Frank de Brabander |
| Wire, SoftwareSerial | Built in |

## Installation and Setup
1. Clone the repository:
   ```bash
   git clone https://github.com/darshanbhadane2007-star/gsm-weather-monitoring-system.git
   ```
2. Wire the circuit as per [Circuit Connections](#circuit-connections).
3. Insert an active SIM (PIN lock disabled) into the GSM module.
4. Open `gsm_weather_monitor/gsm_weather_monitor.ino` in Arduino IDE.
5. In `sendSMS()`, set the recipient number in international format:
   ```cpp
   gsm.println("AT+CMGS=\"+91XXXXXXXXXX\"");
   ```
   Retype this line manually. Copy-pasted numbers can carry hidden Unicode characters that break the `AT+CMGS` command.
6. Select **Tools → Board → Arduino Uno** and the correct port.
7. Upload the sketch. The LCD shows **"Weather Monitor / System Ready"** after startup.
8. Wait for the GSM module to register on the network (status LED blinks about every 3 s), then send the test SMS.

## Configuration
| Setting | Location | Default |
|---|---|---|
| Recipient phone number | `sendSMS()` | `+91XXXXXXXXXX` |
| DHT sensor type / pin | `DHTTYPE`, `DHTPIN` | `DHT11`, `8` |
| LCD I2C address | `LiquidCrystal_I2C lcd(...)` | `0x27` (try `0x3F`) |
| Light threshold (Dark/Bright) | `loop()` | `500` |
| Rain-soon humidity / rain value | `predictWeather()` | `> 70 %`, `< 500` |
| SMS trigger keyword | `loop()` | `HELLO` (case-sensitive) |
| Scroll speed | `scrollMessage()` | `350 ms` |
| Buzzer pattern | `buzzerAlert()` | 5 × (4 s on, 2 s off) |
| GSM baud rate | `gsm.begin()` | `9600` |

## Usage
**Local monitoring:** Live readings and the forecast scroll across the two LCD rows continuously.

**Remote report:** Send an SMS with the text `HELLO` to the SIM number in the module. The system replies with the current report.

**Rain alert:** When the rain sensor digital output goes LOW, the LCD shows `Rain` and the buzzer sounds.

### Sample SMS reply
```
Weather Report:
[ Weather Update ] Current: Temp:31.00C  Hum:45.00%  Pressure:1024.00hPa  Wind:2km/h  Cond:Sunny
[ Forecast ] No rain expected soon Pred.Temp:33.00C
```

## Weather Logic
| Condition | Rule |
|---|---|
| Rain Soon | humidity > 70 % **and** rain analog value < 500 |
| Sunny | temperature > 30 °C **and** humidity < 40 % |
| Cloudy | all other cases |

| Forecast text | Trigger |
|---|---|
| Rain detected | Rain sensor DO is LOW |
| Rain in ~2 hrs | Prediction is "Rain Soon" |
| No rain expected soon | Otherwise |

Predicted temperature is current temperature + 2 °C (simple assumption).

## Code Overview
| Function | Purpose |
|---|---|
| `setup()` | Initializes serial, sensors, LCD, and the GSM module (`AT`, `AT+CMGF=1`, `AT+CNMI=1,2,0,0,0`) |
| `loop()` | Reads sensors, builds messages, updates LCD, triggers buzzer, checks incoming SMS |
| `predictWeather()` | Classifies conditions from temperature, humidity and rain value |
| `buzzerAlert()` | Sounds the buzzer pattern on rain |
| `sendSMS()` | Sends a text-mode SMS (`AT+CMGS`, terminated with Ctrl+Z) |
| `scrollMessage()` | Scrolls a long string across one LCD row |

**GSM commands used**

| Command | Function |
|---|---|
| `AT` | Module check |
| `AT+CMGF=1` | SMS text mode |
| `AT+CNMI=1,2,0,0,0` | Push incoming SMS directly to serial |
| `AT+CMGS="<number>"` | Send SMS |

## Troubleshooting
| Problem | Likely cause / fix |
|---|---|
| LCD blank | Change address to `0x3F`, adjust contrast pot |
| `BMP180 not detected!` in Serial Monitor | Check SDA/SCL wiring and module supply voltage |
| `nan` temperature/humidity | Check DHT wiring, pull-up and `DHTTYPE` |
| GSM module resets or no response | Insufficient power; use a 2 A supply and common GND |
| No SMS received | Wrong number format, no balance/coverage, or hidden characters in the number line |
| SMS not sent or truncated | Message is about 160+ characters; shorten the text (module text mode limit) |
| No reply to `HELLO` | Keyword must be uppercase; see the SMS buffer limitation below |
| Rain always detected | Adjust the sensor potentiometer; clean or dry the sensor board |

## Known Limitations
- **Wind speed and pressure are placeholders.** Wind is fixed at `2km/h` and pressure at `1024 hPa` (no anemometer, `bmp.readPressure()` not used).
- **Slow loop cycle.** Blocking LCD scrolling takes about 45 s per cycle (plus 30 s of buzzer when raining), so sensors are sampled roughly once a minute.
- **SMS requests can be delayed or missed.** Incoming SMS is checked once per cycle and the `SoftwareSerial` buffer holds only 64 bytes, so a request arriving mid-cycle may be lost.
- LDR light level is read but not shown on the LCD or sent by SMS.
- Prediction uses simple thresholds, not a real forecasting model. Treat it as indicative only.
- Reports go to a fixed recipient number.

## Future Improvements
- Use `bmp.readPressure() / 100.0` for real pressure and add an anemometer for wind speed
- Non-blocking code (`millis()`) so LCD, buzzer and GSM polling run together
- Automatic SMS alerts on threshold breach and periodic reports
- Reply to the sender's number instead of a fixed number
- Pressure-trend-based forecasting
- Data logging to SD card or cloud (ESP32/GPRS)
- Solar power with battery backup
- Enclosure and weatherproofing for outdoor deployment

## Project Structure
```
gsm-weather-monitoring-system/
├── gsm_weather_monitor/
│   └── gsm_weather_monitor.ino
├── docs/
│   ├── circuit-diagram.png
│   └── demo.jpg
├── LICENSE
└── README.md
```

## Contributing
1. Fork the repository
2. Create a branch: `git checkout -b feature/your-feature`
3. Commit your changes: `git commit -m "Add your feature"`
4. Push and open a Pull Request

## License
Released under the [MIT License](LICENSE).

## Author
**Darshan Bhadane**
Computer Science and AI, VIT Pune
[GitHub](https://github.com/darshanbhadane2007-star) · [LinkedIn](https://www.linkedin.com/in/darshan-bhadane-bb2444398/)
