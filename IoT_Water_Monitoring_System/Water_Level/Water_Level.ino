// Required Libraries for WiFi, OLED Display, ML model, Time, and HTTP
#include <WiFi.h>  // Handles WiFi connection
#include <Wire.h>  // I2C communication for OLED
#include <Adafruit_GFX.h>  // Graphics library for OLED
#include <Adafruit_SSD1306.h>  // OLED driver
#include "model_data.h"  // Contains the TFLite model array
#include "time.h"  // For getting local time via NTP
#include "tensorflow/lite/micro/micro_interpreter.h"  // TFLite Micro interpreter
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"  // For ops resolution
#include "tensorflow/lite/schema/schema_generated.h"  // TFLite schema definitions
#include <HTTPClient.h>  // Used to send HTTP requests

// OLED Display Configuration
#define SCREEN_WIDTH 128  // OLED width in pixels
#define SCREEN_HEIGHT 64  // OLED height in pixels
#define OLED_RESET -1  // Reset pin not used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // OLED display instance

// Pin Definitions
#define TRIG_PIN 5  // Ultrasonic sensor trigger pin
#define ECHO_PIN 18  // Ultrasonic sensor echo pin
#define BUZZER_PIN 13  // Buzzer pin
#define PUMP 15  // Pump control pin
#define BUTTON_PIN 12  // Button to enable/disable buzzer

// Tank Configuration
const float tankHeight = 17.0;  // Height of the tank in cm
const float minDistance = 1.5;  // Distance from sensor to max water level
bool buzzerEnabled = true;  // Buzzer initially enabled
unsigned long lastBuzzerTime = 0;  // Timestamp for buzzer control
const unsigned long buzzerInterval = 2000;  // 2 seconds between buzzes

// TensorFlow Lite Model Arena and Interpreter
#define TENSOR_ARENA_SIZE 8 * 1024  // Memory for model
uint8_t tensor_arena[TENSOR_ARENA_SIZE];  // Arena buffer
tflite::MicroInterpreter* interpreter;  // Interpreter instance
TfLiteTensor* input;  // Model input tensor
TfLiteTensor* output;  // Model output tensor

// Normalization parameters for model input
float mean_hour = 11.5, scale_hour = 5.0;
float mean_day = 3.0, scale_day = 2.0;

// WiFi and InfluxDB Configuration
const char* ssid = "The BadMans TP";  // WiFi name
const char* password = "Sheesiop00";  // WiFi password
const char* influxdb_url = "http://192.168.0.105:8086/api/v2/write?org=IoT_NTU&bucket=water_level_bucket&precision=s";  // InfluxDB URL
const char* influxdb_token = "FPpTEORxYMQWjwc-0eNAz1ESQ18TxjanfxUaLUWvwFP6FhBJ0SmIk-O8hkPp7OLO9nAJu7-yv8DQwru429qI3g==";  // Token to authenticate InfluxDB

// Time Configuration
const char* ntpServer = "pk.pool.ntp.org";  // NTP server
const long gmtOffset_sec = 5 * 3600;  // Timezone GMT+5
const int daylightOffset_sec = 0;  // No DST

// Data logging configuration
unsigned long lastInfluxSend = 0;  // Last time data was sent to InfluxDB
const unsigned long influxInterval = 5000;  // 5 seconds interval
float lastValidLevel = -1;  // Last stable water level

// --------------------------- Setup Time Function ----------------------------
// Synchronizes ESP32 clock with NTP server
void setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  // Configure NTP
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.println("✅ Time synchronized");  // Success
  } else {
    Serial.println("❌ Time sync failed");  // Failure
  }
}

// ----------------------------- Setup Function ------------------------------
// Runs once on boot: initializes everything
void setup() {
  Serial.begin(115200);  // Start serial monitor
  WiFi.begin(ssid, password);  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
  setupTime();  // Sync time

  Wire.begin(8, 9);  // I2C with custom SDA, SCL pins
  Wire.setClock(100000);  // I2C clock
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // OLED init
  display.clearDisplay(); display.display();  // Clear display

  // Set pin modes
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(BUZZER_PIN, LOW);  // Buzzer off
  digitalWrite(PUMP, LOW);  // Pump off

  // Load and initialize TFLite model
  const tflite::Model* model = tflite::GetModel(water_model_tflite);
  static tflite::MicroMutableOpResolver<6> resolver;  // Supported ops
  resolver.AddFullyConnected(); resolver.AddSoftmax();
  resolver.AddRelu(); resolver.AddQuantize();
  resolver.AddDequantize(); resolver.AddReshape();

  static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, TENSOR_ARENA_SIZE);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("❌ AllocateTensors failed"); while (true);
  }

  input = interpreter->input(0);  // Get input tensor
  output = interpreter->output(0);  // Get output tensor

  Serial.println("✅ System initialized");
}

// ------------------------------ Loop Function ------------------------------
// Main loop runs repeatedly
void loop() {
  static unsigned long lastPress = 0;  // Button debounce
  static float lastCandidate = -1;  // For stable readings
  static int stableCount = 0;

  // Toggle buzzer with button press
  if (digitalRead(BUTTON_PIN) == LOW && millis() - lastPress > 300) {
    buzzerEnabled = !buzzerEnabled;
    lastPress = millis();
    Serial.println(buzzerEnabled ? "Buzzer enabled" : "Buzzer disabled");
    digitalWrite(BUZZER_PIN, LOW);
  }

  float distance = readDistanceMode(10);  // Get stable distance
  float level = calculateWaterLevel(distance);  // Convert to %

  // Use stable readings logic
  if (abs(level - lastCandidate) < 1.5) {
    stableCount++;
    if (stableCount >= 3) {
      lastValidLevel = level;
      updateSystemWithLevel(level);  // Main logic here
      stableCount = 0;
    }
  } else {
    lastCandidate = level;
    stableCount = 1;
  }

  delay(1000);  // Loop delay
}

// ------------------------ Distance Sensor Mode Reading ----------------------
// Returns the mode (most frequent) distance over N samples
float readDistanceMode(int samples) {
  float bins[100] = {0}; int count = 0;

  for (int i = 0; i < samples; i++) {
    digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);  // Read echo
    float distance = duration * 0.0343 / 2;  // Convert to cm

    if (duration > 0 && distance >= minDistance && distance <= tankHeight + minDistance + 5) {
      int binIndex = (int)round(distance);
      if (binIndex >= 0 && binIndex < 100) {
        bins[binIndex]++; count++;
      }
    }
    delay(50);
  }

  if (count == 0) return tankHeight + minDistance + 1;

  // Find mode from histogram
  int maxCount = 0, modeIndex = 0;
  for (int i = 0; i < 100; i++) {
    if (bins[i] > maxCount) {
      maxCount = bins[i]; modeIndex = i;
    }
  }
  return (float)modeIndex;
}

// ---------------------- Calculate Water Level from Distance -----------------
float calculateWaterLevel(float distance) {
  if (distance >= tankHeight + minDistance) return 0;
  if (distance <= minDistance) return 100;
  return ((tankHeight + minDistance - distance) / tankHeight) * 100.0;
}

// ------------------------ Update System Based on Level ----------------------
void updateSystemWithLevel(float level) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("⚠️ Time unavailable"); return;
  }

  // Normalize time for model input
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

  int prediction = getPrediction();  // Get max output index
  String usageLevel = getUsageLevel(prediction);  // Label
  String tankStatus = getTankStatus(level);  // Status

  // Handle pump and buzzer
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

  displayInfo(level, usageLevel, tankStatus);  // Show info on OLED
  Serial.printf("📊 Level: %.1f%% | Usage: %s | Status: %s\n",
                level, usageLevel.c_str(), tankStatus.c_str());

  if (millis() - lastInfluxSend > influxInterval) {
    sendToInfluxDB(level, timeinfo.tm_hour, timeinfo.tm_wday, prediction, tankStatus, timeinfo);
    lastInfluxSend = millis();
  }
}

// ------------------------- Get Max Prediction Index -------------------------
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

// -------------------------- Map Prediction to Label -------------------------
String getUsageLevel(int prediction) {
  switch (prediction) {
    case 0: return "Low";
    case 1: return "Medium";
    case 2: return "High";
    default: return "Unknown";
  }
}

// -------------------------- Map Level to Status -----------------------------
String getTankStatus(float level) {
  if (level <= 10) return "Empty Soon!";
  else if (level <= 30) return "Low";
  else if (level >= 90) return "Full Soon!";
  else if (level >= 70) return "High";
  return "Normal";
}

// -------------------------- Display Info on OLED ----------------------------
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

// -------------------------- Buzzer Beep Function ----------------------------
void beepBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH); delay(100); digitalWrite(BUZZER_PIN, LOW);
}

// -------------------- Send Data to InfluxDB Server --------------------------
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
  strftime(timeBuf, sizeof(timeBuf), "%H:%M", &timeinfo);  // Format time

  String statusEncoded = tankStatus;
  statusEncoded.replace(" ", "_");  // For Influx compatibility

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
