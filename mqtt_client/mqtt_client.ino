
#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

#include <ArduinoOTA.h>
#include <AsyncMqttClient.h>

#define MAX_CLIENT 10
int nclient = 0;
AsyncMqttClient client[MAX_CLIENT];
const short MQTT_PORT = 1883;

void setup() {
#ifdef PORT
  ArduinoOTA.setPort(PORT);
#endif

  Serial.begin(115200);
  while (!Serial);
  WiFi.mode(WIFI_STA);

  WiFi.begin(STRINGIZE_VALUE_OF(NETWORK_NAME), STRINGIZE_VALUE_OF(NETWORK_PASSWORD));

  while (!Serial);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

#ifdef OTA_PASSWORD
  ArduinoOTA.setPassword(STRINGIZE_VALUE_OF(PASSWORD));
#endif

#ifdef HOSTNAME
  ArduinoOTA.setHostname(STRINGIZE_VALUE_OF(HOSTNAME));
  WiFi.setHostname(STRINGIZE_VALUE_OF(HOSTNAME));
#else
  auto val = random(100);
  ArduinoOTA.setHostname(("ESP-MQTT-CLIENT-" + String(val)).c_str());
  WiFi.setHostname(("ESP-MQTT-CLIENT-" + String(val)).c_str());
#endif

  ArduinoOTA.begin();

  nclient = MDNS.queryService("_mqtt", "_tcp");
 
  Serial.println("Ho fatto la query");

  if (nclient > 0) {
    Serial.println("Ho trovato qualcosa con la query");
    //MDNS.IP(i);

    for( int i = 0; i < nclient; i++){
      client[i].setServer(MDNS.IP(i), MQTT_PORT);
      client[i].setClientId(STRINGIZE_VALUE_OF(HOSTNAME));
      client[i].connect();

      Serial.println("Ho trovato: " + MDNS.hostname(i));
    }
  } else {

    ESP.restart();
  }

  Serial.println("Ho finito il setup");
}

void loop() {
  ArduinoOTA.handle();

  Serial.println("Sono nel loop");

  Serial.println("Sono nel loop, sto per entrare nel for");
  for (int i = 0; i < nclient; ++i) {
    
    auto rnd = random(100);
    Serial.println("Client " + String(i) + ": sto provando a mandare qualcosa: " + String(rnd));
    client[i].connect();
    auto timestart = millis();
    while(!client[i].connected()){
      if (millis() - timestart > 1000)
        break;
    }
    client[i].publish("prova/test", 0, false, String(rnd).c_str());

  }
  
  delay(500);
}
