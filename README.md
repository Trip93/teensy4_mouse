# teensy4_mouse
Arduino code for a high speed 8000Hz wired mouse using a teensy 4 MCU. 
This code is inspired by https://github.com/mrjohnk/PMW3360DM-T2QU and https://github.com/SunjunKim/PMW3360_Arduino.

This project includes code for a modified 8000Hz wired mouse.
It uses standard libraries included with the arduino IDE and teensyduino. 

Hardware relevant for this project:

PMW3360 image sensor https://www.pixart.com/products-detail/10/PMW3360DM-T2QU

Teensy4 microcontroller development board https://www.pjrc.com/store/teensy40.html 

Glorious Model O wired version. https://www.pcgamingrace.com/products/glorious-model-o-black

![alt text](https://github.com/Trip93/teensy4_mouse/blob/main/pictures/mouse_top_closed.jpg)
Glorious mouse modified with the teensy internally.

The following connection need to be made to the mouse sensor. The power supply pins are already taken care of by the mouse itself.
The microcontroller of the mouse should also be removed. Recommended is to use a hot air station to remove the microcontroller. 
It should be possible to do this project with any mouse that uses a PMW3360 sensor.
A pinout is given below: 

![alt text](https://github.com/Trip93/teensy4_mouse/blob/main/pictures/teensy4_mouse_pinout.png)

The teensy4 uses a microcontroller capable of USB high speed and therefore high polling speed (8000Hz). 
No drivers are required to enable 8000Hz. Signal integrity of the cable is important. 
The cable of the glorious mouse was shortened to 1 meter length and USB data lines were soldered straight to the Teensy4 D- and D+ pads. 
Picture of the modified mouse are given below:

![alt text](https://github.com/Trip93/teensy4_mouse/blob/main/pictures/mouse_top_open.jpg)

![alt text](https://github.com/Trip93/teensy4_mouse/blob/main/pictures/mouse_bottom_open.jpgJ)


To verify 8000Hz operation mousetester1.5.3 was used https://www.overclock.net/threads/mousetester-software-reloaded.1590569/ 

Polling rate is limited by the speed at which the user moves the mouse. The graph has shows what happens when the user moves the mouse at varying speeds. 
At low speeds polling rates are closer to a 1000-2000Hz (0.5 - 1mS intervals), moderate speeds average at 4000hz (0.250ms intervals) and high speeds average at 8000Hz.
The PWM3360 adjusts the sensor framerate based on the speed that the user moves it. This can be up to 12000fps.

![alt text](https://github.com/Trip93/teensy4_mouse/blob/main/pictures/mousetester_teensy_mouse.png)

Furthermore this modification supports fast button inputs without software based debouncing. 
Both NC and NO connectors of the micro switch are connected to the microcontroller. Enabling fast state change detection without contact bounce problems.

![alt text](https://github.com/Trip93/teensy4_mouse/blob/main/pictures/NC_NO_debouncing.png)
