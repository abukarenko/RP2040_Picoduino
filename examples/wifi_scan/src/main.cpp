#include <Arduino.h>
#include <PicoDuinoLCD.h>
#include <PicoDuinoWiFiESPAT.h>

using namespace PicoDuinoColor;

PicoDuinoLCD lcd;
PicoDuinoWiFiESPAT wifi;

void printLine(uint8_t row, const String &text, uint16_t color = WHITE) {
  lcd.setTextSize(1);
  lcd.setTextColor(color, BLACK);
  lcd.setCursor(4, 28 + row * 14);
  lcd.print(text);
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  lcd.begin();
  lcd.fillScreen(BLACK);
  lcd.setTextSize(2);
  lcd.setTextColor(CYAN, BLACK);
  lcd.setCursor(8, 6);
  lcd.println("WiFi scan");

  wifi.begin(Serial1, &Serial);
  if (!wifi.testAT()) {
    printLine(0, "ESP AT not found", RED);
    return;
  }

  PicoDuinoWiFiNetwork networks[12];
  size_t count = wifi.scanNetworks(networks, 12);
  for (size_t i = 0; i < count; i++) {
    String line = networks[i].ssid;
    line += " ";
    line += networks[i].rssi;
    line += " dBm";
    printLine(i, line);
  }
  if (count == 0) {
    printLine(0, "No networks found", YELLOW);
  }
}

void loop() {
}
