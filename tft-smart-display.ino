#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "arduino_secrets.h"

const char *ssid = SECRET_SSID;
const char *pass = SECRET_PASS;

WiFiMulti WiFiMulti;

const int led = LED_BUILTIN;

bool connected = false;

void setup() {
    Serial.begin(115200);

    pinMode(led, OUTPUT);
    digitalWrite(led, HIGH);

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(ssid, pass);
}

void loop() {
    if (WiFiMulti.run() == WL_CONNECTED) {
        if (!connected) {
            connected = true;
            Serial.println("WiFi connected!");
            fetch_https();
        }
    } else {
        if (connected) {
            connected = false;
            Serial.println("WiFi connection lost!");
        }
    }

    int blink_delay = connected ? 1000 : 500;
    digitalWrite(led, HIGH);
    delay(blink_delay);
    digitalWrite(led, LOW);
    delay(blink_delay);
}

void fetch_https(void) {
    HTTPClient https;
    https.setInsecure();
    Serial.print("[HTTPS] begin...\n");
    if (https.begin("https://jigsaw.w3.org/HTTP/connection.html")) {
        Serial.print("[HTTPS] GET...\n");
        int httpCode = https.GET();
        if (httpCode > 0) {
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK ||
                httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                String payload = https.getString();
                Serial.println(payload);
            }
        } else {
            Serial.printf("[HTTPS] GET... failed, error: %s\n",
                          https.errorToString(httpCode).c_str());
        }
        https.end();
    } else {
        Serial.printf("[HTTPS] Unable to connect\n");
    }
}