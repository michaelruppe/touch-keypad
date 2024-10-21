#include "config.h"
#include <esp_now.h>
#include <WiFi.h>
#include <EEPROM.h>

enum ctrlCode {
  codeUp = 10,
  codeDown = 11,
  codeStop = 12
};

// Updated structure to receive data
typedef struct struct_rx_message {
  uint8_t codeLen;
  ctrlCode ctrlButton;
  uint8_t code[12];
} struct_rx_message;

// Structure to send status message
typedef struct struct_tx_message {
  uint8_t status;             // 0: Not authorized, 1: Authorized, 2: New code set, 3: Locked out
  uint8_t remainingAttempts;  // 0-3, where 0 means locked out
} struct_tx_message;

// Create message structures
struct_rx_message rxData;
struct_tx_message txData;

// Variables for code setting and validation
uint8_t validCode[12];
uint8_t validCodeLen = 0;
bool isCodeSettingMode = false;

// Lockout variables
uint8_t remainingAttempts = MAX_ATTEMPTS;
unsigned long lockoutStartTime = 0;
bool lockedOut = false;

// Function to save code to EEPROM
void saveCodeToEEPROM() {
  EEPROM.write(CODE_LENGTH_ADDR, validCodeLen);
  for (int i = 0; i < validCodeLen; i++) {
    EEPROM.write(CODE_START_ADDR + i, validCode[i]);
  }
  EEPROM.commit();
}

// Function to load code from EEPROM
void loadCodeFromEEPROM() {
  validCodeLen = EEPROM.read(CODE_LENGTH_ADDR);
  if (validCodeLen > 12) validCodeLen = 0;  // Safety check
  for (int i = 0; i < validCodeLen; i++) {
    validCode[i] = EEPROM.read(CODE_START_ADDR + i);
  }
}

// Function to print MAC address on Serial Monitor
void printMAC(const uint8_t *mac_addr) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
}

// Function to compare received code with valid code
bool isCodeValid(const uint8_t *receivedCode, uint8_t receivedCodeLen) {
  if (receivedCodeLen != validCodeLen) return false;
  for (int i = 0; i < validCodeLen; i++) {
    if (receivedCode[i] != validCode[i]) return false;
  }
  return true;
}

// Function to send status message
void sendStatusMessage() {
  esp_err_t result = esp_now_send(masterMacAddress, (uint8_t *)&txData, sizeof(txData));
  if (result != ESP_OK) {
    Serial.println("Error sending status message");
  }
}

// Callback function when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Updated callback function executed when data is received
void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len) {
  const uint8_t *mac_addr = esp_now_info->src_addr;
  Serial.print("Packet received from: ");
  printMAC(mac_addr);

  memcpy(&rxData, incomingData, sizeof(rxData));

  // Print the message contents
  Serial.print("Received numbers: ");
  for (int i = 0; i < rxData.codeLen; i++) {
    Serial.print(rxData.code[i]);
  }
  Serial.println();


  if (isCodeSettingMode) {
    // Save the received code as the new valid code
    memcpy(validCode, rxData.code, rxData.codeLen);
    validCodeLen = rxData.codeLen;
    saveCodeToEEPROM();  // Save the new code to EEPROM
    Serial.println("New code set and saved to EEPROM");
    isCodeSettingMode = false;
    txData.status = 2;
    txData.remainingAttempts = MAX_ATTEMPTS;
    sendStatusMessage();
  } else {
    bool codeValid = isCodeValid(rxData.code, rxData.codeLen);
    switch (rxData.ctrlButton) {
      case codeUp:
        Serial.println("Up");
        if (codeValid && !lockedOut) {
          Serial.println("Code authorized");
          Serial.print("Control code: ");
          txData.status = 1;
          remainingAttempts = MAX_ATTEMPTS;
        } else {
          if (!codeValid) {
            Serial.println("Invalid code");
            txData.status = 0;
            if (remainingAttempts > 0) remainingAttempts--;  // Don't underflow an unsigned integer! :I
            if (remainingAttempts == 0) {
              lockedOut = true;

              lockoutStartTime = millis();
              Serial.println("Entering lockout state");
            }
            if (lockedOut) {
              txData.status = 3;
            }
          }
        }
        break;
      case codeDown:
        Serial.println("Down");
        txData.status = 1;  // Down commands are always authorised
        break;
      case codeStop:
        Serial.println("Stop");
        txData.status = 1;  // Stop commands are always authorised
        break;
      default:
        Serial.println("Unknown");
    }

    txData.remainingAttempts = remainingAttempts;
    sendStatusMessage();

    Serial.println("--------------------");
  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Load the valid code from EEPROM
  loadCodeFromEEPROM();
  Serial.println("Loaded code from EEPROM");

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("There was an error initializing ESP-NOW");
    return;
  }

  // Set the PMK key
  esp_now_set_pmk((uint8_t *)PMK_KEY_STR);

  // Register the master as peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, masterMacAddress, 6);
  peerInfo.channel = 0;
  // Setting the master device LMK key
  for (uint8_t i = 0; i < 16; i++) {
    peerInfo.lmk[i] = LMK_KEY_STR[i];
  }
  // Set encryption to true
  peerInfo.encrypt = true;

  // Add master as peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Register callbacks
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  Serial.println("ESP-NOW Receiver initialized. Send 'reset' to enter code setting mode.");
}

void loop() {
  // Check if 'reset' command is received over Serial
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.equalsIgnoreCase("reset")) {
      isCodeSettingMode = true;
      Serial.println("Entered code setting mode. Waiting for new code...");
    }
  }

  // Check if lockout period has ended (rollover-safe version)
  if (remainingAttempts == 0) {
    unsigned long currentTime = millis();
    if (currentTime - lockoutStartTime >= LOCKOUT_DURATION || currentTime < lockoutStartTime) {
      remainingAttempts = MAX_ATTEMPTS;
      lockedOut = false;
      Serial.println("Lockout period ended");
    }
  }
}