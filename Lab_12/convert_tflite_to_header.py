def bin_to_c_array(input_file, output_file, array_name="dht_classifier_tflite"):
    with open(input_file, "rb") as f:
        data = f.read()

    with open(output_file, "w") as f:
        f.write(f"const unsigned char {array_name}[] = {{\n")
        for i in range(0, len(data), 12):
            chunk = data[i:i+12]
            line = ", ".join(f"0x{b:02x}" for b in chunk)
            f.write("  " + line + ",\n")
        f.write("};\n")
        f.write(f"const unsigned int {array_name}_len = {len(data)};\n")

# Call the function
bin_to_c_array("dht_classifier.tflite", "model_data.h")
