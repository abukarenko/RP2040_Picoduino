#include <Arduino.h>
#include <PicoDuinoLCD.h>
#include <PicoDuinoTouch.h>

using namespace PicoDuinoColor;

PicoDuinoLCD lcd;
PicoDuinoTouch touch(lcd);

uint16_t color = RED;

void setup() {
  Serial.begin(115200);
  lcd.begin();
  lcd.fillScreen(BLACK);
  lcd.setTextSize(2);
  lcd.setTextColor(CYAN, BLACK);
  lcd.setCursor(8, 8);
  lcd.println("Touch paint");
  lcd.drawFastHLine(0, 34, lcd.width(), WHITE);
}

void loop() {
  PicoDuinoTouchPoint raw = touch.readRaw();
  if (!raw.touched) {
    delay(15);
    return;
  }

  int16_t x = 0;
  int16_t y = 0;
  touch.toScreen(raw, x, y);
  lcd.fillCircle(x, y, 2, color);

  Serial.print("raw=");
  Serial.print(raw.x);
  Serial.print(",");
  Serial.print(raw.y);
  Serial.print(" z=");
  Serial.print(raw.z);
  Serial.print(" screen=");
  Serial.print(x);
  Serial.print(",");
  Serial.println(y);
  delay(20);
}
