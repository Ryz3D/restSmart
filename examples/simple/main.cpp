#include <Arduino.h>
#include "wifiSettings.h"
#include "restSmart.h"

RestSmart rest;

float brightness = 0.0f;
bool power = 0;
PropColor color = PropColor(255, 0, 0);

void setup() {
  Serial.begin(9600);
  EEPROM.begin(20);
  delay(500);
  Serial.print("\r\n\r\n");

  rest.wifiSSID = "";
  rest.wifiPassword = "";
  rest.wifiHostname = "esp32dev1";

  rest.props.push_back(Prop());
  rest.props.back().id = "power";
  rest.props.back().type = PropType::Bool;
  rest.props.back().valueBool = &power;

  rest.props.push_back(Prop());
  rest.props.back().id = "brightness";
  rest.props.back().type = PropType::Float;
  rest.props.back().valueFloat = &brightness;

  rest.props.push_back(Prop());
  rest.props.back().id = "color";
  rest.props.back().type = PropType::Color;
  rest.props.back().valueColor = &color;

  rest.updateProps();
}

void loop() {
  rest.loop();

  Serial.print(power ? "on" : "off");
  Serial.print("\t");
  Serial.print(brightness);
  Serial.print("%\t");
  Serial.println(color);
}
