#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiNTP.h>

#include "arduino_secrets.h"

const char *ssid = SECRET_SSID;
const char *pass = SECRET_PASS;

WiFiMulti wifi_client;

const int SDCARD_CS = 7;
const int TOUCHSCREEN_CS = 8;
const int TFT_CS = 9;
const int TFT_DC = 10;
const int SPI_CLK = 18;
const int SPI_MO = 19;
const int SPI_MI = 20;

Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS);
Adafruit_STMPE610 ts = Adafruit_STMPE610(TOUCHSCREEN_CS, &SPI);

void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    toggle_led();

    SPI.setSCK(SPI_CLK);
    SPI.setTX(SPI_MO);
    SPI.setRX(SPI_MI);
    //SPI.begin(false);

    SD.begin(SDCARD_CS, SPI);
    ts.begin();
    tft.begin();

    uint8_t x = tft.readcommand8(ILI9341_RDMODE);
    Serial.print("Display Power Mode: 0x");
    Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDMADCTL);
    Serial.print("MADCTL Mode: 0x");
    Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDPIXFMT);
    Serial.print("Pixel Format: 0x");
    Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDIMGFMT);
    Serial.print("Image Format: 0x");
    Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDSELFDIAG);
    Serial.print("Self Diagnostic: 0x");
    Serial.println(x, HEX);

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
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(0, 0);
        tft.setTextColor(ILI9341_GREEN);
        tft.setTextSize(1);
        tft.println("Hello world.");
        yield();

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

    Serial.println(ts.touched());
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