print("Hello, Maryam!")

import network
import BlynkLib
import neopixel
from machine import Pin

# WiFi Credentials
WIFI_SSID = "NTU FSD"     # Replace with your WiFi SSID
WIFI_PASS = ""  # Replace with your WiFi Password

# Blynk Auth Token
BLYNK_AUTH = "zwJFERTtCzGLgaYNYNXCeJc_4IMPTfeE"

# Connect to WiFi
wifi = network.WLAN(network.STA_IF)
wifi.active(True)
wifi.connect(WIFI_SSID, WIFI_PASS)
while not wifi.isconnected():
    pass
print("Connected to WiFi")

# Initialize Blynk
blynk = BlynkLib.Blynk(BLYNK_AUTH)

# Define NeoPixel LED pin
neo_pin = 48  # Built-in RGB LED pin
num_leds = 1  # Only one built-in RGB LED
np = neopixel.NeoPixel(Pin(neo_pin), num_leds)

# Track current active color and brightness
active_color = (0, 0, 0)
brightness = 255  # Default brightness

# Function to update the LED color
def update_led():
    scaled_color = tuple(int(c * (brightness / 255)) for c in active_color)
    np[0] = scaled_color
    np.write()

# Virtual pin handlers
@blynk.on("V0")  # Red Button
def v0_handler(value):
    global active_color
    if int(value[0]) == 1:
        active_color = (255, 0, 0)
        update_led()

@blynk.on("V1")  # Green Button
def v1_handler(value):
    global active_color
    if int(value[0]) == 1:
        active_color = (0, 255, 0)
        update_led()

@blynk.on("V2")  # Blue Button
def v2_handler(value):
    global active_color
    if int(value[0]) == 1:
        active_color = (0, 0, 255)
        update_led()

@blynk.on("V3")  # Brightness Slider
def v3_handler(value):
    global brightness
    brightness = int(value[0])
    update_led()

# Run Blynk loop
while True:
    blynk.run()

