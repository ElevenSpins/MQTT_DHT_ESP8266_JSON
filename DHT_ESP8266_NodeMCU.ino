

// Created by Marian - ElevenSpins
// zur freien Nutzung bereitgestellt 



#include "DHT.h"
#include <PubSubClient.h> // MQTT Bibliothek
#include <ESP8266WiFi.h>
#include <ArduinoJson.h> // JSON library

#define DHTPIN D1     // Digital Pin 2
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* TOPIC_TEMPERATUR = "TOPIC";
const char* TOPIC_LUFTFEUCHTIGKEIT = "TOPIC";
const char* TOPIC_HITZEINDEX = "TOPIC";  // gefühlte Temperatur
const char* broker = "MQTT-URL";
const char* device = "ESP_8266";
const char* location = "---"; 

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

class Messwert{
public:
  Messwert()
  :temperatur(0),
  luftfeuchtigkeit(0),
  hitzeIndex(0)
  {};

  bool operator==(Messwert& rhs)const {
    if (rhs.temperatur == temperatur && rhs.luftfeuchtigkeit == luftfeuchtigkeit){
      return true;
    } else {
      return false;
    }
  }
  
  float temperatur;
  float luftfeuchtigkeit;
  float hitzeIndex;
};

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
  
void setup() {
  Serial.begin(115200);
  setup_wifi();
  Serial.println("DHT22 Procucer Init ...");
  dht.begin();

  // Broker erreichbar über ip-Adresse = server, port = 1883
  client.setServer(broker, 1883); // Adresse des MQTT-Brokers
  // client.setCallback(callback);   // Handler für eingehende Nachrichten
  
  // Kleine Pause
  delay(1500);

}

void loop() {
 
  
  // MQTT => Solange probieren bis es klappt:
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Messwerte abfragen und rausblasen.
  if (timerintervall(60 * 10)) {  // alle 10 Minuten eine Messung ausgeben
      bool mitMesswertvergleich = false;
      messung(mitMesswertvergleich);
  }
  else { // ... sonst bei Messwertänderung alle 2 Sekunden 
    if (timerintervall(2)) {
      bool mitMesswertvergleich = true;
      messung(mitMesswertvergleich);
    }
  }
}



void messung(bool mitMesswertvergleich){

    // Messwert einlesen
    Messwert messwert = leseMesswert();
    // Messwert ausgeben
    publishMesswert(messwert, mitMesswertvergleich);
}


/*
 *  leseMesswert liest die Messwerte vom DHT22 Sensor ein 
 */
Messwert leseMesswert(){

  Messwert messung;

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Fehler beim lesen vom  DHT Sensor!");
    return messung;
  }

  messung.temperatur = t - 1;  // Des Glomp liefert einen zu hohen Wert. Laut Thermometer 1 Grad zu viel :=(
  messung.luftfeuchtigkeit = h;

  return messung;
}
  

/*
 *  Gibt die Messwerte auf der seriellen Schnittstelle und über MQTT wieder aus
 *  Bei mitMesswertvergleich wird nur ausgegeben wenn sich die neuen Messwerte von den alten unterscheiden
 */
void publishMesswert(const Messwert& messwert, bool mitMesswertvergleich ){

    static Messwert alterMesswert;
    
    bool publish = true;
    if (mitMesswertvergleich) {
      // 1. Vergleich => hat sich dre Wert geändert ?
      if (messwert == alterMesswert) {
        publish = false;
      } 
      // 2. Vergleich => In der Blacklist sind die letzten Messwerte Verhindert das Zappeln des letzten Digits
      if (inBlacklist(messwert)) {
        publish = false;
      }
    }
    
    if (publish) {

      alterMesswert = messwert;
    
      if (client.connected()) {
   
      const int capacity = JSON_OBJECT_SIZE(4);
      StaticJsonDocument<capacity> jsonDoc;
      
   //   JsonObject root = jsonDoc.as<JsonObject>();
      jsonDoc["value"] = messwert.temperatur;
      jsonDoc["unit"] = "°C";
      jsonDoc["device"] = device;
      jsonDoc["location"] = location;
//      root["recordingdate"] = "";
      String sTemperatur; 
      serializeJson(jsonDoc, sTemperatur);   
      client.publish(TOPIC_TEMPERATUR,sTemperatur.c_str());
      Serial.println("Temperatur: ");
      serializeJson(jsonDoc,Serial);        
      Serial.println();
      jsonDoc.clear();
      
      jsonDoc["value"] = messwert.luftfeuchtigkeit;
      jsonDoc["unit"] = "%";
      jsonDoc["device"] = device;
      jsonDoc["location"] = location;
//      root["recordingdate"] = "";
      String sLuftfeuchtigkeit;
      serializeJson(jsonDoc, sLuftfeuchtigkeit);   
      client.publish(TOPIC_LUFTFEUCHTIGKEIT,sLuftfeuchtigkeit.c_str());
      Serial.println("Luftfeuchtigkeit: ");
      serializeJson(jsonDoc, Serial);        
      Serial.println();
      jsonDoc.clear();
      // Compute heat index in Celsius (isFahreheit = false)
      float hic = dht.computeHeatIndex(messwert.temperatur, messwert.luftfeuchtigkeit, false);
      jsonDoc["value"] = hic;
      jsonDoc["unit"] = "°C";
//      root["recordingdate"] = "";
      String shic;
      serializeJson(jsonDoc,shic);   
      client.publish(TOPIC_HITZEINDEX,shic.c_str());
      Serial.println("Hitze Index (Gefühlte Temperatur): ");
      serializeJson(jsonDoc,Serial);        
      Serial.println();
      jsonDoc.clear();

      }
    }
  }

  /*
   *  liefert true wenn die angegebene Zeit verstrichen ist
   */
  bool timerintervall(unsigned int sekunden){

    static unsigned long vergangeneMillis = 0;
    
    bool ret = false;
    unsigned long aktuelleMillis = millis();
    unsigned long interval = sekunden * 1000;
    if (aktuelleMillis - vergangeneMillis >= interval) {
      vergangeneMillis = aktuelleMillis;
      Serial.print("vergangene Millisekunden: ");
      Serial.println(vergangeneMillis);
      ret = true;
    }
    return ret;
  }
  /*
   *  In der Blacklist werden die letzten Messwerte gespeichert
   *  Soll das Zappeln des letzten Digits verhindern.
   */
  bool inBlacklist(Messwert messwert) {

    bool inBlacklistGefunden = false;
    
    static const int ARRAYGROESSE = 6;
    static Messwert blacklist[ARRAYGROESSE];

    int i = 0;
    // in blacklist suchen ...
    for(i = 0; i < ARRAYGROESSE; i++ ) {
      if (messwert == blacklist[i]){
        inBlacklistGefunden = true;
        break;
      }
    }

    // ... nicht in blacklist gefunden. => Daher in blacklist aufnehmen 
    if (!inBlacklistGefunden){
      // Alle rutschen einen Stuhl weiter. Der letzte wird überschrieben 
      for(i = ARRAYGROESSE - 1; i > 0; i-- ) {
        blacklist[i] = blacklist[i-1];
      }
       // Der Neue kommt auf den ersten Platz
      blacklist[0] = messwert;
    }
  
    return inBlacklistGefunden;  
  }



// *************************************************** MQTT **************************************************************************

/*
Die nachfolgende Methode wird aufgerufen, sobald eine Nachricht für das angegebene Topic eintrifft:
*/
//void callback(char* topic, byte* payload, unsigned int length) {
//  Serial.print("Nachricht eingetroffen [");
//  Serial.print(topic);
//  Serial.print("]: ");
//  for (int i=0;i<length;i++) {
//    Serial.print((char)payload[i]);
//  }
//  Serial.println();
//}

void reconnect() {
  // Solange wiederholen bis Verbindung wiederhergestellt ist

  while (!client.connected()) {
    Serial.print("Versuch des MQTT Verbindungsaufbaus...");
    //Verbindungsversuch:
    if (client.connect("arduinoClient_Werner")) {
      delay(200);
      Serial.println("Erfolgreich verbunden!");
      // Nun versendet der Arduino eine Nachricht in outTopic ...
      //   client.publish("Arduino/Messwert","Werners Arduino nach Hause telefonieren");
      // und meldet sich bei inTopic für eingehende Nachrichten an:
      //      client.subscribe("inTopic");
    } else { // Im Fehlerfall => Fehlermeldung und neuer Versuch
      Serial.print("Fehler, rc=");
      Serial.print(client.state());
      Serial.println(" Nächster Versuch in 5 Sekunden");
      // 5 Sekunden Pause vor dem nächsten Versuch
      delay(5000);
    }
  }
}
