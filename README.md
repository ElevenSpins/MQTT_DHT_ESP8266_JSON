# MQTT_DHT_ESP8266_JSON
Send DHT 22 values in a JSON string over MQTT.


## About
Mit diesem ESP8266 Sketch ist es möglich Werte (temperature,humidity) welche mit einem DHT22 erfasst werden, als JSON per MQTT zu versenden.
Dabei werden die Daten mit erfasst und in ein JSON format umgewandelt. Dieser JSON String wird dann Publishd.

**Features:**

 * Support subscribing, publishing, authentication, will messages.
 * Support multiple publish Topics.
 * Support WIFI reconnect & MQTT reconnect
 * Easy to setup and use
 
 ***Prerequire:***

- Arduino IDE
- Driver (CH341SER, CP210x)
- ESP8266
- DHT22
 
# Default Configuration

### Librarys

```C++
#include "DHT.h"
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
```

### Licences
- PubSubClient - [Nick O'Leary - knolleary](https://github.com/knolleary/pubsubclient)
- ESP8266WiFi - (https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi)
- ArduinoJSON - [Benoît Blanchon - bblanchon] (https://github.com/bblanchon/ArduinoJson)

### Credits
@ElevenSpins
