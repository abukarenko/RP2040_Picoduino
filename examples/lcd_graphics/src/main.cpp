#include <Arduino.h>
#include <PicoDuinoLCD.h>

using namespace PicoDuinoColor;

PicoDuinoLCD lcd;

void setup() {
  Serial.begin(115200);
  lcd.begin();

  lcd.fillScreen(BLACK);
  lcd.setTextSize(2);
  lcd.setTextColor(CYAN, BLACK);
  lcd.setCursor(10, 10);
  lcd.println("PicoDuino LCD");

  lcd.drawRect(8, 40, lcd.width() - 16, lcd.height() - 48, WHITE);
  lcd.fillCircle(80, 100, 32, YELLOW);
  lcd.fillCircle(160, 140, 32, BLUE);
  lcd.fillRect(220, 70, 70, 90, RED);
  lcd.drawLine(20, 220, 300, 60, GREEN);
}

void loop() {
}
