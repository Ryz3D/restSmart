# restSmart

**Hosts a REST Server on an ESP32 to set variables of different types. Compatible with openHAB HTTP Binding.**

Features:
* Optional automatic storage of data in EEPROM
* Automatic WiFi reconnect
* Easily controlled through a simple REST interface

Available Prop types:
* Boolean
* Float
* Int
* Color
* String

## Read/Write values

To read values you need to send an HTTP GET request to ``/get/propname``.
You can easily do this in your browser (navigate to ``http://devicename/get/propname``) or via Postman.

To write values you need to send an HTTP PUT request to ``/set/propname``.
This is done most easily via Postman, where you can put the data to be set in the "Body" settings with the type set to "raw".

## Connect to openHAB

1. Install [HTTP Binding](https://www.openhab.org/addons/bindings/http/)
2. Add new HTTP thing with baseURL ``http://devicename/`` where devicename is the wifiHostname specified in your ESP32 sketch
3. Go to the "Code" tab and set up the commandMethod ``PUT`` and your desired channels, as shown in this example (* means don't change):
```yaml
UID: *
label: *
thingTypeUID: http:url
configuration:
  baseURL: *
  refresh: 10
  commandMethod: PUT
channels:
  - id: brightness
    channelTypeUID: http:dimmer
    label: Brightness
    description: ""
    configuration:
      stateExtension: get/brightness
      commandExtension: set/brightness
  - id: power
    channelTypeUID: http:switch
    label: Power
    description: ""
    configuration:
      stateExtension: get/power
      commandExtension: set/power
      onValue: ON
      offValue: OFF
  - id: color
    channelTypeUID: http:color
    label: Color
    description: ""
    configuration:
      stateExtension: get/color
      commandExtension: set/color
      colorMode: RGB
```
This example is compatible with the "simple" example sketch.
