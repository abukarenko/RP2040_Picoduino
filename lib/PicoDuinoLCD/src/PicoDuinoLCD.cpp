#include "PicoDuinoLCD.h"

#include <hardware/structs/sio.h>

namespace {
// Measured PicoDuino UNO -> mcufriend 2.4" ST7789V shield wiring.
const uint8_t TFT_D[8] = {
    10, // LCD_D0
    11, // LCD_D1
    2,  // LCD_D2
    3,  // LCD_D3
    6,  // LCD_D4
    7,  // LCD_D5
    8,  // LCD_D6
    9,  // LCD_D7
};

constexpr uint8_t TFT_RD = 26;
constexpr uint8_t TFT_WR = 27;
constexpr uint8_t TFT_RS = 28;
constexpr uint8_t TFT_CS = 29;
constexpr uint8_t TFT_RST = 14;

constexpr uint32_t TFT_DATA_MASK =
    (1UL << 2) | (1UL << 3) | (1UL << 6) | (1UL << 7) |
    (1UL << 8) | (1UL << 9) | (1UL << 10) | (1UL << 11);
constexpr uint32_t TFT_WR_MASK = 1UL << TFT_WR;

constexpr uint16_t TFT_W = 240;
constexpr uint16_t TFT_H = 320;
}  // namespace

uint16_t picoDuinoColor565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

PicoDuinoLCD::PicoDuinoLCD() : Adafruit_GFX(TFT_W, TFT_H) {}

void PicoDuinoLCD::begin() {
  initPins();
  reset();
  initPanel();
  fillScreen(PicoDuinoColor::BLACK);
}

void PicoDuinoLCD::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (x < 0 || y < 0 || x >= width() || y >= height()) {
    return;
  }
  setAddrWindow(x, y, x, y);
  writeColor(color);
}

void PicoDuinoLCD::fillScreen(uint16_t color) {
  fillRect(0, 0, width(), height(), color);
}

void PicoDuinoLCD::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                            uint16_t color) {
  if (w <= 0 || h <= 0) {
    return;
  }
  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > width()) {
    w = width() - x;
  }
  if (y + h > height()) {
    h = height() - y;
  }
  if (w <= 0 || h <= 0) {
    return;
  }

  setAddrWindow(x, y, x + w - 1, y + h - 1);
  digitalWrite(TFT_CS, LOW);
  digitalWrite(TFT_RS, HIGH);
  for (uint32_t i = 0; i < static_cast<uint32_t>(w) * h; i++) {
    writeBus(color >> 8);
    writeBus(color & 0xFF);
  }
  digitalWrite(TFT_CS, HIGH);
}

void PicoDuinoLCD::setRotation(uint8_t r) {
  rotation = r & 3;
  uint8_t madctl;

  switch (rotation) {
    case 0:
      _width = TFT_W;
      _height = TFT_H;
      madctl = 0x00;
      break;
    case 1:
      _width = TFT_H;
      _height = TFT_W;
      madctl = 0x20;
      break;
    case 2:
      _width = TFT_W;
      _height = TFT_H;
      madctl = 0xC0;
      break;
    default:
      _width = TFT_H;
      _height = TFT_W;
      madctl = 0xA0;
      break;
  }

  writeCommandData(0x36, {madctl});
}

void PicoDuinoLCD::printDiagnostics(Stream &out) {
  out.println("Measured TFT pin map:");
  out.println("D0=GPIO10 D1=GPIO11 D2=GPIO2 D3=GPIO3");
  out.println("D4=GPIO6 D5=GPIO7 D6=GPIO8 D7=GPIO9");
  out.println("RD=GPIO26 WR=GPIO27 RS=GPIO28 CS=GPIO29 RST=GPIO14");
}

void PicoDuinoLCD::restoreBusPins() {
  pinMode(TFT_RD, OUTPUT);
  pinMode(TFT_WR, OUTPUT);
  pinMode(TFT_RS, OUTPUT);
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_RD, HIGH);
  digitalWrite(TFT_WR, HIGH);
  digitalWrite(TFT_RS, HIGH);
  digitalWrite(TFT_CS, HIGH);
  for (uint8_t pin : TFT_D) {
    pinMode(pin, OUTPUT);
  }
}

void PicoDuinoLCD::initPins() {
  restoreBusPins();
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH);

  for (uint8_t pin : TFT_D) {
    digitalWrite(pin, LOW);
  }
}

void PicoDuinoLCD::reset() {
  digitalWrite(TFT_RST, LOW);
  delay(30);
  digitalWrite(TFT_RST, HIGH);
  delay(150);
}

void PicoDuinoLCD::initPanel() {
  writeCommand(0x01);
  delay(150);
  writeCommand(0x11);
  delay(150);

  // ST7789V init table from MCUFRIEND_kbv for ID 0x7789.
  writeCommandData(0xB2, {0x0C, 0x0C, 0x00, 0x33, 0x33});
  writeCommandData(0xB7, {0x35});
  writeCommandData(0xBB, {0x2B});
  writeCommandData(0xC0, {0x04});
  writeCommandData(0xC2, {0x01, 0xFF});
  writeCommandData(0xC3, {0x11});
  writeCommandData(0xC4, {0x20});
  writeCommandData(0xC6, {0x0F});
  writeCommandData(0xD0, {0xA4, 0xA1});
  writeCommandData(0xE0, {0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37,
                          0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19});
  writeCommandData(0xE1, {0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D,
                          0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19});

  writeCommandData(0x3A, {0x55});
  setRotation(1);
  writeCommand(0x20);
  writeCommand(0x29);
  delay(120);
}

void PicoDuinoLCD::writeBus(uint8_t value) {
  uint32_t mapped = ((value & 0x0C) << 0) |  // D2,D3 -> GPIO2,GPIO3
                    ((value & 0xF0) << 2) |  // D4..D7 -> GPIO6..GPIO9
                    ((value & 0x03) << 10);  // D0,D1 -> GPIO10,GPIO11

  sio_hw->gpio_clr = TFT_DATA_MASK;
  sio_hw->gpio_set = mapped;
  writeDelay();
  sio_hw->gpio_clr = TFT_WR_MASK;
  writeDelay();
  sio_hw->gpio_set = TFT_WR_MASK;
  writeDelay();
}

void PicoDuinoLCD::writeDelay() {
  asm volatile(
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t");
}

void PicoDuinoLCD::writeCommand(uint8_t command) {
  digitalWrite(TFT_CS, LOW);
  digitalWrite(TFT_RS, LOW);
  writeBus(command);
  digitalWrite(TFT_RS, HIGH);
  digitalWrite(TFT_CS, HIGH);
}

void PicoDuinoLCD::writeCommandData(uint8_t command,
                                    std::initializer_list<uint8_t> data) {
  digitalWrite(TFT_CS, LOW);
  digitalWrite(TFT_RS, LOW);
  writeBus(command);
  digitalWrite(TFT_RS, HIGH);
  for (uint8_t value : data) {
    writeBus(value);
  }
  digitalWrite(TFT_CS, HIGH);
}

void PicoDuinoLCD::writeData16(uint16_t data) {
  digitalWrite(TFT_CS, LOW);
  digitalWrite(TFT_RS, HIGH);
  writeBus(data >> 8);
  writeBus(data & 0xFF);
  digitalWrite(TFT_CS, HIGH);
}

void PicoDuinoLCD::writeColor(uint16_t color) {
  writeData16(color);
}

void PicoDuinoLCD::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
                                 uint16_t y1) {
  writeCommand(0x2A);
  writeData16(x0);
  writeData16(x1);
  writeCommand(0x2B);
  writeData16(y0);
  writeData16(y1);
  writeCommand(0x2C);
}
