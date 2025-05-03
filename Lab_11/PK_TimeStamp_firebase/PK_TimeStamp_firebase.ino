#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// WiFi Credentials
const char* ssid = "Shaham";
const char* password = "abcd1234";

// Firebase Configuration
const String FIREBASE_HOST = "lab-11-firebase1-default-rtdb.firebaseio.com";
const String FIREBASE_AUTH = "8BKM5WVlZKH3vTxu2dySoUFQkBiZlHsXAhOq6HIV";
const String FIREBASE_PATH = "/DHT11_data.json";

// DHT Sensor
#define DHTPIN 4
#define DHTTYPE DHT11

// Timing
const unsigned long SEND_INTERVAL = 10000;
const unsigned long SENSOR_DELAY = 2000;

// NTP Configuration (Pakistan Standard Time UTC+5)
const long utcOffsetInSeconds = 18000;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

DHT dht(DHTPIN, DHTTYPE);
unsigned long lastSendTime = 0;
unsigned long lastReadTime = 0;

String getPakistanTime() {
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  
  // Calculate current time in Pakistan
  int currentHour = (epochTime / 3600) % 24;
  int currentMinute = (epochTime % 3600) / 60;
  
  // Convert to 12-hour format with AM/PM
  String ampm = "AM";
  if (currentHour >= 12) {
    ampm = "PM";
    if (currentHour > 12) currentHour -= 12;
  }
  if (currentHour == 0) currentHour = 12;  // Midnight case
  
  // Format as "3:27 PM"
  return String(currentHour) + ":" + 
         (currentMinute < 10 ? "0" : "") + String(currentMinute) + 
         " " + ampm;
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nESP32-S3 DHT11 Firebase Monitor");

  dht.begin();
  connectWiFi();
  timeClient.begin();
  timeClient.update();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    timeClient.begin();
  }

  if (millis() - lastReadTime >= SENSOR_DELAY) {
    float temp, hum;
    if (readDHT(&temp, &hum)) {
      if (millis() - lastSendTime >= SEND_INTERVAL) {
        String pakistanTime = getPakistanTime();
        Serial.println("Current Pakistan Time: " + pakistanTime);
        sendToFirebase(temp, hum, pakistanTime);
        lastSendTime = millis();
      }
    }
    lastReadTime = millis();
  }
}

bool readDHT(float* temp, float* humidity) {
  *temp = dht.readTemperature();
  *humidity = dht.readHumidity();

  if (isnan(*temp) || isnan(*humidity)) {
    Serial.println("DHT read failed! Retrying...");
    digitalWrite(DHTPIN, LOW);
    pinMode(DHTPIN, INPUT);
    delay(100);
    dht.begin();
    return false;
  }

  Serial.printf("DHT Read: %.1f°C, %.1f%%\n", *temp, *humidity);
  return true;
}

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.disconnect(true);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Connection Failed!");
  }
}

void sendToFirebase(float temp, float humidity, String pakistanTime) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot send - WiFi disconnected");
    return;
  }

  HTTPClient http;
  String url = "https://" + FIREBASE_HOST + FIREBASE_PATH + "?auth=" + FIREBASE_AUTH;

  String jsonPayload = "{\"temperature\":" + String(temp) +
                      ",\"humidity\":" + String(humidity) +
                      ",\"time\":\"" + pakistanTime + "\"" +
                      ",\"timestamp\":" + String(timeClient.getEpochTime()) + "}";

  Serial.println("Sending to Firebase:");
  Serial.println(jsonPayload);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(jsonPayload);

  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Firebase update successful");
  } else {
    Serial.printf("Firebase error: %d\n", httpCode);
  }

  http.end();
}