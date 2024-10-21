#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH THE RECEIVER'S MAC Address
uint8_t receiverAddress[] = {0x54, 0x32, 0x04, 0x33, 0x3F, 0xE4};

// PMK and LMK keys
static const char* PMK_KEY_STR = "0123456789abcdef";
static const char* LMK_KEY_STR = "314159abcdefghij";


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
    uint8_t status;  // 0: Not authorized, 1: Authorized, 2: New code set, 3: Locked out
    uint8_t remainingAttempts;  // 0-3, where 0 means locked out
} struct_rx_message;

struct_tx_message txData;
struct_rx_message rxData;

// Variable to save peerInfo
esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback function executed when data is received
void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len) {
  memcpy(&rxData, incomingData, sizeof(rxData));
  
  Serial.println("\nReceived status message:");
  switch(rxData.status) {
    case 0:
      Serial.println("Status: Not authorized");
      break;
    case 1:
      Serial.println("Status: Authorized");
      break;
    case 2:
      Serial.println("Status: New code set");
      break;
    case 3:
      Serial.println("Status: Locked out");
      break;
    default:
      Serial.println("Status: Unknown");
  }
  
  if (rxData.status != 3) {
    Serial.print("Remaining attempts: ");
    Serial.println(rxData.remainingAttempts);
  } else {
    Serial.println("Device is locked out. Please wait.");
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("There was an error initializing ESP-NOW");
    return;
  }
  
  // Set PMK key
  esp_now_set_pmk((uint8_t *)PMK_KEY_STR);
  
  // Register the receiver board as peer
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  //Set the receiver device LMK key
  for (uint8_t i = 0; i < 16; i++) {
    peerInfo.lmk[i] = LMK_KEY_STR[i];
  }
  // Set encryption to true
  peerInfo.encrypt = true;
  
  // Add receiver as peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Register callbacks
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Ready to receive input. Enter up to 12 digits followed by u (up), d (down), or s (stop).");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() > 0) {
      char controlChar = input.charAt(input.length() - 1);
      String numberString = input.substring(0, input.length() - 1);
      
      // Check if the number string is too long
      if (numberString.length() > 12) {
        Serial.println("Error: Input too long. Maximum 12 digits allowed.");
        return;
      }

      // Clear previous data
      memset(txData.code, 0, sizeof(txData.code));
      txData.codeLen = 0;

      // Parse numbers
      for (int i = 0; i < numberString.length(); i++) {
        if (isdigit(numberString.charAt(i))) {
          txData.code[txData.codeLen++] = numberString.charAt(i) - '0';
        } else {
          Serial.println("Error: Invalid character in input. Only digits are allowed.");
          return;
        }
      }

      // Set control code
      switch (controlChar) {
        case 'u':
          txData.ctrlButton = codeUp;
          break;
        case 'd':
          txData.ctrlButton = codeDown;
          break;
        case 's':
          txData.ctrlButton = codeStop;
          break;
        default:
          Serial.println("Invalid control character. Use u, d, or s.");
          return;
      }

      // Send message via ESP-NOW
      esp_err_t result = esp_now_send(receiverAddress, (uint8_t *) &txData, sizeof(txData));
      if (result == ESP_OK) {
        Serial.println("Data sent successfully");
      } else {
        Serial.println("Error sending the data");
      }

      // Print the data being sent
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
  }
}