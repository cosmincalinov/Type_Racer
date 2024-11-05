# Type Racer
This project uses an ATMega328P microcontroller (with the Arduino Framework) to emulate a type racer game.

## Table of contents
- [Overview](#overview)
- [Hardware Setup](#hardware-setup)
- [Installation](#installation)
- [Code Overview](#code-overview)
- [Usage](#usage)


## Overview


## Hardware Setup
### Components
- Arduino UNO board (ATmega328P microcontroller)
- x1 RGB LED - shows if a word was type correctly or not
- x2 push buttons (BTN1 - start/stop the round, 
                   BTN2 - selecting the difficulty)
- x3 220Ω resistors
- x2 5K1 resistors (for buttons)
- Breadboard
- Jumper wires

### Connections


### Project schematic
![Circuit](./aux/schema1.png)
## Live circuit
![circ1](./aux/poza_circ.jpeg)
![circ2](./aux/poza_circ2.jpeg)
## Code Overview
```
void BTN2pressed()
```
Checks if BTN2 was pressed for a second using debouncing and interrupts the charging state.
```
void syncBlink()
```
If charging is complete, makes all the LEDs blink synchronously for 3 times.
```
void charge()
```
Starts the charging process when BTN1 is pressed and checks is BTN2 was pressed.
## Usage
1. After connecting the serial cable to your PC and Arduino board, wait for the RGB LED to turn green.
2. Press BTN1 to begin the charging process and the blue LEDs will start blinking, while the RGB LED turns red.
3. While charging, you can stop the process by pressing BTN2 for 1-3 seconds. Alternatively, you can wait for the charging to end by itself.

## Final result
https://youtu.be/pdLlaTGCSkE