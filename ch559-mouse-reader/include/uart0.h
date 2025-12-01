#pragma once

#include <stdint.h>
#include "register.h"

// alt pins: rx = P0.2 tx = P0.3
void uart0_init(uint32_t baudrate, uint8_t use_alt_pin) {
  if (use_alt_pin) {
    PORT_CFG |= (1 << 0);
    P0_DIR |= (1 << 3);
    P0_PU |= (1 << 3) | (1 << 2);
    PIN_FUNC |= (1 << 4);
  }

  SM0 = 0;
	SM1 = 1;
	SM2 = 0;
	REN = 1;
  T2CON &= ~((1 << 5) | (1 << 4));
  PCON |= (1 << 7);
  TMOD = TMOD & ~((1 << 7) | (1 << 6) | (1 << 4)) | (1 << 5);
  T2MOD |= (1 << 7) | (1 << 5);
  uint32_t x = ((((uint32_t)SYSTEM_FREQ / 8) / baudrate + 1) / 2);
  TH1 = (256 - x) & 0xFF;
  TR1 = 1;
	TI = 1;
}
