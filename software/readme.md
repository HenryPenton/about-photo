# Software

## Components

### 1. HTTP Server (`server.py`)

Listens for sensor data from the ESP32 and saves it to JSON.

**Usage:**
```bash
cd software
python3 server.py
```

The server:
- Listens on port 8080
- Accepts POST requests with JSON sensor data
- Appends data to `sensor_data.json` with timestamps
- Logs each received reading

**Example log output:**
```
[2026-02-05 14:30:00] Saved: {'temperature': 25.3, 'humidity': 62.5, ...}
```

### 2. Metadata Embedder (`embed_metadata.py`)

Writes sensor data into DNG photo files as EXIF/XMP metadata.

**Requirements:**
```bash
brew install exiftool  # macOS
# or
apt install libimage-exiftool-perl  # Linux
```

**Usage:**

Embed the latest sensor reading:
```bash
python3 embed_metadata.py photo.dng
```

Embed a specific reading (by index):
```bash
python3 embed_metadata.py photo.dng 0
```

Process all DNGs in a folder:
```bash
python3 embed_metadata.py ./photos/
```

## Data Format

### Sensor Data JSON

```json
{
  "temperature": 25.3,
  "humidity": 62.5,
  "pressure": 1015.2,
  "light": 512,
  "latitude": 41.502351,
  "longitude": -0.157758,
  "gps_fix": true,
  "satellites": 8,
  "timestamp": "2026-02-05 14:30:00"
}
```

### Embedded Metadata

The following metadata is written to DNG files:

| Field       | EXIF/XMP Tag        |
| ----------- | ------------------- |
| Temperature | XMP:Temperature     |
| Humidity    | XMP:Humidity        |
| Pressure    | XMP:Pressure        |
| Light Level | XMP:LightLevel      |
| Timestamp   | XMP:SensorTimestamp |
| Latitude    | GPSLatitude         |
| Longitude   | GPSLongitude        |
| GPS Fix     | XMP:GPSFix          |
| Satellites  | XMP:Satellites      |
| Summary     | XMP:Description     |

## Photo and Metadata Pairing

There are many ways to link extra metadata to the correct photo. Camera clocks can be inaccurate, so pairing should consider gaps between photos as well as timestamps.

**Current implementation:**
- `embed_metadata.py` pairs the most recent sensor reading by default
- Can specify a particular reading by index
- Can batch process all DNGs in order with all sensor readings

**Future improvements:**
- Timestamp-based matching with tolerance
- Gap detection between photo sequences
- Manual review interface for ambiguous matches