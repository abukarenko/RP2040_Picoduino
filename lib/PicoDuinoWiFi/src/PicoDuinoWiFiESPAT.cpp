#include "PicoDuinoWiFiESPAT.h"

void PicoDuinoWiFiESPAT::begin(Stream &serial, Stream *debug) {
  esp = &serial;
  log = debug;
  drainInput();
}

bool PicoDuinoWiFiESPAT::testAT(uint32_t timeoutMs) {
  return responseOK(sendCommand("AT", timeoutMs));
}

bool PicoDuinoWiFiESPAT::setStationMode(uint32_t timeoutMs) {
  return responseOK(sendCommand("AT+CWMODE=1", timeoutMs));
}

bool PicoDuinoWiFiESPAT::connect(const char *ssid, const char *password,
                                 uint32_t timeoutMs) {
  if (!setStationMode()) {
    return false;
  }

  String command = "AT+CWJAP=\"";
  command += ssid;
  command += "\",\"";
  command += password;
  command += "\"";
  String response = sendCommand(command, timeoutMs);
  return responseOK(response) || response.indexOf("WIFI CONNECTED") >= 0;
}

bool PicoDuinoWiFiESPAT::connected(uint32_t timeoutMs) {
  String response = sendCommand("AT+CWJAP?", timeoutMs);
  return response.indexOf("+CWJAP:") >= 0 && response.indexOf("No AP") < 0;
}

size_t PicoDuinoWiFiESPAT::scanNetworks(PicoDuinoWiFiNetwork *networks,
                                        size_t maxNetworks,
                                        uint32_t timeoutMs) {
  if (!networks || maxNetworks == 0) {
    return 0;
  }

  String response = sendCommand("AT+CWLAP", timeoutMs);
  size_t count = 0;
  int start = 0;
  while (count < maxNetworks) {
    int end = response.indexOf('\n', start);
    if (end < 0) {
      end = response.length();
    }

    String line = response.substring(start, end);
    line.trim();
    if (line.startsWith("+CWLAP:") && parseCwlapLine(line, networks[count])) {
      count++;
    }

    if (static_cast<unsigned int>(end) >= response.length()) {
      break;
    }
    start = end + 1;
  }
  return count;
}

String PicoDuinoWiFiESPAT::localIP(uint32_t timeoutMs) {
  String response = sendCommand("AT+CIFSR", timeoutMs);
  int tag = response.indexOf("+CIFSR:STAIP,\"");
  if (tag < 0) {
    return "";
  }
  int first = response.indexOf('"', tag);
  int second = response.indexOf('"', first + 1);
  if (first < 0 || second < 0) {
    return "";
  }
  return response.substring(first + 1, second);
}

String PicoDuinoWiFiESPAT::sendCommand(const String &command,
                                       uint32_t timeoutMs) {
  if (!esp) {
    return "";
  }

  drainInput();
  debugPrint(">> " + command + "\n");
  esp->print(command);
  esp->print("\r\n");

  String response;
  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    while (esp->available()) {
      char c = static_cast<char>(esp->read());
      response += c;
      start = millis();
    }

    if (response.indexOf("\r\nOK\r\n") >= 0 ||
        response.indexOf("\r\nERROR\r\n") >= 0 ||
        response.indexOf("FAIL") >= 0) {
      break;
    }
    delay(2);
  }

  debugPrint(response);
  return response;
}

bool PicoDuinoWiFiESPAT::responseOK(const String &response) const {
  return response.indexOf("\r\nOK\r\n") >= 0 || response.endsWith("OK\r\n") ||
         response == "OK";
}

void PicoDuinoWiFiESPAT::drainInput() {
  if (!esp) {
    return;
  }
  while (esp->available()) {
    esp->read();
  }
}

void PicoDuinoWiFiESPAT::debugPrint(const String &text) {
  if (log) {
    log->print(text);
  }
}

bool PicoDuinoWiFiESPAT::parseCwlapLine(
    const String &line, PicoDuinoWiFiNetwork &network) const {
  int open = line.indexOf('(');
  int q1 = line.indexOf('"', open);
  int q2 = line.indexOf('"', q1 + 1);
  if (open < 0 || q1 < 0 || q2 < 0) {
    return false;
  }

  int firstComma = line.indexOf(',', open);
  int rssiComma = line.indexOf(',', q2 + 1);
  if (firstComma < 0 || rssiComma < 0) {
    return false;
  }

  network.encryption = line.substring(open + 1, firstComma).toInt();
  network.ssid = line.substring(q1 + 1, q2);
  network.rssi = line.substring(q2 + 2, rssiComma).toInt();
  return true;
}
