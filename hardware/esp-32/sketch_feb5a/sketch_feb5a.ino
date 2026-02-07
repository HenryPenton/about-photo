#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

// Trigger pin - short this pin to GND to send
const int triggerPin = 4;
bool lastState = HIGH;

// GPS module on UART2: RX=16, TX=17
#define GPS_RX 16
#define GPS_TX 17
#define GPS_BAUD 9600

HardwareSerial gpsSerial(2);

// Parsed GPS data
double gpsLat = 0.0;
double gpsLon = 0.0;
bool gpsFix = false;
String gpsTime = "";
int gpsSats = 0;

// Buffer for incoming NMEA sentences
String nmeaBuffer = "";

// Convert NMEA coordinate (ddmm.mmmm) to decimal degrees
double nmeaToDecimal(String raw, String dir) {
  if (raw.length() == 0) return 0.0;

  int dotPos = raw.indexOf('.');
  int degLen = dotPos - 2;
  double degrees = raw.substring(0, degLen).toDouble();
  double minutes = raw.substring(degLen).toDouble();
  double decimal = degrees + (minutes / 60.0);

  if (dir == "S" || dir == "W") decimal = -decimal;
  return decimal;
}

// Parse a $GPGGA sentence for lat, lon, fix, satellites, time
void parseGGA(String sentence) {
  int fieldIndex = 0;
  int start = 0;
  String fields[15];

  for (int i = 0; i <= (int)sentence.length(); i++) {
    if (i == (int)sentence.length() || sentence[i] == ',') {
      if (fieldIndex < 15) {
        fields[fieldIndex] = sentence.substring(start, i);
      }
      fieldIndex++;
      start = i + 1;
    }
  }

  int fix = fields[6].toInt();
  if (fix > 0) {
    gpsFix = true;
    gpsLat = nmeaToDecimal(fields[2], fields[3]);
    gpsLon = nmeaToDecimal(fields[4], fields[5]);
    gpsSats = fields[7].toInt();
    gpsTime = fields[1];
  } else {
    gpsFix = false;
  }
}

// Parse a $GPRMC sentence as a fallback
void parseRMC(String sentence) {
  int fieldIndex = 0;
  int start = 0;
  String fields[13];

  for (int i = 0; i <= (int)sentence.length(); i++) {
    if (i == (int)sentence.length() || sentence[i] == ',') {
      if (fieldIndex < 13) {
        fields[fieldIndex] = sentence.substring(start, i);
      }
      fieldIndex++;
      start = i + 1;
    }
  }

  if (fields[2] == "A") {
    gpsFix = true;
    gpsLat = nmeaToDecimal(fields[3], fields[4]);
    gpsLon = nmeaToDecimal(fields[5], fields[6]);
    gpsTime = fields[1];
  } else {
    gpsFix = false;
  }
}

void processNMEA(String sentence) {
  int starPos = sentence.indexOf('*');
  if (starPos > 0) sentence = sentence.substring(0, starPos);

  if (sentence.startsWith("$GPGGA") || sentence.startsWith("$GNGGA")) {
    parseGGA(sentence);
  } else if (sentence.startsWith("$GPRMC") || sentence.startsWith("$GNRMC")) {
    parseRMC(sentence);
  }
}

// Read and process any available GPS data
void updateGPS() {
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    if (c == '\n') {
      nmeaBuffer.trim();
      if (nmeaBuffer.startsWith("$")) {
        Serial.println(nmeaBuffer);  // Print raw NMEA sentence
        processNMEA(nmeaBuffer);
      }
      nmeaBuffer = "";
    } else {
      nmeaBuffer += c;
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  pinMode(triggerPin, INPUT_PULLUP);

  // Start GPS serial
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("GPS module on pins 16 (RX) / 17 (TX)");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(2, !digitalRead(2));
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(2, HIGH);
}

String generateSensorData() {
  // Generate fake sensor readings (real sensors TBD)
  float temperature = 20.0 + random(0, 150) / 10.0;
  float humidity = 40.0 + random(0, 400) / 10.0;
  float pressure = 1000.0 + random(0, 300) / 10.0;
  int lightLevel = random(0, 1024);

  // Format as JSON — uses real GPS if we have a fix, zeros otherwise
  String json = "{";
  json += "\"temperature\":" + String(temperature, 1) + ",";
  json += "\"humidity\":" + String(humidity, 1) + ",";
  json += "\"pressure\":" + String(pressure, 1) + ",";
  json += "\"light\":" + String(lightLevel) + ",";
  json += "\"latitude\":" + String(gpsLat, 6) + ",";
  json += "\"longitude\":" + String(gpsLon, 6) + ",";
  json += "\"gps_fix\":" + String(gpsFix ? "true" : "false") + ",";
  json += "\"satellites\":" + String(gpsSats);
  json += "}";

  return json;
}

void sendToServer(String data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    int responseCode = http.POST(data);

    if (responseCode > 0) {
      Serial.printf("Sent: %s | Response: %d\n", data.c_str(), responseCode);
    } else {
      Serial.printf("Error sending: %s\n", http.errorToString(responseCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected!");
  }
}

unsigned long lastGPSLog = 0;

void loop() {
  // Continuously read GPS data
  updateGPS();

  // Log GPS status every 5 seconds
  if (millis() - lastGPSLog >= 5000) {
    lastGPSLog = millis();
    if (gpsFix) {
      Serial.printf("GPS FIX | Lat: %.6f  Lon: %.6f  Sats: %d  UTC: %s\n",
                     gpsLat, gpsLon, gpsSats, gpsTime.c_str());
    } else {
      Serial.println("GPS: NO FIX — waiting for satellites...");
    }
  }

  // Detect trigger (falling edge — pin shorted to GND)
  bool currentState = digitalRead(triggerPin);

  if (lastState == HIGH && currentState == LOW) {
    String sensorData = generateSensorData();
    sendToServer(sensorData);

    // Blink LED to indicate send
    digitalWrite(2, LOW);
    delay(100);
    digitalWrite(2, HIGH);
  }

  lastState = currentState;
  delay(50); // Debounce
}