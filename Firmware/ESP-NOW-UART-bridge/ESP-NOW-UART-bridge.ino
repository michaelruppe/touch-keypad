/*
    ESP-NOW UART Bridge
    Use the ESP32 as a wireless transceiver using ESP-NOW, bridging USB Serial <-> ESP-NOW
    All traffic is presented over the usb Serial Interface
    
    To properly visualize the data being sent, set the line ending in the Serial Monitor to "Both NL & CR".
*/

#include "ESP32_NOW_Serial.h"
#include "MacAddress.h"
#include "WiFi.h"
#include "esp_wifi.h"

// 0: AP mode, 1: Station mode
#define ESPNOW_WIFI_MODE_STATION 1

// Channel to be used by the ESP-NOW protocol
#define ESPNOW_WIFI_CHANNEL 1

#if ESPNOW_WIFI_MODE_STATION          // ESP-NOW using WiFi Station mode
#define ESPNOW_WIFI_MODE WIFI_STA     // WiFi Mode
#define ESPNOW_WIFI_IF   WIFI_IF_STA  // WiFi Interface
#else                                 // ESP-NOW using WiFi AP mode
#define ESPNOW_WIFI_MODE WIFI_AP      // WiFi Mode
#define ESPNOW_WIFI_IF   WIFI_IF_AP   // WiFi Interface
#endif

// Set the MAC address of the device that will receive the data
// const MacAddress peer_mac({0x8C, 0xBF, 0xEA, 0xCC, 0xB1, 0xF0});
const MacAddress peer_mac({0x54, 0x32, 0x04, 0x33, 0x3F, 0xE4});

ESP_NOW_Serial_Class NowSerial(peer_mac, ESPNOW_WIFI_CHANNEL, ESPNOW_WIFI_IF);


void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);


  Serial.print("WiFi Mode: ");
  Serial.println(ESPNOW_WIFI_MODE == WIFI_AP ? "AP" : "Station");
  WiFi.mode(ESPNOW_WIFI_MODE);

  Serial.print("Channel: ");
  Serial.println(ESPNOW_WIFI_CHANNEL);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  while (!(WiFi.STA.started() || WiFi.AP.started())) {
    delay(100);
  }

  Serial.print("MAC Address: ");
  Serial.println(ESPNOW_WIFI_MODE == WIFI_AP ? WiFi.softAPmacAddress() : WiFi.macAddress());

  // Start the ESP-NOW communication
  Serial.println("ESP-NOW communication starting...");
  NowSerial.begin(115200);
  Serial.println("ESP-NOW Serial bridge is now active.\n");
}

void loop() {
  // Handle ESP-NOW to Serial
  while (NowSerial.available()) {
    char c = NowSerial.read();
    Serial.write(c);  // Print to Serial
  }

  // Handle Serial to ESP-NOW
  while (Serial.available() && NowSerial.availableForWrite()) {
    char c = Serial.read();
    if (NowSerial.write(c) <= 0) {
      Serial.println("Failed to send data over ESP-NOW");
      break;
    }
    Serial1.write(c); // Also send to UART
  }

  delay(1);
}