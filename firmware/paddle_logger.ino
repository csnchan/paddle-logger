#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <SD.h>
#include <SPI.h>
#include <HX711.h>
#include <Adafruit_BNO08x.h>
#include <TinyGPSPlus.h>
#include <Preferences.h>

// ==================== CHANGE THESE ====================
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Pins
#define HX711_1_DT   4
#define HX711_1_SCK  5
#define HX711_2_DT   6
#define HX711_2_SCK  7
#define BNO_SDA      8
#define BNO_SCL      9
#define GPS_RX       17
#define GPS_TX       18
#define SD_CS        10
#define SD_MOSI      11
#define SD_SCK       12
#define SD_MISO      13
#define STATUS_LED   14

// ==================== OBJECTS ====================
WebServer server(80);
HX711 scale1, scale2;
Adafruit_BNO08x bno08x;
TinyGPSPlus gps;
HardwareSerial GPSSerial(1);
Preferences prefs;

// ==================== STATE ====================
bool logging = false;
File logFile;
unsigned long lastLog = 0;
const unsigned long LOG_INTERVAL = 50;

float strain1 = 0, strain2 = 0;
float qw = 0, qx = 0, qy = 0, qz = 0;
float ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
double lat = 0, lon = 0, speed = 0;
String gpsTime = "";
int satellites = 0;

float offset1 = 0, scale1 = 1.0f;
float offset2 = 0, scale2 = 1.0f;

// ==================== HELPERS ====================
void setStatusLED(bool on) {
  digitalWrite(STATUS_LED, on ? HIGH : LOW);
}

String getTimestamp() {
  if (gps.time.isValid()) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
             gps.time.hour(), gps.time.minute(), gps.time.second());
    return String(buf);
  }
  return String(millis());
}

void updateSensors() {
  if (scale1.is_ready()) {
    long raw = scale1.read();
    strain1 = (raw - offset1) * scale1;
  }
  if (scale2.is_ready()) {
    long raw = scale2.read();
    strain2 = (raw - offset2) * scale2;
  }

  if (bno08x.getSensorEvent()) {
    if (bno08x.getSensorEventID() == SENSOR_TYPE_ROTATION_VECTOR) {
      qw = bno08x.getQuatReal();
      qx = bno08x.getQuatI();
      qy = bno08x.getQuatJ();
      qz = bno08x.getQuatK();
    }
    if (bno08x.getSensorEventID() == SENSOR_TYPE_ACCELEROMETER) {
      ax = bno08x.getAccelX();
      ay = bno08x.getAccelY();
      az = bno08x.getAccelZ();
    }
    if (bno08x.getSensorEventID() == SENSOR_TYPE_GYROSCOPE) {
      gx = bno08x.getGyroX();
      gy = bno08x.getGyroY();
      gz = bno08x.getGyroZ();
    }
  }

  while (GPSSerial.available()) gps.encode(GPSSerial.read());
  if (gps.location.isValid()) {
    lat = gps.location.lat();
    lon = gps.location.lng();
  }
  if (gps.speed.isValid()) speed = gps.speed.kmph();
  satellites = gps.satellites.value();
  if (gps.time.isValid()) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
             gps.time.hour(), gps.time.minute(), gps.time.second());
    gpsTime = buf;
  }
}

void writeCSVHeader() {
  if (logFile) {
    logFile.println("time,strain1_n,strain2_n,qw,qx,qy,qz,accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z,latitude,longitude,speed,gps_time,satellites");
    logFile.flush();
  }
}

void logRow() {
  if (!logging || !logFile) return;
  if (millis() - lastLog < LOG_INTERVAL) return;
  lastLog = millis();

  logFile.print(getTimestamp()); logFile.print(",");
  logFile.print(strain1, 3); logFile.print(",");
  logFile.print(strain2, 3); logFile.print(",");
  logFile.print(qw, 4); logFile.print(",");
  logFile.print(qx, 4); logFile.print(",");
  logFile.print(qy, 4); logFile.print(",");
  logFile.print(qz, 4); logFile.print(",");
  logFile.print(ax, 3); logFile.print(",");
  logFile.print(ay, 3); logFile.print(",");
  logFile.print(az, 3); logFile.print(",");
  logFile.print(gx, 3); logFile.print(",");
  logFile.print(gy, 3); logFile.print(",");
  logFile.print(gz, 3); logFile.print(",");
  logFile.print(lat, 6); logFile.print(",");
  logFile.print(lon, 6); logFile.print(",");
  logFile.print(speed, 2); logFile.print(",");
  logFile.print(gpsTime); logFile.print(",");
  logFile.println(satellites);
  logFile.flush();
}

// ==================== WEB HANDLERS ====================
void handleRoot() {
  File f = SPIFFS.open("/index.html", "r");
  if (f) {
    server.streamFile(f, "text/html");
    f.close();
  } else {
    server.send(404, "text/plain", "index.html missing – upload SPIFFS data folder");
  }
}

void handleStart() {
  if (!logging) {
    logFile = SD.open("/paddle.csv", FILE_APPEND);
    if (logFile) {
      if (logFile.size() == 0) writeCSVHeader();
      logging = true;
      setStatusLED(true);
    }
  }
  server.send(200, "text/plain", "ok");
}

void handleStop() {
  logging = false;
  if (logFile) logFile.close();
  setStatusLED(false);
  server.send(200, "text/plain", "ok");
}

void handleDownload() {
  if (SD.exists("/paddle.csv")) {
    File f = SD.open("/paddle.csv", FILE_READ);
    server.streamFile(f, "text/csv");
    f.close();
  } else {
    server.send(404, "text/plain", "no file");
  }
}

void handleStatus() {
  server.send(200, "text/plain", logging ? "true" : "false");
}

void handleData() {
  String json = "{";
  json += "\"strain1\":" + String(strain1, 3) + ",";
  json += "\"strain2\":" + String(strain2, 3) + ",";
  json += "\"timestamp\":\"" + getTimestamp() + "\",";
  json += "\"qw\":" + String(qw, 4) + ",";
  json += "\"qx\":" + String(qx, 4) + ",";
  json += "\"qy\":" + String(qy, 4) + ",";
  json += "\"qz\":" + String(qz, 4) + ",";
  json += "\"accel_x\":" + String(ax, 3) + ",";
  json += "\"accel_y\":" + String(ay, 3) + ",";
  json += "\"accel_z\":" + String(az, 3) + ",";
  json += "\"gyro_x\":" + String(gx, 3) + ",";
  json += "\"gyro_y\":" + String(gy, 3) + ",";
  json += "\"gyro_z\":" + String(gz, 3);
  json += "}";
  server.send(200, "application/json", json);
}

void handleTare() {
  long sum1 = 0, sum2 = 0;
  int n = 10;
  for (int i = 0; i < n; i++) {
    if (scale1.is_ready()) sum1 += scale1.read();
    if (scale2.is_ready()) sum2 += scale2.read();
    delay(10);
  }
  offset1 = sum1 / (float)n;
  offset2 = sum2 / (float)n;
  prefs.putFloat("off1", offset1);
  prefs.putFloat("off2", offset2);
  server.send(200, "text/plain", "tared");
}

void handleCalPoint() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "no body");
    return;
  }
  String body = server.arg("plain");
  float load_n = 0;
  int nPos = body.indexOf("\"load_n\":");
  if (nPos > 0) load_n = body.substring(nPos + 9).toFloat();

  long raw1 = scale1.is_ready() ? scale1.read() : 0;
  long raw2 = scale2.is_ready() ? scale2.read() 
