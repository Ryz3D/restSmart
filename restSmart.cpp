#include "restSmart.h"

PropColor::PropColor(uint8_t r, uint8_t g, uint8_t b)
{
  this->r = r;
  this->g = g;
  this->b = b;
}

uint8_t Prop::getSize()
{
  switch (type)
  {
  case PropType::Bool:
    return 1;
  case PropType::Float:
    return 4;
  case PropType::Int:
    return 4;
  case PropType::Color:
    return 3;
  case PropType::Str:
    return STRING_BUFFER;
  default:
    return 0;
  }
}

void Prop::read()
{
#if !DISABLE_EEPROM
  if (eepromSave)
  {
    switch (type)
    {
    case PropType::Bool:
      *valueBool = EEPROM.readBool(startAddress);
      break;
    case PropType::Float:
      *valueFloat = EEPROM.readFloat(startAddress);
      break;
    case PropType::Int:
      *valueInt = EEPROM.readInt(startAddress);
      break;
    case PropType::Color:
      (*valueColor).r = EEPROM.readByte(startAddress + 0);
      (*valueColor).g = EEPROM.readByte(startAddress + 1);
      (*valueColor).b = EEPROM.readByte(startAddress + 2);
      break;
    case PropType::Str:
      *valueStr = EEPROM.readString(startAddress);
      break;
    default:
      break;
    }
  }
#endif
}

void Prop::write()
{
#if !DISABLE_EEPROM
  if (eepromSave)
  {
    switch (type)
    {
    case PropType::Bool:
      EEPROM.writeBool(startAddress, *valueBool);
      break;
    case PropType::Float:
      EEPROM.writeFloat(startAddress, *valueFloat);
      break;
    case PropType::Int:
      EEPROM.writeInt(startAddress, *valueInt);
      break;
    case PropType::Color:
      EEPROM.writeByte(startAddress + 0, (*valueColor).r);
      EEPROM.writeByte(startAddress + 1, (*valueColor).g);
      EEPROM.writeByte(startAddress + 2, (*valueColor).b);
      break;
    case PropType::Str:
      EEPROM.writeString(startAddress, valueStr->length() > STRING_BUFFER ? valueStr->substring(0, STRING_BUFFER - 1) : *valueStr);
      break;
    default:
      break;
    }
    EEPROM.commit();
  }
#endif
}

void Prop::setFromBuffer()
{
#if RESTSMART_DEBUG
  Serial.print(id);
  Serial.print(" = ");
  Serial.println(rxBuffer);
#endif
  switch (type)
  {
  case PropType::Bool:
    *valueBool = rxBuffer == "ON";
    break;
  case PropType::Float:
    *valueFloat = rxBuffer.toFloat();
    break;
  case PropType::Int:
    *valueInt = rxBuffer.toInt();
    break;
  case PropType::Color:
  {
    String buffer = "";
    uint8_t channel = 0;
    rxBuffer += ",";
    for (uint16_t i = 0; i < rxBuffer.length(); i++)
    {
      if (rxBuffer[i] == ',')
      {
        switch (channel)
        {
        case 0:
          (*valueColor).r = buffer.toInt();
          break;
        case 1:
          (*valueColor).g = buffer.toInt();
          break;
        case 2:
          (*valueColor).b = buffer.toInt();
          break;
        default:
          break;
        }
        buffer = "";
        channel++;
      }
      else
      {
        buffer += rxBuffer[i];
      }
    }
    break;
  }
  case PropType::Str:
    *valueStr = rxBuffer;
    break;
  default:
    break;
  }
  write();
  rxBuffer = "";
}

String Prop::stringify()
{
  switch (type)
  {
  case PropType::Bool:
    return *valueBool ? "ON" : "OFF";
    break;
  case PropType::Float:
    return String(*valueFloat, floatPrecision);
    break;
  case PropType::Int:
    return String(*valueInt);
    break;
  case PropType::Color:
    return String((*valueColor).r) + "," + String((*valueColor).g) + "," + String((*valueColor).b);
  case PropType::Str:
    return *valueStr;
  default:
    return "UNDEF";
  }
}

void RestSmart::updateProps()
{
  uint16_t eepromCounter = eepromStart;
  for (std::vector<Prop>::iterator it = props.begin(); it != props.end(); it++)
  {
    it->startAddress = eepromCounter;
    it->read();
    eepromCounter += it->getSize();
  }
}

String RestSmart::stringify()
{
  String json = "{";
  for (std::vector<Prop>::iterator it = props.begin(); it != props.end(); it++)
  {
    json += "\"" + it->id + "\": " + it->stringify();
    if (it < props.end() - 1)
    {
      json += ",";
    }
  }
  json += "}";
  return json;
}

void RestSmart::connect()
{
  while (!WiFi.isConnected())
  {
#if RESTSMART_DEBUG
    Serial.print("WiFi connecting");
#endif
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    for (uint8_t i = 0; i < 10; i++)
    {
      if (WiFi.isConnected())
      {
        break;
      }
#if RESTSMART_DEBUG
      Serial.print('.');
#endif
      delay(500);
    }
#if RESTSMART_DEBUG
    Serial.println();
#endif
  }
  delay(100);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.setHostname(wifiHostname.c_str());
  delay(100);
  if (!MDNS.begin(wifiHostname.c_str()))
  {
#if RESTSMART_DEBUG
    Serial.println("MDNS Failed!");
#endif
    while (1)
      ;
  }
  server.begin();
  for (std::vector<Prop>::iterator it = props.begin(); it != props.end(); it++)
  {
    server.on(String("/get/" + it->id).c_str(), HTTP_ANY, [it](AsyncWebServerRequest *r)
              { r->send(200, "text/plain", it->stringify()); });
    server.on(String("/set/" + it->id).c_str(), HTTP_GET, [this, it](AsyncWebServerRequest *r)
              {
                if (r->hasParam("v"))
                {
                  it->rxBuffer = r->getParam("v")->value();
                  it->setFromBuffer();
                  r->send(200, "text/plain", "OK");
                }
                else
                {
                  r->send(400, "text/plain", "Missing \"v\" parameter for value");
                }
              });
    server.on(
        String("/set/" + it->id).c_str(), HTTP_PUT, [this, it](AsyncWebServerRequest *r)
        { r->send(200, "text/plain", "OK"); },
        [](AsyncWebServerRequest *r, String filename, size_t index, uint8_t *data, size_t len, bool f) {}, [it](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t ind, size_t total)
        {
          for (int i = 0; i < len; i++)
          {
            it->rxBuffer += (char)data[i];
          }
          if (ind + len == total)
          {
            it->setFromBuffer();
          }
        });
  }
  server.on("/get", HTTP_GET, [this](AsyncWebServerRequest *r)
            { r->send(200, "application/json", stringify()); });
  server.onNotFound([](AsyncWebServerRequest *r)
                    { r->send(404); });
  MDNS.addService("http", "tcp", RESTSMART_PORT);
}

void RestSmart::loop()
{
  if (!WiFi.isConnected())
  {
    server.end();
    MDNS.end();
    connect();
  }
}
