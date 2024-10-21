# Touch Keypad

This project is for a general-purpose, capacitive-touch keypad powered by a ESP32 and a matching receiver.

This project was originally designed as a keypad entry system for my garage door, but could easily be modified to other purposes. It is essentially a touch-keypad and receiver pair, where all authorisation is handled by the receier. The keypad only transmits information about what key sequence has been pressed.

## Instructions for Use

### Opening the Door
1. Enter your authorised code on the keypad.
2. Press the UP key.
3. If the code is correct, you'll hear an ascending tone, and the door will begin to open.
4. If the code is incorrect, you'll hear a descending tone. The door will not open.

### Closing the Door
1. Simply press the DOWN button on the keypad.
2. The door will begin to close immediately. No code is required.

### Stopping the Door
1. Press the STOP button on the keypad at any time during door movement.
2. The door will stop immediately. No code is required.

### Setting a New Code
1. Press the CODE button on the receiver. The status light will flash continuously.
2. On the keypad, enter your new code (must be 4-12 digits long).
3. Press the UP key to confirm.
4. If accepted, you'll hear 3 ascending tones.

### Understanding Keypad Sounds
The keypad provides audible feedback to guide you:
- Ascending tone: Your code was authorised.
- Descending tone: Your code was not authorised.
- 3 low beeps: The system is locked out due to too many unauthorised attempts.
- 3 ascending tones: A new code has been successfully set.

### Important Notes
- The UP command always requires an authorised code for security reasons.
- DOWN and STOP commands can be used at any time without entering a code.
- If you enter too many incorrect codes, the system will lock out. Wait for the lockout period to end before trying again.
- Keep your code confidential to ensure the security of your property.

## How it works
When the user enters a code on the keypad, the numerical code and the control key (Up, Down, or Stop) are sent as an encrypted message to the receiver. The receiver decrypts the message and determines if the code is correct. It responds with the authorisation status and the number of attempts remaining. If too many unauthorised attempts are made the Receiver locks-out the system.


```mermaid
sequenceDiagram
    participant User
    participant Keypad
    participant Receiver
    
    User->>Keypad: Enter code + control key
    Keypad->>Keypad: Encrypt message
    Keypad->>Receiver: Send encrypted message
    Receiver->>Receiver: Decrypt message
    
    alt UP command
        Receiver->>Receiver: Check authorization
        alt authorised
            Receiver->>Keypad: Send authorization status
            Receiver->>Receiver: Execute UP command
        else Unauthorised
            Receiver->>Keypad: Send status + remaining attempts
            alt Too many attempts
                Receiver->>Receiver: Lock out system
            end
        end
    else DOWN or STOP command
        Receiver->>Receiver: Execute command without authorization
        Receiver->>Keypad: Send execution status
    end
```


## Keypad PCB Features

- Capacitive touch sensors - simplify assembly
- All components are on the back side
- Prototyping connections Vin, GND, Tx, Rx
- Gerbers pre-generated and LCSC components specified in BOM file


## Receiver PCB Features

- 3ch open-drain outputs
- Wide input voltage
- Pass-through terminals for daisy-chaining other controllers