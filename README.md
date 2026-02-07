# About Photo

Capture environmental sensor data with an ESP32 and embed it into DNG photo metadata.

## Overview

This project links environmental data to photographs by:

1. **ESP32 hardware** - Captures real-time sensor data (GPS, temperature, humidity, pressure, light) and transmits over WiFi
2. **HTTP server** - Receives and stores sensor data as JSON
3. **Metadata embedder** - Writes sensor data into DNG photo files as EXIF/XMP metadata

The goal is to preserve the environmental context of each photograph for archival, scientific, or artistic purposes.

## Quick Start

1. **Hardware**: See [hardware/readme.md](hardware/readme.md) for ESP32 setup, wiring, and sensor configuration
2. **Software**: See [software/readme.md](software/readme.md) for server and metadata embedding instructions

## Project Structure

```
├── hardware/
│   └── esp-32/
│       └── sketch_feb5a/
│           ├── sketch_feb5a.ino    # ESP32 firmware with GPS integration
│           └── secrets.h.example   # WiFi/server config template
└── software/
    ├── server.py                   # HTTP server for receiving sensor data
    ├── embed_metadata.py           # DNG metadata embedder
    └── sensor_data.json            # Captured sensor data (generated at runtime)
```

## Current Status

✅ **Implemented:**
- GPS module integration (UART on pins 16/17)
- WiFi data transmission to server
- HTTP server with JSON storage
- DNG metadata embedding with exiftool

🚧 **In Progress:**
- Temperature/humidity sensor (DHT22 or BME280)
- Barometric pressure sensor (BME280/BMP280)
- Light level sensor (BH1750 or LDR)
- Timestamp-based photo matching

## License

See [LICENSE](LICENSE) for details.
