#pragma once

#ifndef RESTSMART_PORT
#define RESTSMART_PORT 80
#endif

#ifndef RESTSMART_DEBUG
#define RESTSMART_DEBUG false
#endif

#ifndef DISABLE_EEPROM
#define DISABLE_EEPROM false
#endif

#ifndef STRING_BUFFER
#define STRING_BUFFER 10
#endif

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <vector>

#if !DISABLE_EEPROM
#include <EEPROM.h>
#endif

enum PropType
{
  Invalid,
  Bool,
  Float,
  Int,
  Color,
  Str,
};

class PropColor
{
public:
  PropColor(uint8_t r, uint8_t g, uint8_t b);

  uint8_t r, g, b;
};

class Prop
{
public:
  uint8_t getSize();
  void read();
  void write();
  void setFromBuffer();
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

class RestSmart
{
public:
  void updateProps();
  void loop();

  AsyncWebServer server = AsyncWebServer(80);
  std::vector<Prop> props;
  uint16_t eepromStart;
  String wifiSSID;
  String wifiPassword;
  String wifiHostname;

private:
  String stringify();
  void connect();
};
