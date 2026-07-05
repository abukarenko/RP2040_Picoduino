# RP2040 PicoDuino UNO

PlatformIO/Arduino project for the PicoDuino UNO RP2040 board with onboard
ESP8285 Wi-Fi and a 2.4 inch mcufriend-style ST7789V parallel TFT shield.

The project currently contains a simple resistive touch paint program in
`src/main.cpp`. The hardware-specific code is split into local libraries under
`lib/`, so future sketches can reuse the LCD, touch, and ESP AT Wi-Fi support.

## Libraries

- `lib/PicoDuinoLCD` - ST7789V 8-bit parallel LCD driver for the measured
  PicoDuino UNO pin map.
- `lib/PicoDuinoLCD35` - 3.5 inch 320x480 parallel LCD driver for the same
  PicoDuino UNO pin map, using an ILI9486-style initialization.
- `lib/PicoDuinoTouch` - resistive touch reader with the measured 8x8 raw
  calibration table.
- `lib/PicoDuinoWiFi` - ESP8285/ESP8266 AT-command helper for testing,
  connecting, scanning networks, and reading the local IP address.

## Measured LCD Pin Map

| LCD signal | RP2040 GPIO |
| --- | --- |
| D0 | GPIO10 |
| D1 | GPIO11 |
| D2 | GPIO2 |
| D3 | GPIO3 |
| D4 | GPIO6 |
| D5 | GPIO7 |
| D6 | GPIO8 |
| D7 | GPIO9 |
| RD | GPIO26 |
| WR | GPIO27 |
| RS | GPIO28 |
| CS | GPIO29 |
| RST | GPIO14 |

## Touch Pins

The shield follows the common MCUFRIEND `Touch_shield_new` wiring:

| Touch signal | Shield signal | RP2040 GPIO |
| --- | --- | --- |
| XP | D6 | GPIO8 |
| XM | A2 | GPIO28 |
| YP | A1 | GPIO27 |
| YM | D7 | GPIO9 |

## Build

```powershell
platformio run
```

The root project uses `copy_uf2.py` as a post-build script. It tries to reset
the RP2040 through the USB serial port and copy the generated UF2 to the
`RPI-RP2` drive automatically.

## Examples

Examples are in `examples/`:

- `examples/lcd_graphics` - basic LCD drawing test.
- `examples/lcd35_graphics` - 3.5 inch LCD geometry and color-bar test.
- `examples/touch_paint` - minimal touch paint sketch.
- `examples/wifi_scan` - ESP AT Wi-Fi scan shown on the TFT.

Build an example from its folder:

```powershell
cd examples\lcd_graphics
platformio run
```

## Wi-Fi Helper Scripts

The Python helper scripts use environment variables for Wi-Fi credentials:

```powershell
$env:PICODUINO_WIFI_SSID = "your-ssid"
$env:PICODUINO_WIFI_PASSWORD = "your-password"
python connect_wifi.py
```

Do not commit real Wi-Fi passwords.
