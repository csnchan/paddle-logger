#pragma once
// Fill these from the schematic before enabling the SD card.
// Leave a pin at -1 until you know it. The sketch will refuse to log.

static const int PIN_STRAIN_1 = -1;  // WHEAT_ADC1, gauge socket J1
static const int PIN_STRAIN_2 = -1;  // WHEAT_ADC2, leave unused
static const int PIN_LOG_BUTTON = -1;  // BTN_LOG
static const int PIN_SD_CS = -1;
static const int PIN_SD_SCK = -1;
static const int PIN_SD_MOSI = -1;
static const int PIN_SD_MISO = -1;
static const int PIN_GPS_RX = -1;  // ESP32 pin that hears the GPS TX wire
static const int PIN_GPS_TX = -1;
