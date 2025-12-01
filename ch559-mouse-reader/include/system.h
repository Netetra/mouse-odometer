#pragma once

#include <stdint.h>

void (*run_bootloader)(void) = 0xF400;

inline void __enter_safe_mode(void) {
  SAFE_MOD = 0x55;
  SAFE_MOD = 0xAA;
}

inline void __exit_safe_mode(void) {
  SAFE_MOD = 0xFF;
}

inline uint16_t chip_id(void) {
  return 0x500 | CHIP_ID;
}
