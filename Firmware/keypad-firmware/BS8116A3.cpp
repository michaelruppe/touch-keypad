#include "BS8116A3.h"

BS8116A3::BS8116A3(uint8_t irqPin)
  : _irqPin(irqPin), _touchStatus(0), _previousTouchStatus(0) {}

void BS8116A3::begin() {
  Wire.begin();
  pinMode(_irqPin, INPUT_PULLUP);
}

bool BS8116A3::readTouchStatus() {
  if (digitalRead(_irqPin) == LOW) {
    _previousTouchStatus = _touchStatus;
    readTouchRegisters();
    return true;
  }
  return false;
}

uint16_t BS8116A3::getRawTouchedKeys() {
  return _touchStatus;
}

uint8_t* BS8116A3::getPressedKeys(int& count) {
  count = 0;
  for (int i = 0; i < 16; i++) {
    if (_touchStatus & (1 << i)) {
      _pressedKeys[count++] = i;
    }
  }
  return _pressedKeys;
}

uint8_t* BS8116A3::getNewlyPressedKeys(int& count) {
  count = 0;
  uint16_t newPresses = _touchStatus & ~_previousTouchStatus;
  for (int i = 0; i < 16; i++) {
    if (newPresses & (1 << i)) {
      _pressedKeys[count++] = i;
    }
  }
  return _pressedKeys;
}

void BS8116A3::setWakeUpKeys(uint16_t keys) {
  for (int i = 0; i < 16; i++) {
    uint8_t reg = REG_KEY1_THRESHOLD + i;
    uint8_t value = readRegister(reg);
    if (keys & (1 << i)) {
      value &= ~0x80;  // Enable wake-up
    } else {
      value |= 0x80;  // Disable wake-up
    }
    writeRegister(reg, value);
  }
}

void BS8116A3::setSensitivity(uint8_t key, uint8_t threshold) {
  if (key > 15) return;
  uint8_t reg = REG_KEY1_THRESHOLD + key;
  uint8_t value = readRegister(reg);
  value = (value & 0x80) | (threshold & 0x3F);
  writeRegister(reg, value);
}

void BS8116A3::enableLowPowerMode(bool enable) {
  uint8_t value = readRegister(REG_OPTION2);
  if (enable) {
    value |= 0x40;  // Set LSC bit
  } else {
    value &= ~0x40;  // Clear LSC bit
  }
  writeRegister(REG_OPTION2, value);
}

void BS8116A3::setIRQMode(bool oneShot) {
  uint8_t value = readRegister(REG_OPTION1);
  if (oneShot) {
    value |= 0x01;  // Set IRQ_OMS bit
  } else {
    value &= ~0x01;  // Clear IRQ_OMS bit
  }
  writeRegister(REG_OPTION1, value);
}

void BS8116A3::writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t BS8116A3::readRegister(uint8_t reg) {
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDRESS, 1);
  return Wire.read();
}

void BS8116A3::readTouchRegisters() {
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(REG_KEY_STATUS0);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDRESS, 2);
  uint8_t lowByte = Wire.read();
  uint8_t highByte = Wire.read();

  _touchStatus = (highByte << 8) | lowByte;
}

void BS8116A3::clearInterrupt() {
  writeRegister(REG_INTERRUPT_CLEAR, 0x00);  // Writing any value to this register clears the interrupt
}