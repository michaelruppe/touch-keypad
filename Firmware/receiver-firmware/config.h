// Config.h
#pragma once

uint8_t masterMacAddress[] = {0x8C, 0xBF, 0xEA, 0xCC, 0xB1, 0xF0};

// Encryption Keys
#define PMK_KEY_STR "0123456789abcdef"
#define LMK_KEY_STR "314159abcdefghij"

// EEPROM addresses
#define EEPROM_SIZE 64
#define CODE_LENGTH_ADDR 0
#define CODE_START_ADDR 1

// Incorrect code behaviour
#define MAX_ATTEMPTS 3
#define LOCKOUT_DURATION 60000  // 1 minute in milliseconds