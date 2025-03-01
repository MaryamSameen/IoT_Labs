### Wowki Project Link:

https://wokwi.com/projects/424228055575420929

### Task 1 : Blow on the sensor and observe whether it detects minor changes in temperature and humidity.

Yes, It changes the temperature and humidity after a warm Blow on the sensor after a proper delay we have given it.

-> Emojis does not get supported simply in esp, I tried using framebuff Library to display it as a Bitmap emoji but it didn't work, so I simply added Text emojis.

### Task 2 : Running the Code Without Interrupt


Before (With Interrupts):

1- The button press was detected and processed instantly.
2- The CPU was free to perform other tasks without checking the button constantly.
3- Was Allowing other tasks to perform their execution without blocking them.

After (Without Interrupts):

1- The button press is checked once per loop iteration, leading to slight delays.
2- The system continuously checks for a button press i.e Polling.
3- CPU spent more time in checking the state of button leading other exections of program in delay.

### Task 3: Understanding debouncing issue

### 1. What is a Debounce Issue and Why Do We Get Rid of It?

The debounce issue occurs when a mechanical switch or button is pressed, and instead of registering a single press, 
it registers multiple rapid signals due to physical vibrations (bouncing contacts). This happens because mechanical 
contacts do not make a perfect connection instantly, causing unintended multiple triggers.

### Why We Get Rid of It?

1- Prevents unwanted multiple detections of a single press.
2- Ensures reliable and stable input processing.
3- Avoids bugs and unexpected behavior in applications that rely on accurate button presses.

### 2. Applications Where Debounce Issues Can Be Critical

If debounce issues are not handled, they can cause failures in multiple domains, such as:

1- Embedded Systems: A switch press may be registered multiple times, leading to unintended actions.
2- Industrial Control Systems: False triggers in control buttons may disrupt machine operations.
3- Medical Devices: Unstable button presses in life-critical devices (like ventilators or ECG machines) can lead to errors.
4- Gaming & Keyboards: A single key press may be registered multiple times, causing incorrect input.
5- Automotive Systems: Debounce issues in car buttons (e.g., engine start/stop) could lead to dangerous failures.

### 3. Why Does Debounce Occur?

Debounce occurs due to mechanical contact behavior in switches, not because of a compiler error,
logic error, or a cheap microcontroller.

### Reason for Debounce:
When you press a button, the metal contacts inside the switch vibrate before settling.
This vibration sends multiple fast, unintended pulses before stabilizing, leading to multiple detections of a single press.

### What It’s Not:

1- Not a Compiler Error: The compiler translates code correctly; it's a hardware issue.
2- Not a Logical Error: The program logic works fine, but hardware behavior causes multiple triggers.
3- Not Because of a Cheap Microcontroller: Even high-quality MCUs experience this if debouncing isn’t handled.

### How to Fix It?

1- Software Debouncing: Adding a small delay (e.g., 200ms) or checking for a stable signal before registering a press.
2- Hardware Debouncing: Using capacitors or specialized circuits (Schmitt triggers) to filter out bouncing signals.

### ✅Conclusion: 

Debounce is a hardware issue caused by mechanical switch behavior. 
It must be handled properly in applications where accuracy matters to prevent errors and improve system reliability. 🚀

### Task 4: : Why Do We Use Interrupts

### • Why do we use interrupt?

Interrupts are used to improve efficiency and responsiveness in microcontrollers by allowing the system to react to
external or internal events immediately, without constantly checking for them in a loop.

### • How does interrupt lower the processing cost of the micro-controller?

Interrupts reduce processing costs by eliminating the need for constant polling (i.e., checking for an event repeatedly in a loop).
