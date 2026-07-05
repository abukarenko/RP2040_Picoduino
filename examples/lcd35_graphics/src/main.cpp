#include <Arduino.h>
#include <PicoDuinoLCD35.h>

using namespace PicoDuinoLCD35Color;

PicoDuinoLCD35 lcd;

void drawCornerLabels() {
  lcd.setTextSize(2);
  lcd.setTextColor(WHITE, BLACK);
  lcd.setCursor(6, 6);
  lcd.print("0,0");

  lcd.setCursor(lcd.width() - 76, 6);
  lcd.print("MAX X");

  lcd.setCursor(6, lcd.height() - 22);
  lcd.print("MAX Y");

  lcd.setCursor(lcd.width() - 94, lcd.height() - 22);
  lcd.print("480x320");
}

void drawColorBars() {
  const uint16_t colors[] = {RED, ORANGE, YELLOW, GREEN, CYAN, BLUE, MAGENTA, WHITE};
  int16_t barW = lcd.width() / 8;
  for (uint8_t i = 0; i < 8; i++) {
    lcd.fillRect(i * barW, 44, barW, 54, colors[i]);
  }
}

void drawGeometry() {
  lcd.fillScreen(BLACK);
  lcd.drawRect(0, 0, lcd.width(), lcd.height(), WHITE);
  lcd.drawRect(4, 4, lcd.width() - 8, lcd.height() - 8, GREEN);
  lcd.drawFastHLine(0, lcd.height() / 2, lcd.width(), CYAN);
  lcd.drawFastVLine(lcd.width() / 2, 0, lcd.height(), CYAN);
  lcd.drawLine(0, 0, lcd.width() - 1, lcd.height() - 1, YELLOW);
  lcd.drawLine(lcd.width() - 1, 0, 0, lcd.height() - 1, MAGENTA);
  drawColorBars();
  drawCornerLabels();
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    delay(10);
  }

  Serial.println();
  Serial.println("PicoDuino 3.5 inch TFT test");
  lcd.printDiagnostics();
  lcd.begin();
  drawGeometry();
}

void loop() {
  static uint32_t last = 0;
  if (millis() - last < 1000) {
    return;
  }
  last = millis();

  static uint16_t r = 16;
  lcd.drawCircle(lcd.width() / 2, lcd.height() / 2, r, WHITE);
  r += 12;
  if (r > 140) {
    r = 16;
    drawGeometry();
  }
}
