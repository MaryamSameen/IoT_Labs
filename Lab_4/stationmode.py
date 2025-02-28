print("Hello, Maryam!")

import network
import time

ssid = "Maryam"
password = "sajida06"

print("Connecting to WiFi...", end="")
sta = network.WLAN(network.STA_IF)
sta.active(True)
sta.connect(ssid , password)

# Wait for connection
for _ in range(10):
    if sta.isconnected():
        break
    time.sleep(1)
    
    
if sta.isconnected():
    print("\nConnected to WiFi Succsessfully!")
    print("IP Address of wifi:", sta.ifconfig()[0])
else:
    print("\nFailed to connect to wifi!!!")

