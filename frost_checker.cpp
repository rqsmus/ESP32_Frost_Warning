#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

float minTemp;

const long gmtOffset_sec = 3600;        // Danmark (UTC+1)
const int daylightOffset_sec = 0;       // Sommertid = 3600, Vintertid = 0
const char* ntpServer = "pool.ntp.org";

struct tm getTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Kunne ikke hente tid!");
    }
    return timeinfo;
}

long secondsUntil18() {
    struct tm t = getTime();

    int nowSec = t.tm_hour * 3600 + t.tm_min * 60 + t.tm_sec;
    int targetSec = 18 * 3600; // KL 18:00

    if (nowSec <= targetSec) {
        return targetSec - nowSec;
    } else {
        return (24 * 3600 - nowSec) + targetSec;
    }
}



void setup() {
    Serial.begin(115200);

    // WiFi login
    WiFiManager wm;
    wm.autoConnect("ESP-Setup", "Setup1234");

    // NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(1000);

    struct tm t = getTime();
    int hour = t.tm_hour;

    if (hour >= 18 && hour <= 22) {
        // Kør temperatur check
        getTemperature();

        if (minTemp < 2.0) {
            Serial.println("Frost i nat");
        } else {
            Serial.println("Ingen frost");
        }
    }

    // Beregn næste wakeup kl 18
    long sleepSec = secondsUntil18();

    Serial.print("Deep sleep i ");
    Serial.print(sleepSec);
    Serial.println(" sekunder.");

    esp_sleep_enable_timer_wakeup((uint64_t)sleepSec * 1000000ULL);
    esp_deep_sleep_start();
}



void getTemperature() {
    WiFiClientSecure client;
    client.setInsecure(); 

    HTTPClient https;

    String url = "https://api.openweathermap.org/"; // API nøgle fjernet

    https.begin(client, url);

    int httpCode = https.GET();
    if (httpCode > 0) {
        String payload = https.getString();
        Serial.println(payload);

        DynamicJsonDocument doc(30000);
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.print("JSON fejl: ");
            Serial.println(error.c_str());
            return;
        }

        JsonArray list = doc["list"];

        minTemp = 100.0;

        for (JsonObject entry : list) {
            long timestamp = entry["dt"];           
            float temp = entry["main"]["temp"];    

            int hourOfDay = getLocalHourFromUnix(timestamp);

            if (hourOfDay >= 18 || hourOfDay <= 7) {
                if (temp < minTemp) {
                    minTemp = temp;
                }
            }
        }
    }
    https.end();
}


int getLocalHourFromUnix(long unixTime) {
    time_t rawTime = unixTime;
    struct tm timeInfo;

    localtime_r(&rawTime, &timeInfo);    
    return timeInfo.tm_hour;
}

void loop() {
}
