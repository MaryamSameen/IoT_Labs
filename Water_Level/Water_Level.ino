#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "model_data.h"
#include "time.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include <HTTPClient.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TRIG_PIN 5
#define ECHO_PIN 18
#define BUZZER_PIN 13
#define PUMP 15
#define BUTTON_PIN 12

const float tankHeight = 17.0;
const float minDistance = 1.5;
bool buzzerEnabled = true;
unsigned long lastBuzzerTime = 0;
const unsigned long buzzerInterval = 2000;

#define TENSOR_ARENA_SIZE 8 * 1024
uint8_t tensor_arena[TENSOR_ARENA_SIZE];
tflite::MicroInterpreter* interpreter;
TfLiteTensor* input;
TfLiteTensor* output;

float mean_hour = 11.5, scale_hour = 5.0;
float mean_day = 3.0, scale_day = 2.0;

const char* ssid = "okokok";
const char* password = "09000000";
const char* influxdb_url = "http://192.168.191.78:8086/api/v2/write?org=IoT_NTU&bucket=water_level_bucket&precision=s";
const char* influxdb_token = "FPpTEORxYMQWjwc-0eNAz1ESQ18TxjanfxUaLUWvwFP6FhBJ0SmIk-O8hkPp7OLO9nAJu7-yv8DQwru429qI3g==";

const char* ntpServer = "pk.pool.ntp.org";
const long gmtOffset_sec = 5 * 3600;
const int daylightOffset_sec = 0;

unsigned long lastInfluxSend = 0;
const unsigned long influxInterval = 5000;
float lastValidLevel = -1;

void setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.println("✅ Time synchronized");
  } else {
    Serial.println("❌ Time sync failed");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
  setupTime();

  Wire.begin(8, 9);
  Wire.setClock(100000);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay(); display.display();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(PUMP, LOW);

  const tflite::Model* model = tflite::GetModel(water_model_tflite);
  static tflite::MicroMutableOpResolver<6> resolver;
  resolver.AddFullyConnected(); resolver.AddSoftmax();
  resolver.AddRelu(); resolver.AddQuantize();
  resolver.AddDequantize(); resolver.AddReshape();
  static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, TENSOR_ARENA_SIZE);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("❌ AllocateTensors failed"); while (true);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("✅ System initialized");
}

void loop() {
  static unsigned long lastPress = 0;
  static float lastCandidate = -1;
  static int stableCount = 0;

  if (digitalRead(BUTTON_PIN) == LOW && millis() - lastPress > 300) {
    buzzerEnabled = !buzzerEnabled;
    lastPress = millis();
    Serial.println(buzzerEnabled ? "Buzzer enabled" : "Buzzer disabled");
    digitalWrite(BUZZER_PIN, LOW);
  }

  float distance = readDistanceMode(10);
  float level = calculateWaterLevel(distance);

  if (abs(level - lastCandidate) < 1.5) {
    stableCount++;
    if (stableCount >= 3) {
      lastValidLevel = level;
      updateSystemWithLevel(level);
      stableCount = 0;
    }
  } else {
    lastCandidate = level;
    stableCount = 1;
  }

  delay(1000);
}

float readDistanceMode(int samples) {
  float bins[100] = {0}; int count = 0;

  for (int i = 0; i < samples; i++) {
    digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    float distance = duration * 0.0343 / 2;

    if (duration > 0 && distance >= minDistance && distance <= tankHeight + minDistance + 5) {
      int binIndex = (int)round(distance);
      if (binIndex >= 0 && binIndex < 100) {
        bins[binIndex]++; count++;
      }
    }
    delay(50);
  }

  if (count == 0) return tankHeight + minDistance + 1;

  int maxCount = 0, modeIndex = 0;
  for (int i = 0; i < 100; i++) {
    if (bins[i] > maxCount) {
      maxCount = bins[i];
      modeIndex = i;
    }
  }
  return (float)modeIndex;
}

float calculateWaterLevel(float distance) {
  if (distance >= tankHeight + minDistance) return 0;
  if (distance <= minDistance) return 100;
  return ((tankHeight + minDistance - distance) / tankHeight) * 100.0;
}

void updateSystemWithLevel(float level) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("⚠️ Time unavailable"); return;
  }

  float hourNorm = (timeinfo.tm_hour - mean_hour) / scale_hour;
  float dayNorm = (timeinfo.tm_wday - mean_day) / scale_day;
  input->data.f[0] = hourNorm;
  input->data.f[1] = dayNorm;

  Serial.printf("🧠 Model Input: HourNorm=%.2f, DayNorm=%.2f\n", hourNorm, dayNorm);
  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("❌ ML model inference failed"); return;
  }

  for (int i = 0; i < output->dims->data[output->dims->size - 1]; i++) {
    Serial.printf("🔄 Output[%d] = %.4f\n", i, output->data.f[i]);
  }

  int prediction = getPrediction();
  String usageLevel = getUsageLevel(prediction);
  String tankStatus = getTankStatus(level);

  // Pump and buzzer control
  if (level <= 10) {
    digitalWrite(PUMP, HIGH);
    if (buzzerEnabled && millis() - lastBuzzerTime > buzzerInterval) {
      beepBuzzer(); lastBuzzerTime = millis();
    }
  } else if (level >= 70) {
    digitalWrite(PUMP, LOW);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  displayInfo(level, usageLevel, tankStatus);

  Serial.printf("📊 Level: %.1f%% | Usage: %s | Status: %s\n",
                level, usageLevel.c_str(), tankStatus.c_str());

  if (millis() - lastInfluxSend > influxInterval) {
    sendToInfluxDB(level, timeinfo.tm_hour, timeinfo.tm_wday, prediction, tankStatus, timeinfo);
    lastInfluxSend = millis();
  }
}

int getPrediction() {
  int prediction = 0; float maxVal = -999;
  int output_size = output->dims->data[output->dims->size - 1];
  for (int i = 0; i < output_size; i++) {
    if (output->data.f[i] > maxVal) {
      maxVal = output->data.f[i]; prediction = i;
    }
  }
  return prediction;
}

String getUsageLevel(int prediction) {
  switch (prediction) {
    case 0: return "Low";
    case 1: return "Medium";
    case 2: return "High";
    default: return "Unknown";
  }
}

String getTankStatus(float level) {
  if (level <= 10) return "Empty Soon!";
  else if (level <= 30) return "Low";
  else if (level >= 90) return "Full Soon!";
  else if (level >= 70) return "High";
  return "Normal";
}

void displayInfo(float level, String usageLevel, String tankStatus) {
  display.clearDisplay();
  int barWidth = map(level, 0, 100, 0, SCREEN_WIDTH - 20);
  display.drawRect(10, 10, SCREEN_WIDTH - 20, 10, SSD1306_WHITE);
  display.fillRect(10, 10, barWidth, 10, SSD1306_WHITE);
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 25); display.printf("Level: %.1f%%", level);
  display.setCursor(0, 35); display.print("Usage: "); display.print(usageLevel);
  display.setCursor(0, 45); display.print("Status: "); display.print(tankStatus);
  display.setCursor(0, 55); display.print("Buzzer: "); display.print(buzzerEnabled ? "ON" : "OFF");
  display.display();
}

void beepBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW);
}

void sendToInfluxDB(float level, int hour, int day, int usageLevel, String tankStatus, struct tm timeinfo) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Skipping InfluxDB.");
    return;
  }

  HTTPClient http;
  http.begin(influxdb_url);
  http.addHeader("Authorization", String("Token ") + influxdb_token);
  http.addHeader("Content-Type", "text/plain; charset=utf-8");

  char timeBuf[16];
  strftime(timeBuf, sizeof(timeBuf), "%H:%M", &timeinfo);

  String statusEncoded = tankStatus;
  statusEncoded.replace(" ", "_");

  String postData = "water_level,status=" + statusEncoded +
                    " level=" + String(level) +
                    ",hour=" + String(hour) +
                    ",day=" + String(day) +
                    ",usageLevel=" + String(usageLevel) +
                    ",local_time=\"" + String(timeBuf) + "\"";

  int httpResponseCode = http.POST(postData);
  if (httpResponseCode > 0) {
    Serial.printf("✅ InfluxDB write success: %d\n", httpResponseCode);
  } else {
    Serial.printf("❌ InfluxDB write failed: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  http.end();
}
