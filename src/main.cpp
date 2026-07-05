#include <Arduino.h>
#include <PicoDuinoLCD.h>
#include <PicoDuinoTouch.h>
#include <PicoDuinoWiFiESPAT.h>

using namespace PicoDuinoColor;

static constexpr uint8_t PEN_RADIUS = 2;

PicoDuinoLCD lcd;
PicoDuinoTouch touch(lcd);

static uint16_t currentColor = RED;
static int16_t boxSize = 0;

static void drawStatus(const char *title, const char *line1 = "",
                       const char *line2 = "") {
  lcd.fillScreen(BLACK);
  lcd.setTextSize(2);
  lcd.setTextColor(CYAN, BLACK);
  lcd.setCursor(8, 8);
  lcd.println(title);
  lcd.drawFastHLine(0, 32, lcd.width(), picoDuinoColor565(40, 40, 40));
  lcd.setTextSize(1);
  lcd.setTextColor(WHITE, BLACK);
  lcd.setCursor(8, 48);
  lcd.println(line1);
  lcd.setCursor(8, 64);
  lcd.println(line2);
}

static void drawPalette() {
  boxSize = lcd.width() / 6;
  lcd.fillScreen(BLACK);
  lcd.fillRect(0, 0, boxSize, boxSize, RED);
  lcd.fillRect(boxSize, 0, boxSize, boxSize, YELLOW);
  lcd.fillRect(boxSize * 2, 0, boxSize, boxSize, GREEN);
  lcd.fillRect(boxSize * 3, 0, boxSize, boxSize, CYAN);
  lcd.fillRect(boxSize * 4, 0, boxSize, boxSize, BLUE);
  lcd.fillRect(boxSize * 5, 0, boxSize, boxSize, MAGENTA);
  lcd.drawRect(0, 0, boxSize, boxSize, WHITE);

  lcd.setTextSize(1);
  lcd.setTextColor(WHITE, BLACK);
  lcd.setCursor(4, lcd.height() - 22);
  lcd.print("Touch raw/cal: XP=8 XM=28 YP=27 YM=9");
}

static void selectColor(int16_t x) {
  uint16_t old = currentColor;
  if (x < boxSize) currentColor = RED;
  else if (x < boxSize * 2) currentColor = YELLOW;
  else if (x < boxSize * 3) currentColor = GREEN;
  else if (x < boxSize * 4) currentColor = CYAN;
  else if (x < boxSize * 5) currentColor = BLUE;
  else currentColor = MAGENTA;

  if (old != currentColor) {
    drawPalette();
    int16_t selected = x / boxSize;
    lcd.drawRect(selected * boxSize, 0, boxSize, boxSize, WHITE);
    lcd.drawRect(selected * boxSize + 1, 1, boxSize - 2, boxSize - 2, WHITE);
  }
}

static void drawRawValues(const PicoDuinoTouchPoint &raw, int16_t sx,
                          int16_t sy) {
  lcd.fillRect(0, lcd.height() - 12, lcd.width(), 12, BLACK);
  lcd.setCursor(4, lcd.height() - 10);
  lcd.setTextSize(1);
  lcd.setTextColor(WHITE, BLACK);
  lcd.print("raw x=");
  lcd.print(raw.x);
  lcd.print(" y=");
  lcd.print(raw.y);
  lcd.print(" -> ");
  lcd.print(sx);
  lcd.print(",");
  lcd.print(sy);
}

static void handleTouchPaint() {
  static uint32_t lastPrint = 0;

  PicoDuinoTouchPoint raw = touch.readRaw();
  if (!raw.touched) {
    delay(15);
    return;
  }

  int16_t x = 0;
  int16_t y = 0;
  touch.toScreen(raw, x, y);

  if (millis() - lastPrint >= 120) {
    lastPrint = millis();
    Serial.print("TOUCH raw=");
    Serial.print(raw.x);
    Serial.print(",");
    Serial.print(raw.y);
    Serial.print(" z=");
    Serial.print(raw.z);
    Serial.print(" screen=");
    Serial.print(x);
    Serial.print(",");
    Serial.println(y);
  }

  if (y < boxSize) {
    selectColor(x);
  } else if (y > lcd.height() - 18) {
    lcd.fillRect(0, boxSize, lcd.width(), lcd.height() - boxSize, BLACK);
  } else {
    lcd.fillCircle(x, y, PEN_RADIUS, currentColor);
  }

  drawRawValues(raw, x, y);
  delay(20);
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    delay(10);
  }

  Serial.println();
  Serial.println("PicoDuino TFT resistive touch paint");
  lcd.printDiagnostics();
  lcd.begin();
  drawStatus("Touch paint", "8x8 raw grid correction", "Palette at top");
  delay(1500);
  drawPalette();
}

void loop() {
  handleTouchPaint();
}
