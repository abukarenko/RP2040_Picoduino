#pragma once

#include <Arduino.h>
#include <PicoDuinoLCD.h>

struct PicoDuinoTouchPoint {
  int x = 0;
  int y = 0;
  int z = 0;
  bool touched = false;
};

struct PicoDuinoScreenPoint {
  int16_t x = 0;
  int16_t y = 0;
  bool touched = false;
};

class PicoDuinoTouch {
public:
  explicit PicoDuinoTouch(PicoDuinoLCD &display);

  PicoDuinoTouchPoint readRaw();
  PicoDuinoScreenPoint readScreen();
  void toScreen(const PicoDuinoTouchPoint &raw, int16_t &x, int16_t &y);

private:
  int readAnalogAverage(uint8_t pin);
  int clampInt(int value, int lo, int hi);

  PicoDuinoLCD &lcd;
};
