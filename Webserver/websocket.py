print("Hello, Maryam!")

import network
import socket
import time
import machine
from machine import Pin ,I2C
from neopixel import NeoPixel
import dht
import ssd1306

#oled display
i2c = I2C(scl=Pin(9), sda=Pin(8))
oled = ssd1306.SSD1306_I2C(128, 64, i2c)


def update_oled(message):
    oled.fill(0)
    oled.text(message, 0, 0)
    oled.show()


DHT_PIN = 4  # DHT11 data pin

# Initialize DHT11 sensor
dht_sensor = dht.DHT11(machine.Pin(DHT_PIN)) # change DHT11 fr physical device

#DHT Measure

def  dht():
    dht_sensor.measure()
    time.sleep(0.2)
    temp = dht_sensor.temperature()
    humidity = dht_sensor.humidity()
    return temp,humidity

pin = Pin(48, Pin.OUT)   # set GPIO48  to output to drive NeoPixel
neo = NeoPixel(pin, 1)   # create NeoPixel driver on GPIO48 for 1 pixel

ssid_st = "The BadMans TP"
password_st = "Sheesiop00"

print("Connecting to WiFi", end="")
sta = network.WLAN(network.STA_IF)
sta.active(True)
sta.connect("The BadMans TP" , "Sheesiop00")

# Wait for connection
for _ in range(10):
    if sta.isconnected():
        break
    time.sleep(1)
    
    
if sta.isconnected():
    print("Connected to WiFi!")
    print("IP Address as station:", sta.ifconfig()[0])
else:
    print("Failed to connect")
    




ssid_ap = "ESP32_AP"
password_ap = "12345678"  # Minimum 8 characters
auth_mode = network.AUTH_WPA2_PSK  # Secure mode (recommended)

# Create an Access Point
ap = network.WLAN(network.AP_IF)
ap.active(True)  # Activate AP mode
ap.config(essid=ssid_ap, password=password_ap, authmode=auth_mode)  # Set SSID and password

print("Access Point Active")
print("AP IP Address:", ap.ifconfig()[0])


# Start Web Server
def web_page():
    temp, humidity = dht()
    html = f"""<!DOCTYPE html>
    <html>
<head>
    <title>ESP32 RGB LED Control</title>
</head>
<body style="font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f4; padding: 20px;">
    <h1 style="color: #333;">ESP32 RGB LED Control</h1>
    <p><a href="/?RGB=red"><button style="background-color: red; color: white; padding: 10px 20px; border: none; font-size: 16px; cursor: pointer;">Turn RGB RED</button></a></p>
    <p><a href="/?RGB=green"><button style="background-color: green; color: white; padding: 10px 20px; border: none; font-size: 16px; cursor: pointer;">Turn RGB GREEN</button></a></p>
    <p><a href="/?RGB=blue"><button style="background-color: blue; color: white; padding: 10px 20px; border: none; font-size: 16px; cursor: pointer;">Turn RGB BLUE</button></a></p>
    <br>
    <h1 style="color: #333;">Temperature and Humidity</h1>
    <h2 style="color: #444;">Temp: {temp}&deg;C</h2>
    <h2 style="color: #444;">Humidity: {humidity}%</h2>
    <br>
    <br>
    <h2>Display on OLED</h2>
    <form action="/"><input name="msg" type="text"><input type="submit" value="Send"></form>
</body>
</html>
"""
    return html

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(("0.0.0.0", 80))
s.listen(5)

while True:
    conn, addr = s.accept()
    print("Connection from:", addr)
    request = conn.recv(1024).decode()
    print("Request:", request)
    
    if "/?RGB=red" in request:
        neo[0] = (255, 0, 0) # set the first pixel to red
        neo.write()              # write data to all pixels
    elif "/?RGB=green" in request:
        neo[0] = (0, 255, 0) # set the first pixel to green
        neo.write()              # write data to all pixels
    elif "/?RGB=blue" in request:
        neo[0] = (0, 0, 255) # set the first pixel to blue
        neo.write()              # write data to all pixels
    elif "msg=" in request:
        msg = request.split("msg=")[1].split(" ")[0].replace("+", " ")
        print(msg)
        update_oled(msg)    
        
    response = web_page()
    conn.send("HTTP/1.1 200 OK\nContent-Type: text/html\n\n")
    conn.send(response)
    conn.close()

