# Converts TFLite model to C header file for Arduino
input_file = "water_model.tflite"
output_file = "model_data.h"
array_name = "water_model_tflite"

with open(input_file, "rb") as f:
    data = f.read()

with open(output_file, "w") as f:
    f.write(f"const unsigned char {array_name}[] = {{\n")
    for i, byte in enumerate(data):
        if i % 12 == 0:
            f.write("  ")
        f.write(f"0x{byte:02x}, ")
        if (i + 1) % 12 == 0:
            f.write("\n")
    f.write("\n};\n")
    f.write(f"const unsigned int {array_name}_len = {len(data)};\n")
