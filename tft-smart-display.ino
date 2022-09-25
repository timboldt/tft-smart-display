#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiNTP.h>

#include "arduino_secrets.h"

const char *ssid = SECRET_SSID;
const char *pass = SECRET_PASS;

WiFiMulti wifi_client;

void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    toggle_led();

    // Connect to WiFi.
    Serial.println("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    wifi_client.addAP(ssid, pass);
    wifi_client.run();
    toggle_led();

    // Start time service client.
    Serial.println("Fetching current time...");
    NTP.begin("pool.ntp.org", "time.nist.gov");
    NTP.waitSet();
    toggle_led();
}

void loop() {
    static bool fetched = false;
    if (!fetched) {
        fetched = true;
        Serial.println("Fetching HTTPS contents...");
        fetch_https();
    }

    delay(1000);
    toggle_led();

    time_t now = time(nullptr) - 7 * 3600;
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.print(asctime(&timeinfo));
}

void toggle_led(void) {
    static bool on = false;
    on = !on;
    digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
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