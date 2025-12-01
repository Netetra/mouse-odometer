#include <ch559.h>

void main() {
  clock_init();

  PORT_CFG = 0b01001011;
  P2_DIR = 0b00100000;
  P2_PU =  0b11011111;

  PWM_CYCLE = 100;
  PWM_CTRL = 0b00000100;

  while (1) {
    if(!(P4_IN & (1 << 6))) { run_bootloader(); }

    for (uint8_t i = 0; i < 100; i++) {
      PWM_DATA2 = i;
      delay_ms(10);
    }
    for (uint8_t i = 0; i < 100; i++) {
      PWM_DATA2 = 100 - i;
      delay_ms(10);
    }
  }
}
