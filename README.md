# teensy4_mouse
Arduino code for a high speed 8000Hz wired mouse using a teensy 4 MCU. 
This code is inspired by https://github.com/mrjohnk/PMW3360DM-T2QU and https://github.com/SunjunKim/PMW3360_Arduino.

This project includes code for a modified 8000Hz wired mouse.
It uses standard libraries included with the arduino IDE and teensyduino. 

Hardware used for this project:

PMW3360 image sensor https://www.pixart.com/products-detail/10/PMW3360DM-T2QU

Teensy4 microcontroller development board https://www.pjrc.com/store/teensy40.html 

Glorious Model O wired version. https://www.pcgamingrace.com/products/glorious-model-o-black

It should be possible to do this project with any mouse that uses a PMW3360 sensor.
A pinout is given below: 

![alt text](https://github.com/Trip93/teensy4_mouse/blob/main/pictures/teensy4_mouse_pinout.png)

The teensy4 uses a microcontroller capable of USB high speed and therefore high polling speed (8000Hz). 
No drivers are required to enable 8000Hz. Signal integrity of the cable is important. 
The cable of the glorious mouse was shortened to 1 meter length and USB data lines were soldered straight to the Teensy4 D- and D+ pads. 

Furthermore this modification supports fast button inputs without software based debouncing. 
Both NC and NO connectors of the micro switch are connected to the microcontroller. Enabling fast state change detection without contact bounce problems.

![alt text](https://github.com/Trip93/teensy4_mouse/blob/main/pictures/NC_NO_debouncing.png)

A more detailed explanation and possibly a video of the project will be disclosed later.
