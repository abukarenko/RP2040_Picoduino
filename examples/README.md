# Examples

These examples are small standalone PlatformIO projects. Each one uses the
libraries from `../../lib`.

## lcd_graphics

Simple ST7789V parallel LCD drawing test.

## lcd35_graphics

3.5 inch 320x480 parallel LCD drawing test. It uses the same PicoDuino UNO
8-bit pin map and an ILI9486-style initialization sequence.

## touch_paint

Simple resistive touch paint test with the current 8x8 calibration table.

## wifi_scan

ESP8285 AT Wi-Fi scan example. The ESP UART is expected on the board's configured
`Serial1` pins.
