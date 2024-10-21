#ifndef BS8116A3_H
#define BS8116A3_H

#include <Arduino.h>
#include <Wire.h>

class BS8116A3 {
public:
  BS8116A3(uint8_t irqPin);
  void begin();
  bool readTouchStatus();
  uint16_t getRawTouchedKeys();
  uint8_t* getPressedKeys(int& count);
  uint8_t* getNewlyPressedKeys(int& count);
  void setWakeUpKeys(uint16_t keys);
  void setSensitivity(uint8_t key, uint8_t threshold);
  void enableLowPowerMode(bool enable);
  void setIRQMode(bool oneShot);
  void clearInterrupt();  // New method to clear the interrupt

private:
  uint8_t _irqPin;
  uint16_t _touchStatus;
  uint16_t _previousTouchStatus;
  uint8_t _pressedKeys[16];  // Max 16 keys can be pressed simultaneously

  void writeRegister(uint8_t reg, uint8_t value);
  uint8_t readRegister(uint8_t reg);
  void readTouchRegisters();

  static const uint8_t I2C_ADDRESS = 0x50;
  static const uint8_t REG_KEY_STATUS0 = 0x08;
  static const uint8_t REG_KEY_STATUS1 = 0x09;
  static const uint8_t REG_OPTION1 = 0xB0;
  static const uint8_t REG_OPTION2 = 0xB4;
  static const uint8_t REG_KEY1_THRESHOLD = 0xB5;
  static const uint8_t REG_INTERRUPT_CLEAR = 0x0A;  // New register for clearing interrupts
};

#endif