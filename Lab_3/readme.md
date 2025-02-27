# LAB3 - MicroPython Lab Tasks

## LabTasks

### Task 1: Displaying Humidity & Temperature on OLED (Wokwi)
In this task, I connected the DHT22 sensor with the ESP and showed temperature and humidity readings on the OLED using Wokwi. It was cool seeing live data pop up on the small screen. I just had to keep refreshing the readings every second, and sometimes it took a sec to load, but it worked fine overall.

---

### Task 2: Exploring OLED Display (128 x 64)
This task was about playing with the OLED screen. I learned that the OLED has 128 pixels in width and 64 in height. Each character I displayed took 8 pixels, so fitting stuff on the screen needed some thinking. I just wrote "Welcome IoT" to see how it looks. It was simple but fun to see text appear like magic.Here, I tried to center the text "Welcome IoT" on the OLED. I realized it's not just about putting random numbers for x and y positions; you gotta do some calculations. I guessed some values to place it roughly in the middle. Not perfectly centered but close enough. It was kinda tricky but interesting.

---

### Task 3: Interrupt and Debounce on ESP32
This task was different. I uploaded the code directly to the ESP32, where I used interrupts and debounce concepts. Pressing the button turned the OLED on and off, but without debounce, it kept flickering. Adding debounce with a timer fixed that. It was fun seeing how a small press could control the screen. Learned how interrupts react instantly, which was pretty cool.