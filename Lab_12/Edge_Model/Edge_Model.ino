#include <Arduino.h>
#include <DHT.h>
#include <esp_heap_caps.h>

// TensorFlow Lite Micro headers
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// Include your converted model
#include "model_data.h"  // <- generated from dht_classifier.tflite

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// TensorFlow Lite model settings
constexpr int kTensorArenaSize = 40 * 1024;
uint8_t* tensor_arena;
const unsigned char* model_data_ptr = nullptr;

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Allocate PSRAM for the model
  model_data_ptr = (const unsigned char*)heap_caps_malloc(dht_classifier_tflite_len, MALLOC_CAP_SPIRAM);
  if (!model_data_ptr) {
    Serial.println("❌ Failed to allocate PSRAM for model!");
    while (1);
  }

  memcpy((void*)model_data_ptr, dht_classifier_tflite, dht_classifier_tflite_len);
  model = tflite::GetModel(model_data_ptr);

  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("❌ Model schema mismatch!");
    while (1);
  }

  // Allocate tensor arena
  tensor_arena = (uint8_t*)heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM);
  if (!tensor_arena) {
    Serial.println("❌ Failed to allocate tensor arena in PSRAM!");
    while (1);
  }

  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("❌ Tensor allocation failed");
    while (1);
  }

  Serial.println("✅ Setup complete. Starting inference...");
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("⚠️ Failed to read from DHT sensor!");
    delay(2000);
    return;
  }

  // Normalization parameters (replace these with your actual values)
np.savez("normalization.npz", min=X_min, max=X_max)


  float norm_temp = (temperature - t_min) / (t_max - t_min);
  float norm_hum = (humidity - h_min) / (h_max - h_min);

  // Set input tensor
  TfLiteTensor* input = interpreter->input(0);
  input->data.f[0] = norm_temp;
  input->data.f[1] = norm_hum;

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("❌ Inference failed");
    return;
  }

  // Get prediction
  TfLiteTensor* output = interpreter->output(0);
  int prediction = std::max_element(output->data.f, output->data.f + 5) - output->data.f;

  // Labels (match your training labels)
  const char* labels[] = {"Normal", "HotHumid", "ColdDry", "HotDry", "ColdHumid"};

  Serial.print("🌡 Temp: ");
  Serial.print(temperature);
  Serial.print("°C, 💧 Hum: ");
  Serial.print(humidity);
  Serial.print("% => 🧠 Prediction: ");
  Serial.println(labels[prediction]);

  delay(5000);  // 5 seconds delay
}
