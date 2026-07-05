#include "PicoDuinoLCD35.h"

#include <hardware/structs/sio.h>

namespace {
// Measured PicoDuino UNO -> mcufriend-style 8-bit TFT shield wiring.
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

constexpr uint16_t TFT_W = 320;
constexpr uint16_t TFT_H = 480;

constexpr uint8_t MADCTL_MY = 0x80;
constexpr uint8_t MADCTL_MX = 0x40;
constexpr uint8_t MADCTL_MV = 0x20;
constexpr uint8_t MADCTL_BGR = 0x08;
}  // namespace

uint16_t picoDuinoLCD35Color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

PicoDuinoLCD35::PicoDuinoLCD35() : Adafruit_GFX(TFT_W, TFT_H) {}

void PicoDuinoLCD35::begin() {
  initPins();
  reset();
  initPanel();
  fillScreen(PicoDuinoLCD35Color::BLACK);
}

void PicoDuinoLCD35::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (x < 0 || y < 0 || x >= width() || y >= height()) {
    return;
  }
  setAddrWindow(x, y, x, y);
  writeColor(color);
}

void PicoDuinoLCD35::fillScreen(uint16_t color) {
  fillRect(0, 0, width(), height(), color);
}

void PicoDuinoLCD35::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
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

void PicoDuinoLCD35::setRotation(uint8_t r) {
  rotation = r & 3;
  uint8_t madctl = MADCTL_BGR;

  switch (rotation) {
    case 0:
      _width = TFT_W;
      _height = TFT_H;
      madctl |= MADCTL_MX;
      break;
    case 1:
      _width = TFT_H;
      _height = TFT_W;
      madctl |= MADCTL_MV;
      break;
    case 2:
      _width = TFT_W;
      _height = TFT_H;
      madctl |= MADCTL_MY;
      break;
    default:
      _width = TFT_H;
      _height = TFT_W;
      madctl |= MADCTL_MX | MADCTL_MY | MADCTL_MV;
      break;
  }

  writeCommandData(0x36, {madctl});
}

void PicoDuinoLCD35::invertDisplay(bool invert) {
  writeCommand(invert ? 0x21 : 0x20);
}

void PicoDuinoLCD35::printDiagnostics(Stream &out) {
  out.println("PicoDuino 3.5 inch TFT pin map:");
  out.println("D0=GPIO10 D1=GPIO11 D2=GPIO2 D3=GPIO3");
  out.println("D4=GPIO6 D5=GPIO7 D6=GPIO8 D7=GPIO9");
  out.println("RD=GPIO26 WR=GPIO27 RS=GPIO28 CS=GPIO29 RST=GPIO14");
  out.println("Controller init: ILI9486-style 320x480 16-bit color");
}

void PicoDuinoLCD35::restoreBusPins() {
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

void PicoDuinoLCD35::initPins() {
  restoreBusPins();
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH);

  for (uint8_t pin : TFT_D) {
    digitalWrite(pin, LOW);
  }
}

void PicoDuinoLCD35::reset() {
  digitalWrite(TFT_RST, LOW);
  delay(30);
  digitalWrite(TFT_RST, HIGH);
  delay(150);
}

void PicoDuinoLCD35::initPanel() {
  writeCommand(0x01);
  delay(150);
  writeCommand(0x11);
  delay(150);

  // Common ILI9486/ILI9488-style 3.5" MCUFRIEND shield initialization.
  writeCommandData(0x3A, {0x55});  // 16-bit/pixel DBI
  writeCommandData(0xB0, {0x00});
  writeCommandData(0xB1, {0xB0, 0x11});
  writeCommandData(0xB4, {0x02});
  writeCommandData(0xB6, {0x02, 0x02, 0x3B});
  writeCommandData(0xC0, {0x0D, 0x0D});
  writeCommandData(0xC1, {0x43, 0x00});
  writeCommandData(0xC2, {0x00});
  writeCommandData(0xC5, {0x00, 0x48});
  writeCommandData(0xE0, {0x0F, 0x24, 0x1C, 0x0A, 0x0F, 0x08, 0x43, 0x88,
                          0x32, 0x0F, 0x10, 0x06, 0x0F, 0x07, 0x00});
  writeCommandData(0xE1, {0x0F, 0x38, 0x30, 0x09, 0x0F, 0x0F, 0x4E, 0x77,
                          0x3C, 0x07, 0x10, 0x05, 0x23, 0x1B, 0x00});

  setRotation(1);
  invertDisplay(false);
  writeCommand(0x29);
  delay(120);
}

void PicoDuinoLCD35::writeBus(uint8_t value) {
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

void PicoDuinoLCD35::writeDelay() {
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

void PicoDuinoLCD35::writeCommand(uint8_t command) {
  digitalWrite(TFT_CS, LOW);
  digitalWrite(TFT_RS, LOW);
  writeBus(command);
  digitalWrite(TFT_RS, HIGH);
  digitalWrite(TFT_CS, HIGH);
}

void PicoDuinoLCD35::writeCommandData(uint8_t command,
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

void PicoDuinoLCD35::writeData16(uint16_t data) {
  digitalWrite(TFT_CS, LOW);
  digitalWrite(TFT_RS, HIGH);
  writeBus(data >> 8);
  writeBus(data & 0xFF);
  digitalWrite(TFT_CS, HIGH);
}

void PicoDuinoLCD35::writeColor(uint16_t color) {
  writeData16(color);
}

void PicoDuinoLCD35::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
                                   uint16_t y1) {
  writeCommand(0x2A);
  writeData16(x0);
  writeData16(x1);
  writeCommand(0x2B);
  writeData16(y0);
  writeData16(y1);
  writeCommand(0x2C);
}
