#pragma once

#include <stdint.h>
#include "util.h"
#include "register.h"
#include "system.h"

#ifndef USE_EXTERNAL_OSC
#define USE_EXTERNAL_OSC false
#endif

#ifndef SYSTEM_FREQ
#define SYSTEM_FREQ 48000000
#endif

void delay_us(uint16_t n) {
  while (n) {
    ++ SAFE_MOD;  // 2 Fsys cycles, for higher Fsys, add operation here
#ifdef	SYSTEM_FREQ
#if		SYSTEM_FREQ >= 14000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 16000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 18000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 20000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 22000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 24000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 26000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 28000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 30000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 32000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 34000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 36000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 38000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 40000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 42000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 44000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 46000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 48000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 50000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 52000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 54000000
		++ SAFE_MOD;
#endif
#if		SYSTEM_FREQ >= 56000000
		++ SAFE_MOD;
#endif
#endif
		--n;
  }
}

void delay_ms(uint16_t n) {
  while (n) {
    delay_us(1000);
    --n;
  }
}

void use_external_osc(void) {
  __enter_safe_mode();
  CLOCK_CFG |= 1 << 6;
  __exit_safe_mode();

  delay_ms(50);

  __enter_safe_mode();
  CLOCK_CFG &= ~(1 << 7);
  __exit_safe_mode();
}

void __clock_configuration(uint8_t pll_mul, uint8_t fusb_div, uint8_t fsys_div) {
  uint8_t tmp = (fusb_div << 5) | pll_mul;

  __enter_safe_mode();
  PLL_CFG = tmp;
  CLOCK_CFG &= 0b11100000;
  CLOCK_CFG |= fsys_div;
  __exit_safe_mode();
}

void clock_init(void) {
  #if USE_EXTERNAL_OSC
  #error External OSC is not supported.
  #else
  #if SYSTEM_FREQ == 48000000
    __clock_configuration(24, 6, 6);
  #else
  #error The system frequency is not supported.
  #endif
  #endif
}
