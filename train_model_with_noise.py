import pandas as pd
import numpy as np
import tensorflow as tf
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.utils import class_weight
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, Dropout, GaussianNoise

# Step 1: Load Dataset
df = pd.read_csv("water_usage.csv")

# Step 2: Feature + Label
X = df[["hour", "day"]].values
y = df["label"].values

# Step 3: Normalize
scaler = StandardScaler()
X_scaled = scaler.fit_transform(X)

# Step 4: One-hot encode labels
y_cat = tf.keras.utils.to_categorical(y, num_classes=3)

# Step 5: Train-test split
X_train, X_test, y_train, y_test = train_test_split(X_scaled, y_cat, test_size=0.2, random_state=42)

# Step 6: Handle imbalance
weights = class_weight.compute_class_weight(class_weight='balanced', classes=np.unique(y), y=y)
class_weights = dict(enumerate(weights))
print("Class Weights:", class_weights)

# Step 7: Build Model
model = Sequential([
    GaussianNoise(0.1, input_shape=(2,)),
    Dense(64, activation='relu'),
    Dropout(0.3),
    Dense(32, activation='relu'),
    Dense(3, activation='softmax')
])
model.compile(optimizer='adam', loss='categorical_crossentropy', metrics=['accuracy'])

# Step 8: Train
model.fit(X_train, y_train, epochs=50, validation_data=(X_test, y_test), class_weight=class_weights, verbose=1)

# Step 9: Evaluate
train_acc = model.evaluate(X_train, y_train, verbose=0)[1]
test_acc = model.evaluate(X_test, y_test, verbose=0)[1]
print(f"\n✅ Train Accuracy: {train_acc*100:.2f}%")
print(f"✅ Test Accuracy : {test_acc*100:.2f}%")

# Step 10: Save model + scaler
model.save("water_model.h5")
np.savez("normalization.npz", mean=scaler.mean_, scale=scaler.scale_)

# Step 11: Convert to TFLite
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()
with open("water_model.tflite", "wb") as f:
    f.write(tflite_model)

print("📦 Saved: water_model.h5, water_model.tflite, normalization.npz")
