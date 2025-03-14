from microdot import Microdot
import network
# Wi-Fi credentials
SSID = "The BadMans TP"
PASSWORD = "sheesiop00"
# Connect to Wi-Fi
def connect_wifi():
 sta_if = network.WLAN(network.STA_IF)
 if not sta_if.isconnected():
  print("Connecting to Wi-Fi...")
  sta_if.active(True)
  sta_if.connect(SSID, PASSWORD)
  while not sta_if.isconnected():
   pass
 print("Connected to Wi-Fi")
 print("IP Address:", sta_if.ifconfig()[0])
# Initialize MicroDot app
app = Microdot()
# Root route
@app.route('/')
def index(request):
 return "Hello from ESP32 Web Server!"
# Start the server
def start_server():
 connect_wifi()
 app.run(port=80)
# Run the server
start_server()

