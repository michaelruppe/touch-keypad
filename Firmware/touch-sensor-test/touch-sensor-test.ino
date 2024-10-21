#include <BS8116A3.h>

const int IRQ_PIN = D10;
const int PIEZO_PIN = D0;  // Piezo connected to D0

const int PIEZO_FREQ = 2000;            // Frequency in Hz
const unsigned long BIP_DURATION = 50;  // Bip duration in milliseconds

// Define your keypad layout
const int NUM_KEYS = 15;
const char KEY_MAP[NUM_KEYS] = { 'D', 0, '7', '8', '5', '4', '1', '2', '3', '6', '9', 'U', 0, '0', 'S' };

volatile bool touchInterruptOccurred = false;
BS8116A3 touchSensor(IRQ_PIN);

unsigned long bipStartTime = 0;
bool bipActive = false;

void IRAM_ATTR touchInterrupt() {
  touchInterruptOccurred = true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Start");
  touchSensor.begin();
  touchSensor.setWakeUpKeys(0xFFFF);  // Enable all keys for wake-up

  // Set higher sensitivity for all keys
  for (int i = 0; i < NUM_KEYS; i++) {
    touchSensor.setSensitivity(i, 1);  // Set sensitivity to 16 (more sensitive)
  }

  touchSensor.enableLowPowerMode(true);
  touchSensor.setIRQMode(true);  // Use one-shot mode for interrupts

  pinMode(PIEZO_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), touchInterrupt, FALLING);
}

void startBip() {
  tone(PIEZO_PIN, PIEZO_FREQ);
  bipStartTime = millis();
  bipActive = true;
}

void updateBip() {
  if (bipActive && (millis() - bipStartTime >= BIP_DURATION)) {
    noTone(PIEZO_PIN);  // Turn off the piezo
    bipActive = false;
  }
}

void loop() {
  updateBip();  // Check and update bip state

  if (touchInterruptOccurred) {
    touchInterruptOccurred = false;

    if (touchSensor.readTouchStatus()) {
      int count;
      uint8_t* newlyPressedKeys = touchSensor.getNewlyPressedKeys(count);

      if (count > 0) {
        startBip();  // Start the bip for any new key press

        Serial.print("Newly pressed keys: ");
        for (int i = 0; i < count; i++) {
          if (newlyPressedKeys[i] < NUM_KEYS) {
            Serial.print(KEY_MAP[newlyPressedKeys[i]]);
            Serial.print(" ");
          } else {
            Serial.print("Unknown ");
          }
        }
        Serial.println();
      }
    }

    touchSensor.clearInterrupt();  // Clear the interrupt flag on the touch sensor
  }
}