#pragma once

#include <stdint.h>

// PID
#define PACKET_ID_OUT   0x01
#define PACKET_ID_ACK   0x02
#define PACKET_ID_IN    0x09
#define PACKET_ID_SETUP 0x0D
#define PACKET_ID_SOF   0x05
#define PACKET_ID_NAK   0x0A
#define PACKET_ID_STALL 0x0E
#define PACKET_ID_DATA0 0x03
#define PACKET_ID_DATA1 0x0B

// bDescriptorType
#define DEVICE_DESCRIPTOR        0x01
#define CONFIGURATION_DESCRIPTOR 0x02
#define STRING_DESCRIPTOR        0x03
#define INTERFACE_DESCRIPTOR     0x04
#define ENDPOINT_DESCRIPTOR      0x05

#define HID_DESCRIPTOR           0x21
#define HID_REPORT_DESCRIPTOR    0x22

// EndpointDesc.bmAttributes
#define INTERRUPT_TRANSFER 0x03

// SetupRequest.bRequest
#define GET_STATUS        0x00
#define CLEAR_FEATURE     0x01
#define SET_FEATURE       0x03
#define SET_ADDRESS       0x05
#define GET_DESCRIPTOR    0x06
#define SET_DESCRIPTOR    0x07
#define GET_CONFIGURATION 0x08
#define SET_CONFIGURATION 0x09
#define GET_INTERFACE     0x10
#define SET_INTERFACE     0x11
#define SYNC_FRAME        0x12

#define HID_GET_REPORT    0x01
#define HID_SET_IDLE      0x0A
#define HID_SET_PROTOCOL  0x0B

// SetupRequest.bRequestType
#define SETUP_REQUEST_DIR_MASK      0b10000000
#define SETUP_REQUEST_DIR_IN        0b10000000
#define SETUP_REQUEST_DIR_OUT       0b00000000
#define SETUP_REQUEST_TYPE_MASK     0b01100000
#define SETUP_REQUEST_TYPE_STANDARD 0b00000000
#define SETUP_REQUEST_TYPE_CLASS    0b00100000
#define SETUP_REQUEST_TYPE_VENDOR   0b01000000
#define REQUEST_TARGET_DEVICE       0x00
#define REQUEST_TARGET_INTERFACE    0x01
#define REQUEST_TARGET_ENDPOINT     0x02
#define REQUEST_TARGET_OTHER        0x03

struct SetupRequest {
  uint8_t bRequestType;
  uint8_t bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
};

struct DeviceDesc {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t bcdUSB;
  uint8_t bDeviceClass;
  uint8_t bDeviceSubClass;
  uint8_t bDeviceProtocol;
  uint8_t bMaxPacketSize0;
  uint16_t idVendor;
  uint16_t idProduct;
  uint16_t bcdDevice;
  uint8_t iManufacturer;
  uint8_t iProduct;
  uint8_t iSerialNumber;
  uint8_t bNumConfigurations;
};

struct ConfigurationDesc {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t wTotalLength;
  uint8_t bNumInterfaces;
  uint8_t bConfigurationValue;
  uint8_t iConfiguration;
  uint8_t bmAttributes;
  uint8_t bMaxPower;
};

struct InterfaceDesc {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bNumEndpoints;
  uint8_t bInterfaceClass;
  uint8_t bInterfaceSubClass;
  uint8_t bInterfaceProtocol;
  uint8_t iInterface;
};

struct HIDDesc {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t bcdHID;
  uint8_t bCountryCode;
  uint8_t bNumDescriptors;
  uint8_t bReportDescriptorType;
  uint16_t wDescriptorLength;
};

struct EndpointDesc {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bEndpointAddress;
  uint8_t bmAttributes;
  uint16_t wMaxPacketSize;
  uint8_t bInterval;
};
