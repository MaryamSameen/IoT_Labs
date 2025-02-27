from machine import Pin 
from machine import Pin, I2C  #protocol for interface for oled
import machine   
import ssd1306  #Sda->data #scl->clock
import dht
import time

DHT_PIN = 4  # DHT22 data pin

# Initialize DHT22 sensor  (returns value of humidity and temperature)
dht_sensor = dht.DHT22(machine.Pin(DHT_PIN)) # change DHT11 fr physical device

# Initialize OLED display
i2c = machine.I2C(scl=machine.Pin(9), sda=machine.Pin(8))
oled = ssd1306.SSD1306_I2C(128, 64, i2c)  #software for led ,128 x-pixels col,64 y-pixels rows


# Main loop
while True:
    try:
        dht_sensor.measure()
        time.sleep(.2)
        temp = dht_sensor.temperature()
        humidity = dht_sensor.humidity()
        #print(temp, humidity)
        oled.fill(0)  #clear screen for oled
        oled.text("Hello welcome!",0,0)
        oled.text("Maryam Sameen",0,9)
        oled.text("22-NTU-CS-1354",0,18)
        oled.text("IOT Lab",24,27)
        oled.text("!!...Mazzaay...!!",0,35)
        oled.text("Submit:Sir Nasir",0,45)
        oled.text("I am just a Girl",0,56) 
        #oled.text("Temp: {} C".format(temp), 0, 0) #0,0x axis ad y axis (next text after 8 pixels in y axis)
        #oled.text("Humidity: {}%".format(humidity), 0, 16) #0 x axis, 16 y axis(means one line left) , 16 char x axis and 8 char in y axis
        oled.show()



    except Exception as e:
        print("Error reading DHT22 sensor:", e)
    
        
    time.sleep(1)  # Update every 2 seconds