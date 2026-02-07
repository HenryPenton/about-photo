#!/usr/bin/env python3
"""Simple HTTP server to receive sensor data from ESP32."""

from http.server import HTTPServer, BaseHTTPRequestHandler
from datetime import datetime
import json
import os

DATA_FILE = "sensor_data.json"


def load_data():
    if os.path.exists(DATA_FILE):
        with open(DATA_FILE, "r") as f:
            return json.load(f)
    return []


def save_data(data):
    with open(DATA_FILE, "w") as f:
        json.dump(data, f, indent=2)


class ESP32Handler(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(content_length).decode("utf-8")

        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        try:
            sensor_data = json.loads(body)
            sensor_data["timestamp"] = timestamp
        except json.JSONDecodeError:
            sensor_data = {"raw": body, "timestamp": timestamp}

        # Save to file
        all_data = load_data()
        all_data.append(sensor_data)
        save_data(all_data)

        print(f"[{timestamp}] Saved: {sensor_data}")

        self.send_response(200)
        self.send_header("Content-Type", "text/plain")
        self.end_headers()
        self.wfile.write(b"OK")

    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-Type", "text/plain")
        self.end_headers()
        self.wfile.write(b"ESP32 HTTP Server Running")


def run(host="0.0.0.0", port=8080):
    server = HTTPServer((host, port), ESP32Handler)
    print(f"Server running on http://{host}:{port}")
    print("Waiting for ESP32 data...")
    server.serve_forever()


if __name__ == "__main__":
    run()
