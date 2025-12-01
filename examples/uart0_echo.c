#define USE_UART0_STDIO

#include <ch559.h>
#include <stdio.h>

void main() {
  clock_init();
  uart0_init(115200, true);

  char buffer[256];
  uint16_t ptr = 0;

  while (true) {
    if(!(P4_IN & (1 << 6))) { run_bootloader(); }

    while (true) {
      char c = getchar();
      buffer[ptr] = c;
      ptr++;
      if (c == '\n') {
        buffer[ptr] = '\0';
        ptr = 0;
        break;
      }
    }
    printf("Receive: %s", buffer);
  }
}
