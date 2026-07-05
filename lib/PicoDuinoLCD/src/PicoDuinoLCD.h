#pragma once

#include <Adafruit_GFX.h>
#include <Arduino.h>

namespace PicoDuinoColor {
static constexpr uint16_t BLACK = 0x0000;
static constexpr uint16_t BLUE = 0x001F;
static constexpr uint16_t RED = 0xF800;
static constexpr uint16_t GREEN = 0x07E0;
static constexpr uint16_t CYAN = 0x07FF;
static constexpr uint16_t MAGENTA = 0xF81F;
static constexpr uint16_t YELLOW = 0xFFE0;
static constexpr uint16_t WHITE = 0xFFFF;
}  // namespace PicoDuinoColor

uint16_t picoDuinoColor565(uint8_t r, uint8_t g, uint8_t b);

class PicoDuinoLCD : public Adafruit_GFX {
public:
  PicoDuinoLCD();

  void begin();
  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  void fillScreen(uint16_t color) override;
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                uint16_t color) override;
  void setRotation(uint8_t r) override;
  void printDiagnostics(Stream &out = Serial);
  void restoreBusPins();

private:
  void initPins();
  void reset();
  void initPanel();
  void writeBus(uint8_t value);
  void writeDelay();
  void writeCommand(uint8_t command);
  void writeCommandData(uint8_t command, std::initializer_list<uint8_t> data);
  void writeData16(uint16_t data);
  void writeColor(uint16_t color);
  void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
};
