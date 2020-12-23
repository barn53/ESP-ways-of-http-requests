#include <Arduino.h> // Graphics and font library for ST7735 driver chip

#include "secrets.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <StreamUtils.h>

void wifiSleep()
{
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);
}
void wifiWake()
{
    WiFi.forceSleepWake();
    delay(1);
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
}

void setupWiFi()
{
    wifiWake();

    WiFi.begin(ssid, password);
    WiFi.hostname(hostname);

    Serial.print("Connecting ");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }

    Serial.printf("\nConnected\n IP address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf(" Hostname: %s\n", WiFi.hostname().c_str());
}

void method1()
{
    Serial.println("\nMethod 1: complete http response as string - ineffective");

    HTTPClient http;

    http.begin("http://worldtimeapi.org/api/timezone/Europe/Berlin");
    int httpCode = http.GET();
    if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
            Serial.println(http.getString());
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
}

void method2()
{
    Serial.println("\nMethd 2: Use HTTP stream for ArduinoJSON");

    HTTPClient http;
    http.useHTTP10(true); // stream is only available with HTTP1.0 (no chunked transfer)
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

    http.begin("http://worldtimeapi.org/api/timezone/Europe/Berlin");
    int httpCode = http.GET();
    if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
            DynamicJsonDocument doc(768);
            deserializeJson(doc, http.getStream());
            Serial.println(doc["datetime"].as<const char*>());
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
}

void method3()
{
    Serial.println("\nMethod 3: With WifiClient stream ArduinoJSON");

    WiFiClient client;
    HTTPClient http;

    http.begin(client, "http://worldtimeapi.org", 80, "/api/timezone/Europe/Berlin", false);
    int httpCode = http.GET();
    if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
            DynamicJsonDocument doc(768);
            deserializeJson(doc, client);
            Serial.println(doc["datetime"].as<const char*>());
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
}

void method4()
{
    Serial.println("\nMethod 4: HTTPS with WifiClientSecure (insecure) stream ArduinoJSON");

    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();

    http.begin(client, "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=eur&include_24hr_change=true");
    http.useHTTP10(true);
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
            ReadLoggingStream loggingStream(client, Serial);
            DynamicJsonDocument doc(96);
            deserializeJson(doc, loggingStream);
            Serial.printf("\n\nbitcoin eur: %f, 24h: %.2f%%",
                doc["bitcoin"]["eur"].as<double>(),
                doc["bitcoin"]["eur_24h_change"].as<double>());
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
}

void setup(void)
{
    Serial.begin(115200);

    setupWiFi();

    method1();
    method2();
    method3();
    method4();

    wifiSleep();
}

void loop()
{
}
