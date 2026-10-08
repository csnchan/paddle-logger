# Paddle logger

ESP32-S3 board. One 1000 ohm full-bridge strain gauge, onboard BMI270, GPS module, microSD.

The phone dashboard is not required. This repo records a CSV on the SD card.

## Order

1. Assemble and meter-check the board. Do not log yet.
2. Confirm the unloaded gauge number moves when the shaft bends.
3. Calibrate zero and one known load.
4. Only then fill the pin numbers in `firmware/pins.h` and record to the SD card.

## CSV columns

t_ms,strain_raw,force_n,ax,ay,az,gx,gy,gz,lat,lon,speed_mps,sats

force_n uses force = scale * (strain_raw - zero).
scale and zero come from calibration and are written in the header of every file.
