#Task 1

from machine import Pin 
from machine import Pin, I2C
import machine
import ssd1306 
import dht
import time

DHT_PIN = 4  # DHT22 data pin

# Initialize DHT22 sensor
dht_sensor = dht.DHT11(machine.Pin(DHT_PIN)) # change DHT11 fr physical device

# Initialize OLED display
i2c = machine.I2C(scl=machine.Pin(9), sda=machine.Pin(8))
oled = ssd1306.SSD1306_I2C(128, 64, i2c)


# Main loop
while True:
    try:
        dht_sensor.measure()
        time.sleep(.2)
        temp = dht_sensor.temperature()
        humidity = dht_sensor.humidity()
        print(temp, humidity)
        oled.fill(0)
        oled.text("Temp : {} C".format(temp), 0, 0)
        oled.text("Humidity:{}%".format(humidity), 0, 16)
        
#        oled.text("Temp 🥵: {} C".format(temp), 0…