# Hardware

## ESP32 Sensor System

### Components

#### ESP32 Development Board
Microcontroller with WiFi capability for sensor data collection and transmission.

#### GPS Module
**Current:** NEO-6M or similar GPS module on UART2 (pins 16/17)
- Provides latitude, longitude, satellite count, and UTC time
- Parses NMEA sentences ($GPGGA, $GPRMC)
- Requires clear view of sky for satellite fix

#### Planned Sensors

| Measurement | Status     | Planned Hardware  |
| ----------- | ---------- | ----------------- |
| GPS         | ✅ Implemented | NEO-6M GPS module |
| Temperature | Simulated  | DHT22 / BME280    |
| Humidity    | Simulated  | DHT22 / BME280    |
| Pressure    | Simulated  | BME280 / BMP280   |
| Light Level | Simulated  | BH1750 / LDR      |

#### Voltage Regulator
TBD - for battery operation

#### Optocoupler
TBD - for camera shutter trigger integration

#### On/Off Switch
TBD

#### Battery
TBD - for portable operation

#### Status LED
**GPIO 2** - Built-in LED
- Blinking: Connecting to WiFi
- Solid: Connected to WiFi
- Brief flash: Data sent to server

### Wiring

#### Current Setup
- **GPIO 16** → GPS module TX (UART2 RX)
- **GPIO 17** → GPS module RX (UART2 TX)
- **GPIO 4** → Trigger button to GND (uses internal pull-up)
- **GPIO 2** → Built-in LED (status indicator)
- **GPS VCC** → 3.3V or 5V (check your module's requirements)
- **GPS GND** → GND

### Firmware Setup

1. **Install Arduino IDE** with ESP32 board support
   
2. **Configure secrets**
   - Copy `secrets.h.example` to `secrets.h`
   - Update WiFi credentials:
     ```cpp
     const char* ssid = "YOUR_WIFI_SSID";
     const char* password = "YOUR_WIFI_PASSWORD";
     ```
   - Update server URL with your computer's local IP:
     ```cpp
     const char* serverUrl = "http://192.168.1.XXX:8080";
     ```

3. **Upload firmware**
   - Open `sketch_feb5a.ino` in Arduino IDE
   - Select your ESP32 board
   - Upload

4. **Monitor output**
   - Open Serial Monitor
   - Set baud rate to **115200**
   - You should see WiFi connection status and GPS NMEA sentences

### Operation

1. **Power on** - ESP32 connects to WiFi (LED blinks then goes solid)
2. **GPS acquisition** - Module searches for satellites (can take 30+ seconds outdoors)
3. **Trigger** - Short GPIO 4 to GND to send current sensor data to server
4. **Data transmission** - LED flashes briefly, data is POSTed to server

### Casing

TBD - 3D printed enclosure design

## Additional Sensors

### CO2 Sensor
Planned - for measuring atmospheric CO2 concentration

### Rain Sensor
Planned - for detecting precipitation

### Barometer
Planned - BME280 or BMP280 for atmospheric pressure
