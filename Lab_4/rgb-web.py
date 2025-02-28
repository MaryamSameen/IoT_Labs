print("Hello, Maryam!")

import network
import socket
import time
from machine import Pin
from neopixel import NeoPixel

pin = Pin(48, Pin.OUT)   # set GPIO48  to output to drive NeoPixel
neo = NeoPixel(pin, 1)   # create NeoPixel driver on GPIO48 for 1 pixel

# Connect to Wi-Fi
ssid = "Maryam"
password = "sajida06"
sta = network.WLAN(network.STA_IF)
sta.active(True)
sta.connect(ssid, password)

while not sta.isconnected():
    time.sleep(1)

print("Connected!Your IP Address to Browse is:", sta.ifconfig()[0])

# Start Web Server
def web_page():
    html = """<!DOCTYPE html>
    <html>
    <head>
    <title>ESP32 RGB LED Control System</title>
    </head>
    <body>

    <h1 style="text-align: center;">ESP32 RGB LED Control System</h1>
    <hr>

    <p style="text-align: center;"><b>Click a button to change the LED color:</b></p>

    <div style="text-align: center;">
        <p><a href="/?RGB=red"><button>Turn On RED Light</button></a></p>
        <p><a href="/?RGB=green"><button>Turn On GREEN Light</button></a></p>
        <p><a href="/?RGB=blue"><button>Turn On BLUE Light</button></a></p>
    </div>

    <hr>
    <p style="text-align: center;"><i>ESP32 Web Controlled RGB LED</i></p>

    </body>
    </html>
     """
    return html

soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
soc.bind((sta.ifconfig()[0], 80))
soc.listen(5)

while True:
    conn, addr = soc.accept()
    print("Connection from:", addr)
    request = conn.recv(1024).decode()
    print("Request:", request)
    
    if "/?RGB=red" in request:
        neo[0] = (255, 0, 0) # set the first pixel to red
        neo.write()              
    elif "/?RGB=green" in request:
        neo[0] = (0, 255, 0) # set the first pixel to green
        neo.write()              
    elif "/?RGB=blue" in request:
        neo[0] = (0, 0, 255) # set the first pixel to blue
        neo.write()              
        
    response = web_page()
    conn.send("HTTP/1.1 200 OK\nContent-Type: text/html\n\n")
    conn.send(response)
    conn.close()





