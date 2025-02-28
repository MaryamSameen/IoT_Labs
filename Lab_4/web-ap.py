import network
import socket

# Setup Access Point
ssid = "ESP32_Maryam"
password = "06112002"
ap = network.WLAN(network.AP_IF)
ap.active(True)
ap.config(essid=ssid, password=password)

print("Access Point Active of ESP32_Maryam, IP:", ap.ifconfig()[0])

# Start Web Server
def web_page():
    html = """<!DOCTYPE html>
    <html>
    <head>
    <title>IoT-AI 6th Sp25 ESP32 Web Server</title>
    </head>
    <body>

    <h1 style="text-align: center;">ESP32 Web Server</h1>
    <hr>

    <p style="text-align: center;"><b>Welcome to the ESP32 Web Server!</b></p>
    <p style="text-align: center;">This server is running in <b>AP Mode</b> for the <b>IoT class</b> of AI 6th in SP25.</p>

    <hr>
    <p style="text-align: center;"><i>Powered by ESP32 | IoT & AI Integration</i></p>

    </body>
    </html>
     """
    return html

# Setup Socket Server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(("0.0.0.0", 80))
s.listen(5)

while True:
    conn, addr = s.accept()
    print("Connection from:", addr)
    request = conn.recv(1024)
    print("Request:", request)
    
    response = web_page()
    conn.send("HTTP/1.1 200 OK\nContent-Type: text/html\n\n")
    conn.send(response)
    conn.close()

