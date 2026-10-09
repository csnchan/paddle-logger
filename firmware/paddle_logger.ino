#include "pins.h"
#include <SD.h>
#include <SPI.h>

// From calibration. Replace after the zero and known-load test.
float strainZero = 0;
float strainScale = 1;  // newtons per ADC count

File logFile;
bool logging = false;
uint32_t t0 = 0;

bool pinsReady() {
  return PIN_STRAIN_1 >= 0 && PIN_SD_CS >= 0 && PIN_SD_SCK >= 0 &&
         PIN_SD_MOSI >= 0 && PIN_SD_MISO >= 0;
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("paddle logger");
  if (!pinsReady()) {
    Serial.println("pins.h still has -1. USB test only. No SD write.");
    return;
  }
  SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  if (!SD.begin(PIN_SD_CS)) {
    Serial.println("SD mount failed");
    return;
  }
  Serial.println("SD ready. Press the log button to start.");
  pinMode(PIN_LOG_BUTTON, INPUT_PULLUP);
}

void startLog() {
  logFile = SD.open("/paddle.csv", FILE_WRITE);
  if (!logFile) {
    Serial.println("cannot open paddle.csv");
    return;
  }
  logFile.printf("# zero=%.2f scale=%.8f\n", strainZero, strainScale);
  logFile.println("t_ms,strain_raw,force_n");
  t0 = millis();
  logging = true;
  Serial.println("logging");
}

void stopLog() {
  if (logFile) logFile.close();
  logging = false;
  Serial.println("stopped");
}

void loop() {
  if (!pinsReady()) {
    delay(1000);
    return;
  }
  if (digitalRead(PIN_LOG_BUTTON) == LOW) {
    delay(30);
    if (logging) stopLog();
    else startLog();
    while (digitalRead(PIN_LOG_BUTTON) == LOW) delay(10);
  }
  if (!logging) return;
  int raw = analogRead(PIN_STRAIN_1);
  float force = strainScale * (raw - strainZero);
  logFile.printf("%lu,%d,%.2f\n", millis() - t0, raw, force);
  logFile.flush();
  delay(10);
}
