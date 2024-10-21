#include <esp_now.h>
#include <WiFi.h>
#include <BS8116A3.h>

// REPLACE WITH THE RECEIVER'S MAC Address
uint8_t receiverAddress[] = { 0x54, 0x32, 0x04, 0x33, 0x3F, 0xE4 };

// PMK and LMK keys
static const char *PMK_KEY_STR = "0123456789abcdef";
static const char *LMK_KEY_STR = "314159abcdefghij";

// Touch sensor pins and constants
const int IRQ_PIN = D10;
const int PIEZO_PIN = D0;
const int PIEZO_FREQ = 2000;
const unsigned long BIP_DURATION = 50;

// Define your keypad layout
const int NUM_KEYS = 15;
const char KEY_MAP[NUM_KEYS] = { 'D', 0, '7', '8', '5', '4', '1', '2', '3', '6', '9', 'U', 0, '0', 'S' };

volatile bool touchInterruptOccurred = false;
BS8116A3 touchSensor(IRQ_PIN);

unsigned long bipStartTime = 0;
bool bipActive = false;

enum ctrlCode {
  codeUp = 10,
  codeDown = 11,
  codeStop = 12
};

// Structure to send data
typedef struct struct_tx_message {
  uint8_t codeLen;
  ctrlCode ctrlButton;
  uint8_t code[12];
} struct_tx_message;

// Structure to receive status message
typedef struct struct_rx_message {
  uint8_t status;
  uint8_t remainingAttempts;
} struct_rx_message;

struct_tx_message txData;
struct_rx_message rxData;

// Variable to save peerInfo
esp_now_peer_info_t peerInfo;

void IRAM_ATTR touchInterrupt() {
  touchInterruptOccurred = true;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len) {
  memcpy(&rxData, incomingData, sizeof(rxData));

  Serial.println("\nReceived status message:");
  switch (rxData.status) {
    case 0:
      Serial.println("Status: Not authorized");
      playUnauthorisedSound();
      break;
    case 1:
      Serial.println("Status: Authorized");
      playSuccessSound();
      break;
    case 2:
      Serial.println("Status: New code set");
      playNewCodeSound();
      break;
    case 3:
      Serial.println("Status: Locked out");
      playLockoutSound();
      break;
    default:
      Serial.println("Status: Unknown");
  }

  if (rxData.status != 3) {
    Serial.print("Remaining attempts: ");
    Serial.println(rxData.remainingAttempts);
  } else {
    Serial.println("Device is locked out. Please wait.");
    playLockoutSound();
  }
}

void startBip(unsigned int frequency, unsigned int duration) {
  if (frequency > 0) tone(PIEZO_PIN, frequency);
  else noTone(PIEZO_PIN);
  bipStartTime = millis();
  bipActive = true;
}

void updateBip() {
  if (bipActive && (millis() - bipStartTime >= BIP_DURATION)) {
    noTone(PIEZO_PIN);
    bipActive = false;
  }
}

void playSuccessSound(void) {
  startBip(1000, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }
  startBip(2000, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }
}

void playUnauthorisedSound(void) {
  startBip(500, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }
  startBip(250, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }
}

void playLockoutSound(void) {
  startBip(500, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }
  startBip(0, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }

  startBip(500, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }
  startBip(0, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }

  startBip(500, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }
  startBip(0, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }
}

void playNewCodeSound(void) {
  startBip(1000, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }
  startBip(0, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }

  startBip(1500, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }
  startBip(0, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }

  startBip(2000, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }
  startBip(0, 100);
  while (bipActive) {
    updateBip();
    delay(1);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Start");

  // Touch sensor setup
  touchSensor.begin();
  touchSensor.setWakeUpKeys(0xFFFF);
  for (int i = 0; i < NUM_KEYS; i++) {
    touchSensor.setSensitivity(i, 1);
  }
  touchSensor.enableLowPowerMode(true);
  touchSensor.setIRQMode(true);

  pinMode(PIEZO_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), touchInterrupt, FALLING);

  // ESP-NOW setup
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_pmk((uint8_t *)PMK_KEY_STR);

  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  for (uint8_t i = 0; i < 16; i++) {
    peerInfo.lmk[i] = LMK_KEY_STR[i];
  }
  peerInfo.encrypt = true;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Ready to receive input from Serial or Keypad.");
  Serial.println("For Serial: Enter up to 12 digits followed by u (up), d (down), or s (stop).");
}

void processAndSendMessage(String input, bool fromSerial) {
  char controlChar = input.charAt(input.length() - 1);
  String numberString = input.substring(0, input.length() - 1);

  if (numberString.length() > 12) {
    Serial.println("Error: Input too long. Maximum 12 digits allowed.");
    return;
  }

  memset(txData.code, 0, sizeof(txData.code));
  txData.codeLen = 0;

  for (int i = 0; i < numberString.length(); i++) {
    if (isdigit(numberString.charAt(i))) {
      txData.code[txData.codeLen++] = numberString.charAt(i) - '0';
    } else if (!fromSerial) {
      txData.code[txData.codeLen++] = numberString.charAt(i) - '0';
    } else {
      Serial.println("Error: Invalid character in input. Only digits are allowed.");
      return;
    }
  }

  switch (controlChar) {
    case 'u':
    case 'U':
      txData.ctrlButton = codeUp;
      break;
    case 'd':
    case 'D':
      txData.ctrlButton = codeDown;
      break;
    case 's':
    case 'S':
      txData.ctrlButton = codeStop;
      break;
    default:
      Serial.println("Invalid control character. Use u, d, or s.");
      return;
  }

  esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)&txData, sizeof(txData));
  if (result == ESP_OK) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Error sending the data");
  }

  Serial.print("Sending: Length ");
  Serial.print(txData.codeLen);
  Serial.print(", Control ");
  Serial.print(txData.ctrlButton);
  Serial.print(", Code ");
  for (int i = 0; i < txData.codeLen; i++) {
    Serial.print(txData.code[i]);
  }
  Serial.println();
}

void loop() {
  updateBip();

  // Handle Serial input
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0) {
      processAndSendMessage(input, true);
    }
  }

  // Handle Keypad input
  if (touchInterruptOccurred) {
    touchInterruptOccurred = false;
    if (touchSensor.readTouchStatus()) {
      int count;
      uint8_t *newlyPressedKeys = touchSensor.getNewlyPressedKeys(count);

      if (count > 0) {

        // Play audible bips for numeric keys only. Control keys will have their audio behaviour determined by the outome from the receiver (authorised, unauthorised)
        for (int i = 0; i < count; i++) {
          if (newlyPressedKeys[i] < NUM_KEYS) {
            char key = KEY_MAP[newlyPressedKeys[i]];
            if (key != 'u' && key != 's' && key != 'd') startBip(2000, 50);
          }
        }



        static String keypadInput = "";
        for (int i = 0; i < count; i++) {
          if (newlyPressedKeys[i] < NUM_KEYS) {
            char key = KEY_MAP[newlyPressedKeys[i]];
            keypadInput += key;
            Serial.print(key);

            if (key == 'U' || key == 'D' || key == 'S') {
              Serial.println();
              processAndSendMessage(keypadInput, false);
              keypadInput = "";
            }
          }
        }
      }
    }
    touchSensor.clearInterrupt();
  }
}