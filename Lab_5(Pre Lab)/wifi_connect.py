import network
import time

SSID = "Maryam"
PASSWORD = "sajida06"

def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(SSID, PASSWORD)
    
    while not wlan.isconnected():
        print("Connecting to WiFi...")
        time.sleep(1)

    print("Connected to:", wlan.ifconfig())

connect_wifi()
