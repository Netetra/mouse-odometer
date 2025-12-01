#pragma once

#include <stdint.h>
#include "register.h"

#define bool uint8_t
#define true 1
#define false 0

#if defined(USE_UART0_STDIO)
int putchar(int c) {
  while (TI == 0) {}
  SBUF = c & 0xFF;
  TI = 0;
  return c;
}

int getchar(void) {
  while (RI == 0) {}
  RI = 0;
  return SBUF;
}
#elif defined(USE_UART1_STDIO)

#else
int putchar(int c) { return c; }
int getchar(void) {}
#endif

#if defined(ENABLE_DEBUG_LOG)
#define DEBUG(...) printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif
