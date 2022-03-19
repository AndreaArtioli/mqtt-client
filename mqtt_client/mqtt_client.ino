
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

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}

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
  const char* hostname = String(STRINGIZE_VALUE_OF(HOSTNAME)).c_str();
#else
  const char* hostname = "ESP-MQTT-CLIENT-" + String(random(100)).c_str();
#endif

  ArduinoOTA.setHostname(hostname);
  WiFi.setHostname(hostname);
  ArduinoOTA.begin();

  nclient = MDNS.queryService("_mqtt", "_tcp");
 
  Serial.println("Ho fatto la query");

  if (nclient > 0) {
    Serial.println("Ho trovato qualcosa con la query");
    //MDNS.IP(i);

    for( int i = 0; i < nclient; i++){
      client[i].setServer(MDNS.IP(i), MQTT_PORT);
      client[i].onMessage(onMqttMessage);
      client[i].connect();
      client[i].subscribe("prova/test", 0);

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
