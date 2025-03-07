
#RGB controling using Blynk cloude text input
import BlynkLib as blynklib
import network
import uos
import utime as time
from machine import Pin, I2C, Timer
from neopixel import NeoPixel
import ssd1306

#giving the wifi credentials
SSID = 'NTU FSD'
PASS = ''
BLYNK_AUTH = "JjQ_-rdjFVGDI9S6r_m3HBwDbgAJXzt6"

print("Connecting to WiFi network... '{}'".format(SSID))
wifi = network.WLAN(network.STA_IF)
wifi.active(True)
wifi.connect(SSID, PASS)
while not wifi.isconnected():
    time.sleep(1)
    print('WiFi connect retry ...')
print('WiFi IP Address:', wifi.ifconfig()[0])

print("Connecting to Blynk server in 1 2 3...")
blynk = blynklib.Blynk(BLYNK_AUTH)



i2c = I2C(1, scl=Pin(9), sda=Pin(8), freq= 200000)
oled = ssd1306.SSD1306_I2C(128, 64, i2c)


# Blynk Handlers for Virtual Pins
@blynk.on("V0")  # text input
def v0_handler(value):
    try:
        # Parse the input text (expected format: "R,G,B")
        oled.fill(0)
        
        oled.text(value[0], 5,5)
        oled.show()
    except Exception as e:
        print("Invalid input:", e)
    
@blynk.on("connected")
def blynk_connected():
    print("Blynk is Connected successfuly yayy!!!!")
    blynk.sync_virtual(0)  # Sync RGB sliders from the app

@blynk.on("disconnected")
def blynk_disconnected():
    print("Blynk is Disconnected!")

# Main Loop
while True:
    blynk.run()
    time.sleep(0.1)




