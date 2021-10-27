# teensy4_mouse
Arduino code for a high speed 8000Hz wired mouse using a teensy 4 MCU

This project includes code for a modified 8000Hz wired mouse using the PMW3360 sensor and Teensy4 microcontroller development board. 
The mouse used to implement this modification was the Glorious Model O wired version. But it should be possible to implement this on any mouse that uses a PMW3360 sensor.
A pinout is given below the USB connector of the teensy was desoldered and wires were soldered straight to the D- and D+ pins. 
![alt text](https://github.com/Trip93/teensy4_mouse/blob/main/pictures/teensy4_mouse_pinout.png)

The teensy4 uses a microcontroller capable of USB high speed and therefore high polling speed (8000Hz). 
No drivers are required to enable 8000Hz. Signal integrity of the cable is important. 
The cable of the glorious mouse was shortened to 1 meter length and USB data lines were soldered straight to the Teensy4 D- and D+ pads. 

Furthermore this modification supports very fast button inputs without software based debouncing. 
Both NC and NO connectors of the micro switch are connected to the microcontroller. Enabling very fast state change detection without contact bounce problems.
![alt text](https://github.com/Trip93/teensy4_mouse/blob/main/pictures/NC_NO_debouncing.png)

A more detailed explanation and possibly a video of the project will be disclosed later.
