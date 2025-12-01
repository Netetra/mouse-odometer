#define USE_UART0_STDIO
#define ENABLE_DEBUG_LOG

#include <stdio.h>
#include <ch559.h>
#include <usb/host.h>

#define VENDOR_ID  0x30FA
#define PRODUCT_ID 0x0300
#define ENDPOINT   1

struct Report {
  uint8_t left: 1;
  uint8_t right: 1;
  uint8_t wheel: 1;
  uint8_t padding: 3;
  uint32_t x_y_scroll;
};

__code struct SetupRequest set_configuration_req = {
  .bRequestType = SETUP_REQUEST_DIR_OUT,
  .bRequest = SET_CONFIGURATION,
  .wValue = 1,
  .wIndex = 0,
  .wLength = 0
};

uint8_t buffer[64];

uint8_t connected_handler(uint8_t hub, struct DeviceDesc* device_desc) {
  if (device_desc->idVendor != VENDOR_ID || device_desc->idProduct != PRODUCT_ID) { return true; }
  uint8_t error = usbh_transfer_control(hub, &set_configuration_req, 0, 0, 0);
  if (error) { return true; }
  DEBUG("hub%d: set configuration 1\n", hub);
  return false;
}

void disconnected_handler(uint8_t hub) {}

void poll_handler(uint8_t hub) {
  uint16_t len;
  uint8_t error = usbh_transfer_in(hub, 1, buffer, &len, 1);
  if (error) { return; }
  if (len != sizeof(struct Report)) { return; }
  struct Report* report = (struct Report*)buffer;
  int16_t x_12bit = report->x_y_scroll & 0x00000FFF;
  int16_t x = (x_12bit & 0x800) ? (x_12bit | 0xF000) : (x_12bit & 0x0FFF);
  int16_t y_12bit = (report->x_y_scroll & 0x00FFF000) >> 12;
  int16_t y = (y_12bit & 0x800) ? (y_12bit | 0xF000) : (y_12bit & 0x0FFF);
  int8_t scroll = (report->x_y_scroll & 0xFF000000) >> 24;
  printf("hub%d: ", hub);
  printf("left %d, right %d, wheel %d, ", report->left, report->right, report->wheel);
  printf("x %4d, y %4d, scroll %4d\n", x, y, scroll);
}

void main(void) {
  clock_init();
  uart0_init(115200, true);
  uint8_t flags = USBH_USE_HUB0 | USBH_USE_HUB1;
  struct UsbHost* host = usbh_init(flags, connected_handler, disconnected_handler, poll_handler);
  // EA = 1;

  while (true) {
    if(!(P4_IN & (1 << 6))) { run_bootloader(); }
    usbh_poll(0);
    usbh_poll(1);
  }
}
