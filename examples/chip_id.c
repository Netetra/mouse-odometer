#define USE_UART0_STDIO

#include <ch559.h>
#include <stdio.h>

void main() {
  clock_init();
  uart0_init(115200, true);

  printf("Chip: CH%03X\n", chip_id());

  while (true) {
    if(!(P4_IN & (1 << 6))) { run_bootloader(); }
    delay_ms(1000);
  }
}
