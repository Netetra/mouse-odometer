#define USE_UART0_STDIO
// #define ENABLE_DEBUG_LOG

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ch559.h>
#include <usb/device.h>

struct Descriptors {
  struct ConfigurationDesc configuration_desc;
  struct InterfaceDesc interface_desc;
  struct HIDDesc hid_desc;
  struct EndpointDesc endpoint_desc;
};

__code struct DeviceDesc device_desc = {
  18,                // bLength
  DEVICE_DESCRIPTOR, // bDescriptorType: DEVICE
  0x0110,            // bcdUSB: USB 1.1
  0x00,              // bDeviceClass: 0
  0x00,              // bDeviceSubClass
  0x00,              // bDeviceProtocol
  64,                // bMaxPacketSize0: 64 bytes
  0x1a86,            // idVendor
  0x5678,            // idProduct
  0x0100,            // bcdDevice
  1,                 // iManufacturer
  2,                 // iProduct
  0,                 // iSerialNumber
  1                  // bNumConfigurations
};

__code struct Descriptors descriptors = {
  {
    9,                        // bLength
    CONFIGURATION_DESCRIPTOR, // bDescriptorType: CONFIGURATION
    34,                       // wTotalLength
    0x01,                     // bNumInterfaces
    0x01,                     // bConfigurationValue
    0x00,                     // iConfiguration
    0x80,                     // bmAttributes
    0x32                      // bMaxPower: 100 mA
  }, {
    9,                        // bLength
    INTERFACE_DESCRIPTOR,     // bDescriptorType: INTERFACE
    0,                        // bInterfaceNumber
    0,                        // bAlternateSetting
    1,                        // bNumEndpoints
    3,                        // bInterfaceClass: HID
    1,                        // bInterfaceSubClass: Boot Interface Subclass
    2,                        // bInterfaceProtocol: Mouse
    0                         // iInterface
  }, {
    9,                        // bLength
    HID_DESCRIPTOR,           // bDescriptorType: HID
    0x0111,                   // bcdHID: 1.11
    0,                        // bCountryCode
    1,                        // bNumDescriptors
    HID_REPORT_DESCRIPTOR,    // bDescriptorType(HID): Report
    50                        // wDescriptorLength
  }, {
    7,                        // bLength
    ENDPOINT_DESCRIPTOR,      // bDescriptorType: ENDPOINT
    0x81,                     // bEndpointAddress: EP1 IN
    INTERRUPT_TRANSFER,       // bmAttributes
    3,                        // wMaxPacketSize
    10                         // bInterval
  }
};

__code uint8_t hid_report_desc[] = {
  0x05, 0x01,        // Usage Page (Generic Desktop)
  0x09, 0x02,        // Usage (Mouse)
  0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,      //   Usage (Pointer)
    0xA1, 0x00,      //   Collection (Physical)
      0x05, 0x09,    //     Usage Page (Buttons)
      0x19, 0x01,    //     Usage Minimum (Button 1)
      0x29, 0x03,    //     Usage Maximum (Button 3)
      0x15, 0x00,    //     Logical Minimum (0)
      0x25, 0x01,    //     Logical Maximum (1)
      0x95, 0x03,    //     Report Count (3)
      0x75, 0x01,    //     Report Size (1)
      0x81, 0x02,    //     Input (Data,Var,Abs)
      0x95, 0x01,    //     Report Count (1)
      0x75, 0x05,    //     Report Size (5)
      0x81, 0x03,    //     Input (Const,Var,Abs)
      0x05, 0x01,    //     Usage Page (Generic Desktop)
      0x09, 0x30,    //     Usage (X)
      0x09, 0x31,    //     Usage (Y)
      0x15, 0x81,    //     Logical Minimum (-127)
      0x25, 0x7F,    //     Logical Maximum (127)
      0x75, 0x08,    //     Report Size (8)
      0x95, 0x02,    //     Report Count (2)
      0x81, 0x06,    //     Input (Data,Var,Rel)
    0xC0,            //   End Collection
  0xC0               // End Collection
};

__code uint8_t string_desc0[] = { // LANGID
  0x06, 0x03,
  0x09, 0x04,
  0x09, 0x04
};

__code uint8_t string_desc1[] = { // iManufacturer
  24,  0x03,
  'n', 0x00,
  'e', 0x00,
  't', 0x00,
  'e', 0x00,
  't', 0x00,
  'r', 0x00,
  'a', 0x00,
  '.', 0x00,
  'd', 0x00,
  'e', 0x00,
  'v', 0x00,
};

__code uint8_t string_desc2[] = { // iProduct
  12,  0x03,
  'M', 0x00,
  'o', 0x00,
  'u', 0x00,
  's', 0x00,
  'e', 0x00,
};

int8_t report[] = { 0, 0, 0 };

uint8_t get_descriptor(struct SetupRequest* last_setup_req, void** ptr, uint16_t* len) {
  uint8_t desc_type = (last_setup_req->wValue >> 8) & 0xFF;
  uint8_t index = last_setup_req->wValue & 0xFF;
  switch (desc_type) {
    case DEVICE_DESCRIPTOR:
      DEBUG("requested device descriptor\n");
      *ptr = &device_desc;
      *len = sizeof(struct DeviceDesc);
      break;
    case CONFIGURATION_DESCRIPTOR:
      DEBUG("requested configuration descriptor index: %d\n", index);
      switch (index) {
        case 0:
          *ptr = &descriptors;
          *len = sizeof(struct Descriptors);
          break;
        default:
          return 1;
      }
      break;
    case STRING_DESCRIPTOR:
      DEBUG("requested string descriptor index: %d\n", index);
      switch (index) {
        case 0:
          *ptr = string_desc0;
          *len = sizeof(string_desc0);
          break;
        case 1:
          *ptr = string_desc1;
          *len = sizeof(string_desc1);
          break;
        case 2:
          *ptr = string_desc2;
          *len = sizeof(string_desc2);
          break;
        default:
          DEBUG("unknown string descriptor index: %d\n");
          return 1;
      }
      break;
    case HID_REPORT_DESCRIPTOR:
      DEBUG("requested hid report descriptor\n");
      *ptr = hid_report_desc;
      *len = sizeof(hid_report_desc);
      break;
    default:
      DEBUG("unknown descriptor 0x%02X", desc_type);
      return 1;
  }
  return 0;
}

uint8_t setup(struct SetupRequest* last_setup_req, void** ptr, uint16_t* len) {
  if ((last_setup_req->bRequestType & SETUP_REQUEST_TYPE_MASK) != SETUP_REQUEST_TYPE_CLASS) {
    return;
  }
  switch (last_setup_req->bRequest) {
    case HID_GET_REPORT:
      DEBUG("requested report\n");
      *ptr = report;
      *len = 3;
      break;
    case HID_SET_IDLE:
      DEBUG("set idle\n");
      *ptr = NULL;
      *len = 0;
      break;
    case HID_SET_PROTOCOL:
      DEBUG("set protocol\n");
      *ptr = NULL;
      *len = 0;
      break;
    default:
      DEBUG("unknown class request 0x%02X", last_setup_req->bRequest);
      return 1;
  }
  return 0;
}

void main(void) {
  clock_init();
  uart0_init(115200, true);
  uint8_t ep_flags = USBD_USE_EP1_IN;
  struct UsbDevice* device = usbd_init(ep_flags, setup, get_descriptor);
  EA = 1;

  float degree = 0;

  while (true) {
    if(!(P4_IN & (1 << 6))) { run_bootloader(); }
    if (is_ep_ready(1)) {
      report[1] = (int8_t)(2. * cosf((PI / 180.) * degree));
      report[2] = (int8_t)(2. * sinf((PI / 180.) * degree));
      ep_data_send(1, report, 3);
      degree += 1;
      if (degree >= 360) {
        degree = 0;
      }
    }
  }
}
