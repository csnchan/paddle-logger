# Paddle Logger

ESP32-S3. Two HX711 modules (one per 4-wire full-bridge strain gauge on the same shaft), BNO085 (quaternion + accel + gyro), GPS module, microSD.

The page is stored on the board and served after the board joins Wi-Fi. Logging continues to the SD card even if Wi-Fi drops.

## Order
1. USB only. Upload the sketch and confirm the board prints its IP (or “board is alive” style message).
2. Add HX711 #1 and confirm the raw reading changes when the gauge is pressed.
3. Add HX711 #2.
4. Add the BNO085.
5. Add the GPS.
6. Add the microSD card and confirm a CSV is written.
7. Upload the SPIFFS data folder so `data/index.html` is on the board, then open the page.
8. Only then use a battery and waterproof box.

## How to open the page
1. Set Wi-Fi SSID and password at the top of `firmware/paddle_logger.ino`.
2. Upload the sketch (ESP32S3 Dev Module, USB CDC On Boot = Enabled, Flash Size = 16 MB, PSRAM = OPI PSRAM).
3. Upload the SPIFFS filesystem (Tools → ESP32 Sketch Data Upload or Upload Filesystem Image).
4. Open Serial Monitor (115200). Note the IP address.
5. On a phone or computer on the same Wi-Fi, go to `http://<that-IP>/`.

The page title is exactly “Paddle Logger”.

## CSV columns
time,strain1_n,strain2_n,qw,qx,qy,qz,accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z,latitude,longitude,speed,gps_time,satellites

Calibration (tare + known loads) is done from the page. Both gauges are recorded together with one hanging weight.
