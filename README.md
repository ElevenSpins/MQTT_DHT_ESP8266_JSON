# MQTT_DHT_ESP8266_JSON
Send DHT 22 values in a JSON string over MQTT.


## About
Mit diesem ESP8266 Sketch ist es möglich Werte (temperature,humidity) welche mit einem DHT22 erfasst werden, als JSON per MQTT zu versenden.
Dabei werden die Daten mit erfasst und in ein JSON format umgewandelt. Dieser JSON String wird dann Publishd.

### Librarys

```C++
#include "DHT.h"
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
```

### Credits
@ElevenSpins
