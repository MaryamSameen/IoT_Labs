### Task 1:

Locally run the mosquitto and Publish the data.

**File:** `esp_2_mos_simple.ino`  
**Description:**  
The ESP32 board reads temperature and humidity from a DHT11 sensor and publishes the values to an MQTT broker on two topics:
- `esp32/dht/temp`
- `esp32/dht/hum`

This acts as the primary data source for the rest of the system.

---

### ✅ Task 2: DHT Data Listener and Storage

**File:** `1-dht_data_only.py`  
**Description:**  
A Python script that subscribes to MQTT topics, listens for temperature and humidity data from ESP32, and writes the values into **InfluxDB** under the `dht_data` measurement in the `Dht_Lab13` bucket.

---

### ✅ Task 3: Train Classifier Model with Noisy Data

**File:** `2-train_model_with_noise.py`  
**Description:**  
This task involves generating synthetic, slightly noisy temperature-humidity data and training a neural network model to classify the data into five classes:
- Normal
- Hot and Humid
- Cold and Dry
- Hot and Dry
- Cold and Humid

The model is saved as `dht_classifier.h5` and normalization values are stored in `normalization.npz`.

Includes:
- Early stopping for better training control.
- Evaluation via classification report and confusion matrix.

---

### ✅ Task 4: Real-time Classification and Logging

**File:** `3-classify_2_influx_.py`  
**Description:**  
This script performs real-time classification of incoming DHT data using the trained model. It:
- Subscribes to the same MQTT topics.
- Normalizes data using saved parameters.
- Predicts environmental class using the model.
- Stores temperature, humidity, and predicted class into InfluxDB with a UTC timestamp.

---

## 🧰 Requirements

Install the required Python packages using pip:

```bash
pip install paho-mqtt influxdb-client tensorflow scikit-learn matplotlib seaborn
