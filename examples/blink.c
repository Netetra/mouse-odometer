#include <ch559.h>

void main() {
  clock_init();

  PORT_CFG = 0b01001011;
  P2_DIR = 0b00000001;
  P2 = 0x00;

  while (true) {
    if(!(P4_IN & (1 << 6))) { run_bootloader(); }

    P2 = (!(P2 & 0b00000001));
    delay_ms(500);
  }
}
