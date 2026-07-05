#include "PicoDuinoTouch.h"

namespace {
// Touch_shield_new style wiring:
// XP=D6, XM=A2, YP=A1, YM=D7.
// Measured PicoDuino mapping makes this XP=GPIO8, XM=GPIO28, YP=GPIO27, YM=GPIO9.
constexpr uint8_t TOUCH_XP = 8;
constexpr uint8_t TOUCH_XM = 28;
constexpr uint8_t TOUCH_YP = 27;
constexpr uint8_t TOUCH_YM = 9;

constexpr int TOUCH_IDLE_X = 1023;
constexpr int TOUCH_IDLE_Y = 5;
constexpr int TOUCH_MIN_Z = 120;

struct TouchCalPoint {
  int16_t sx;
  int16_t sy;
  int16_t rx;
  int16_t ry;
};

const TouchCalPoint TOUCH_GRID[] = {
    {20, 15, 894, 412},   {60, 15, 902, 390},   {100, 15, 916, 350},
    {140, 15, 940, 298},  {180, 15, 939, 263},  {220, 15, 942, 224},
    {260, 15, 947, 174},  {300, 15, 947, 131},  {20, 45, 865, 422},
    {60, 45, 876, 380},   {100, 45, 888, 359},  {140, 45, 903, 316},
    {180, 45, 906, 278},  {220, 45, 904, 250},  {260, 45, 913, 196},
    {300, 45, 923, 138},  {20, 75, 849, 405},   {60, 75, 838, 411},
    {100, 75, 849, 382},  {140, 75, 869, 337},  {180, 75, 878, 293},
    {220, 75, 882, 255},  {260, 75, 892, 199},  {300, 75, 899, 139},
    {20, 105, 808, 427},  {60, 105, 802, 425},  {100, 105, 823, 388},
    {140, 105, 844, 338}, {180, 105, 857, 296}, {220, 105, 858, 263},
    {260, 105, 866, 206}, {300, 105, 882, 134}, {20, 135, 764, 455},
    {60, 135, 772, 440},  {100, 135, 779, 422}, {140, 135, 802, 377},
    {180, 135, 819, 327}, {220, 135, 822, 288}, {260, 135, 834, 228},
    {300, 135, 857, 146}, {20, 165, 733, 470},  {60, 165, 722, 478},
    {100, 165, 745, 443}, {140, 165, 774, 392}, {180, 165, 794, 342},
    {220, 165, 800, 299}, {260, 165, 807, 242}, {300, 165, 822, 166},
    {20, 195, 696, 463},  {60, 195, 715, 468},  {100, 195, 751, 418},
    {140, 195, 760, 390}, {180, 195, 767, 357}, {220, 195, 783, 298},
    {260, 195, 789, 247}, {300, 195, 805, 167}, {20, 225, 654, 524},
    {60, 225, 691, 481},  {100, 225, 742, 414}, {140, 225, 767, 366},
    {180, 225, 768, 332}, {220, 225, 771, 299}, {260, 225, 778, 246},
    {300, 225, 784, 179},
};
}  // namespace

PicoDuinoTouch::PicoDuinoTouch(PicoDuinoLCD &display) : lcd(display) {}

PicoDuinoTouchPoint PicoDuinoTouch::readRaw() {
  PicoDuinoTouchPoint p;

  pinMode(TOUCH_YP, INPUT_PULLDOWN);
  pinMode(TOUCH_YM, INPUT);
  pinMode(TOUCH_XP, OUTPUT);
  pinMode(TOUCH_XM, OUTPUT);
  digitalWrite(TOUCH_XP, HIGH);
  digitalWrite(TOUCH_XM, LOW);
  delayMicroseconds(80);
  p.x = readAnalogAverage(TOUCH_YP);

  pinMode(TOUCH_XM, INPUT_PULLDOWN);
  pinMode(TOUCH_XP, INPUT);
  pinMode(TOUCH_YP, OUTPUT);
  pinMode(TOUCH_YM, OUTPUT);
  digitalWrite(TOUCH_YP, HIGH);
  digitalWrite(TOUCH_YM, LOW);
  delayMicroseconds(80);
  p.y = readAnalogAverage(TOUCH_XM);

  p.z = abs(p.x - TOUCH_IDLE_X) + abs(p.y - TOUCH_IDLE_Y);
  p.touched = p.z > TOUCH_MIN_Z;
  lcd.restoreBusPins();
  return p;
}

PicoDuinoScreenPoint PicoDuinoTouch::readScreen() {
  PicoDuinoTouchPoint raw = readRaw();
  PicoDuinoScreenPoint screen;
  screen.touched = raw.touched;
  if (raw.touched) {
    toScreen(raw, screen.x, screen.y);
  }
  return screen;
}

void PicoDuinoTouch::toScreen(const PicoDuinoTouchPoint &raw, int16_t &x,
                              int16_t &y) {
  static constexpr uint8_t K = 6;
  uint32_t bestD[K];
  uint8_t bestI[K];

  for (uint8_t i = 0; i < K; i++) {
    bestD[i] = 0xFFFFFFFFUL;
    bestI[i] = 0;
  }

  for (uint8_t i = 0; i < sizeof(TOUCH_GRID) / sizeof(TOUCH_GRID[0]); i++) {
    long dx = static_cast<long>(raw.x) - TOUCH_GRID[i].rx;
    long dy = static_cast<long>(raw.y) - TOUCH_GRID[i].ry;
    uint32_t d = static_cast<uint32_t>(dx * dx + dy * dy);

    for (uint8_t slot = 0; slot < K; slot++) {
      if (d < bestD[slot]) {
        for (int8_t move = K - 1; move > slot; move--) {
          bestD[move] = bestD[move - 1];
          bestI[move] = bestI[move - 1];
        }
        bestD[slot] = d;
        bestI[slot] = i;
        break;
      }
    }
  }

  double sumW = 0.0;
  double sumX = 0.0;
  double sumY = 0.0;
  for (uint8_t i = 0; i < K; i++) {
    const TouchCalPoint &c = TOUCH_GRID[bestI[i]];
    double w = 1.0 / (static_cast<double>(bestD[i]) + 1.0);
    sumW += w;
    sumX += c.sx * w;
    sumY += c.sy * w;
  }

  x = clampInt(static_cast<int>(sumX / sumW + 0.5), 0, lcd.width() - 1);
  y = clampInt(static_cast<int>(sumY / sumW + 0.5), 0, lcd.height() - 1);
}

int PicoDuinoTouch::readAnalogAverage(uint8_t pin) {
  long sum = 0;
  for (uint8_t i = 0; i < 8; i++) {
    sum += analogRead(pin);
    delayMicroseconds(80);
  }
  return sum / 8;
}

int PicoDuinoTouch::clampInt(int value, int lo, int hi) {
  if (value < lo) return lo;
  if (value > hi) return hi;
  return value;
}
