#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <time.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

TFT_eSPI tft = TFT_eSPI();
WiFiClient espClient;
PubSubClient pubSubClient(espClient);

uint8_t backlight = BL_MIN;
uint32_t current_color = THIS_BLACK;
struct tm timeinfo;

void callback(char *topic, byte *payload, unsigned int length) {
    String sTopic(topic);
    String sPayload(payload, length);

    if (sTopic == BIN_TOPIC) {
        if (sPayload == RECYCLES) {
            Serial.println("New state is Recycles");
            if (current_color != THIS_YELLOW) {
                current_color = THIS_YELLOW;
                tft.fillScreen(current_color);
            }
            return;
        } else if (sPayload == LANDFILL){
            Serial.println("New state is Landfill");
            if (current_color != THIS_RED) {
                current_color = THIS_RED;
                tft.fillScreen(current_color);
            }
            return;
        }
    }
    Serial.println("Unknown topic: " + sTopic + " with payload: " + sPayload); 
}

void pubSubSetup() {
  pubSubClient.setServer(MQTT_BROKER, MQTT_PORT);
  pubSubClient.setCallback(callback);
  while (!pubSubClient.connected()) {
      String client_id = "esp32-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
      if (pubSubClient.connect(client_id.c_str())) { 
          Serial.println("EMQX MQTT broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(pubSubClient.state());
          delay(2000);
      }
  }
  pubSubClient.subscribe(BIN_TOPIC);
}

void setupWiFi() {
  Serial.println("Connecting to WiFi...");
  
  WiFi.begin(WIFI_SSID, WIFI_PWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  configTzTime(TIMEZONE, "nz.pool.ntp.org");
}

void setBacklight(int8_t value) {
    if (backlight != value) {
        backlight = value;
        dacWrite(TFT_BL, backlight);
    }
}

void setupBacklight() {
    getLocalTime(&timeinfo);
    if (timeinfo.tm_wday == WEEKDAY) {
        if (timeinfo.tm_hour >= START_HOUR && timeinfo.tm_hour < END_HOUR) {
            setBacklight(BL_MAX);
        } else {
            setBacklight(BL_MIN);
        }
    } else {
        setBacklight(BL_MIN);
    }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting to WiFi...");
  setupWiFi();

  tft.init();
  tft.invertDisplay(true);
  tft.fillScreen(current_color);
  dacWrite(TFT_BL, backlight);

  pubSubSetup();

  ArduinoOTA.setHostname("garbage-monitor");
  ArduinoOTA.begin();
}

void loop() {
  pubSubClient.loop();
  ArduinoOTA.handle();

  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting WiFi");
      setupWiFi();
  }
  if (!pubSubClient.connected() && WiFi.status() == WL_CONNECTED) {
      Serial.println("Reconnecting MQTT");
      pubSubSetup();
  }

  setupBacklight();

  delay(500);
}

