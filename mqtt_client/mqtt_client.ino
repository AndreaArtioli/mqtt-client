
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
#include <TinyMqtt.h>

static MqttClient* client;
int N_CLIENT = 0;

void setup() {
#ifdef PORT
  ArduinoOTA.setPort(PORT);
#endif

  WiFi.mode(WIFI_STA);

  //#ifdef NETWORK_NAME and NETWORK_PASSWORD
  WiFi.begin(STRINGIZE_VALUE_OF(NETWORK_NAME), STRINGIZE_VALUE_OF(NETWORK_PASSWORD));

  Serial.begin(9600);
  while (!Serial);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

#ifdef PASSWORD
  // If you want authentication, uncomment this line
  ArduinoOTA.setPassword(STRINGIZE_VALUE_OF(PASSWORD));
#endif

#ifdef HOSTNAME
  ArduinoOTA.setHostname(STRINGIZE_VALUE_OF(HOSTNAME));
  WiFi.setHostname(STRINGIZE_VALUE_OF(HOSTNAME));
  MDNS.begin(STRINGIZE_VALUE_OF(HOSTNAME));
#else
  ArduinoOTA.setHostname("ESP_OTA");
  WiFi.setHostname("ESP_OTA");
  MDNS.begin("ESP_OTA");
#endif

  ArduinoOTA.begin();
  
  int n = MDNS.queryService("mqtt", "tcp");

  if (n > 0) {
    N_CLIENT = n;
    client = (MqttClient*) malloc(sizeof(MqttClient) * n);
    for (int i = 0; i < N_CLIENT; ++i) {
      //MDNS.IP(i);
      client[i].connect(MDNS.hostname(i).c_str(), (uint16_t)1883);  
    }

  } else {

    ESP.restart();
  }

  

}

void loop() {
  
  for (int i = 0; i < N_CLIENT; ++i) {
      client[i].loop(); 
  }
  
  ArduinoOTA.handle();
  delay(500);
  auto rnd=random(100);
  for (int i = 0; i < N_CLIENT; ++i) {
      client[i].publish("prova/test", String(rnd));
      
  }
}
