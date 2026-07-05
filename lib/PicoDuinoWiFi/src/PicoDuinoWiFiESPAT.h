#pragma once

#include <Arduino.h>

struct PicoDuinoWiFiNetwork {
  String ssid;
  int rssi = 0;
  int encryption = 0;
};

class PicoDuinoWiFiESPAT {
public:
  void begin(Stream &serial, Stream *debug = nullptr);

  bool testAT(uint32_t timeoutMs = 1000);
  bool setStationMode(uint32_t timeoutMs = 1500);
  bool connect(const char *ssid, const char *password,
               uint32_t timeoutMs = 20000);
  bool connected(uint32_t timeoutMs = 2000);
  size_t scanNetworks(PicoDuinoWiFiNetwork *networks, size_t maxNetworks,
                      uint32_t timeoutMs = 12000);
  String localIP(uint32_t timeoutMs = 2000);
  String sendCommand(const String &command, uint32_t timeoutMs = 1000);

private:
  bool responseOK(const String &response) const;
  void drainInput();
  void debugPrint(const String &text);
  bool parseCwlapLine(const String &line, PicoDuinoWiFiNetwork &network) const;

  Stream *esp = nullptr;
  Stream *log = nullptr;
};
