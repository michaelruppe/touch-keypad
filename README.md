# Touch Keypad

This project is for a general-purpose, capacitive-touch keypad powered by a ESP32 and a matching receiver.

This project was originally designed as a keypad entry system for my garage door, but could easily be modified to other purposes. It is essentially a touch-keypad and receiver pair, where all authorisation is handled by the receier. The keypad only transmits information about what key sequence has been pressed.



## Keypad PCB Features

- Capacitive touch sensors - simplify assembly
- All components are on the back side
- Prototyping connections Vin, GND, Tx, Rx
- Gerbers pre-generated and LCSC components specified in BOM file


## Receiver PCB Features

- 3ch open-drain outputs
- Wide input voltage
- Pass-through terminals for daisy-chaining other controllers