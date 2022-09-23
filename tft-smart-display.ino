#include <Arduino.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "arduino_secrets.h"

const char *ssid = SECRET_SSID;
const char *pass = SECRET_PASS;

WiFiMulti wifi_client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, -7 * 3600);

const int led = LED_BUILTIN;

bool connected = false;

void setup() {
    Serial.begin(115200);

    pinMode(led, OUTPUT);
    digitalWrite(led, HIGH);

    WiFi.mode(WIFI_STA);
    wifi_client.addAP(ssid, pass);
}

void loop() {
    if (wifi_client.run() == WL_CONNECTED) {
        if (!connected) {
            connected = true;
            Serial.println("WiFi connected!");

            fetch_https();
            fetch_ntp();
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

void fetch_ntp(void) {
    timeClient.begin();
    timeClient.update();

    Serial.println(timeClient.getFormattedTime());
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