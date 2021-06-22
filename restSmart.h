#pragma once

#define RESTSMART_DEBUG true
#define DISABLE_EEPROM false
#define STRING_BUFFER 10

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <vector>

#if !DISABLE_EEPROM
#include <EEPROM.h>
#endif

enum PropType {
  Invalid,
  Bool,
  Float,
  Int,
  Color,
  Str,
};

class PropColor {
public:
  PropColor(uint8_t r, uint8_t g, uint8_t b);

  uint8_t r, g, b;
};

class Prop {
public:
  uint8_t getSize();
  void read();
  void write();
  String stringify();

  String id;
  String rxBuffer;
  PropType type;
  bool eepromSave = true;
  uint16_t startAddress;
  uint8_t floatPrecision = 2;

  bool *valueBool;
  float *valueFloat;
  int *valueInt;
  PropColor *valueColor;
  String *valueStr;
};

class RestSmart {
public:
  void updateProps();
  void loop();

  AsyncWebServer server = AsyncWebServer(WIFI_PORT);
  std::vector<Prop> props;
  uint16_t eepromStart;
  String wifiSSID;
  String wifiPassword;
  String wifiHostname;
  uint32_t wifiPort = 80;
private:
  String stringify();
  void connect();
};
