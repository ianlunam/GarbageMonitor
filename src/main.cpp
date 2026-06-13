#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <time.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <esp_now.h>


TFT_eSPI tft = TFT_eSPI();

uint8_t backlight = BL_MIN;
uint32_t current_color = THIS_BLACK;
struct tm timeinfo;
typedef struct struct_message {
    bool on;
    bool colour;
} struct_message;

struct_message myData;

//   -DLANDFILL=0
//   -DRECYCLES=1

void setBacklight(int8_t value) {
    if (backlight != value) {
        backlight = value;
        dacWrite(TFT_BL, backlight);
    }
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&myData, incomingData, sizeof(myData));

    if (myData.colour == 0) {
        if (current_color != THIS_RED) {
            current_color = THIS_RED;
            tft.fillScreen(current_color);
        }
        Serial.println("Red");
    } else {
        if (current_color != THIS_YELLOW) {
            current_color = THIS_YELLOW;
            tft.fillScreen(current_color);
        }
        Serial.println("Yellow");
    }

    if (myData.on) {
        setBacklight(BL_MAX);
        Serial.println("Backlight ON");
    } else {
        setBacklight(BL_MIN);
        Serial.println("Backlight OFF");
    }
 
}

void setup() {
    Serial.begin(115200);

    tft.init();
    tft.invertDisplay(true);
    tft.fillScreen(current_color);
    dacWrite(TFT_BL, backlight);

    WiFi.mode(WIFI_STA);
    WiFi.softAP("MyReceiverNetwork", "password123", 2, false);
    // Print own MAC address
    String mac = WiFi.macAddress();
    Serial.print("My MAC Address: ");
    Serial.println(mac);
    Serial.print("WiFi Channel: ");
    Serial.println(WiFi.channel());

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);

    ArduinoOTA.setHostname("garbage-monitor-display");
    ArduinoOTA.begin();
}

void loop() {
    ArduinoOTA.handle();
    delay(500);
}

