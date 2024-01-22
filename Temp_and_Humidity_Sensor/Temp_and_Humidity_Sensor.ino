#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128 // Largeur de l'écran OLED en pixels
#define SCREEN_HEIGHT 32 // Hauteur de l'écran OLED en pixels
#define OLED_RESET     -1 // Pas utilisé

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SDA_PIN 19
#define SCL_PIN 22

#define DHTPIN D5     // Broche où est connecté le DHT22
#define DHTTYPE DHT22   // DHT 22 (AM2302)

const char* ssid = "Livebox-E190";
const char* password = "AE64iEHxDuw5NpCFfE";
const char* serverUrl = "http://192.168.1.20:3000/sensor-data";

String DEVICE_ID;
const char* DEVICE_NAME = "Salon Sensor DHT22";

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    Serial.println("Tentative de connexion au Wi-Fi...");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Connecté au Wi-Fi.");
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());

    DEVICE_ID = WiFi.macAddress(); // Définition de l'ID du dispositif
    Serial.print("Device ID (MAC): ");
    Serial.println(DEVICE_ID);

    // Initialisation de l'écran OLED
    Wire.begin(SDA_PIN, SCL_PIN);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Boucle infinie en cas d'échec
    }
    display.clearDisplay();

    // Initialisation du capteur DHT
    dht.begin();
}

void loop() {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
        Serial.println(F("Failed to read from DHT sensor!"));
    } else {
        Serial.print(F("Humidity: "));
        Serial.print(humidity);
        Serial.print(F("%, Temperature: "));
        Serial.print(temperature);
        Serial.println(F("C"));

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,0);
        display.print(F("Temp: "));
        display.print(temperature);
        display.println(F("C"));
        display.setCursor(0, 10);
        display.print(F("Humid: "));
        display.print(humidity);
        display.println(F("%"));
        display.display();
    }

    delay(300000); // Attendre 5 minutes

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(client, serverUrl);

        http.addHeader("Content-Type", "application/json");

        StaticJsonDocument<300> jsonDoc;
        jsonDoc["temperature"] = temperature;
        jsonDoc["humidity"] = humidity;
        jsonDoc["deviceId"] = DEVICE_ID;
        jsonDoc["deviceName"] = DEVICE_NAME; 

        String requestBody;
        serializeJson(jsonDoc, requestBody);

        int httpResponseCode = http.POST(requestBody);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println(response);
        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    }
}
