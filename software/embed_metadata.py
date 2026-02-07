#!/usr/bin/env python3
"""Embed sensor data from sensor_data.json into DNG files as XMP metadata."""

import json
import subprocess
import sys
import os
from pathlib import Path

DATA_FILE = "sensor_data.json"


def load_sensor_data():
    """Load sensor data from JSON file."""
    if not os.path.exists(DATA_FILE):
        print(f"Error: {DATA_FILE} not found")
        sys.exit(1)

    with open(DATA_FILE, "r") as f:
        return json.load(f)


def embed_metadata(dng_path, sensor_entry):
    """Embed sensor data into a DNG file using exiftool."""

    # Get GPS coordinates
    lat = sensor_entry.get("latitude", 0)
    lon = sensor_entry.get("longitude", 0)
    lat_ref = "N" if lat >= 0 else "S"
    lon_ref = "E" if lon >= 0 else "W"

    # Build exiftool command with XMP metadata and GPS
    cmd = [
        "exiftool",
        "-overwrite_original",
        f"-XMP:Temperature={sensor_entry.get('temperature', '')}",
        f"-XMP:Humidity={sensor_entry.get('humidity', '')}",
        f"-XMP:Pressure={sensor_entry.get('pressure', '')}",
        f"-XMP:LightLevel={sensor_entry.get('light', '')}",
        f"-XMP:SensorTimestamp={sensor_entry.get('timestamp', '')}",
        # GPS coordinates (standard EXIF GPS tags)
        f"-GPSLatitude={abs(lat)}",
        f"-GPSLatitudeRef={lat_ref}",
        f"-GPSLongitude={abs(lon)}",
        f"-GPSLongitudeRef={lon_ref}",
        # Also store as description for easy viewing
        f"-XMP:Description=Temp:{sensor_entry.get('temperature')}C Humidity:{sensor_entry.get('humidity')}% Pressure:{sensor_entry.get('pressure')}hPa Light:{sensor_entry.get('light')} GPS:{lat},{lon}",
        str(dng_path),
    ]

    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode == 0:
            print(f"✓ Embedded metadata into {dng_path.name}")
            return True
        else:
            print(f"✗ Error with {dng_path.name}: {result.stderr}")
            return False
    except FileNotFoundError:
        print("Error: exiftool not installed. Install with: brew install exiftool")
        sys.exit(1)


def main():
    if len(sys.argv) < 2:
        print("Usage: python embed_metadata.py <dng_file_or_folder> [sensor_index]")
        print("")
        print("Examples:")
        print(
            "  python embed_metadata.py photo.dng           # Use latest sensor reading"
        )
        print(
            "  python embed_metadata.py photo.dng 0         # Use first sensor reading"
        )
        print(
            "  python embed_metadata.py ./photos/           # Process all DNGs in folder"
        )
        sys.exit(1)

    target = Path(sys.argv[1])
    sensor_data = load_sensor_data()

    if not sensor_data:
        print("Error: No sensor data available")
        sys.exit(1)

    # Get sensor entry (default to latest)
    sensor_index = int(sys.argv[2]) if len(sys.argv) > 2 else -1
    try:
        sensor_entry = sensor_data[sensor_index]
    except IndexError:
        print(
            f"Error: Sensor index {sensor_index} out of range (0-{len(sensor_data) - 1})"
        )
        sys.exit(1)

    print(f"Using sensor data: {sensor_entry}")
    print("")

    # Find DNG files
    if target.is_file():
        dng_files = [target]
    elif target.is_dir():
        dng_files = list(target.glob("*.dng")) + list(target.glob("*.DNG"))
    else:
        print(f"Error: {target} not found")
        sys.exit(1)

    if not dng_files:
        print(f"No DNG files found in {target}")
        sys.exit(1)

    # Process each file
    success = 0
    for dng in dng_files:
        if embed_metadata(dng, sensor_entry):
            success += 1

    print(f"\nProcessed {success}/{len(dng_files)} files")


if __name__ == "__main__":
    main()
