#pragma once

#include <stdint.h>
#include <string.h>
#include "../register.h"
#include "descriptor.h"
#include "../util.h"

#define USBD_BUFFER_MAX_SIZE 64

// UsbDevice.ep_flags
#define USBD_USE_EP1_OUT 0b00000001
#define USBD_USE_EP1_IN  0b00000010
#define USBD_USE_EP2_OUT 0b00000100
#define USBD_USE_EP2_IN  0b00001000
#define USBD_USE_EP3_OUT 0b00010000
#define USBD_USE_EP3_IN  0b00100000

struct UsbDevice {
  uint8_t ep_flags;
  uint8_t (*setup)(struct SetupRequest*, void**, uint16_t*);
  uint8_t (*get_descriptor)(struct SetupRequest*, void**, uint16_t*);
  uint8_t address;
  uint8_t configuration_num;
  struct SetupRequest last_setup_req;
  void* ep0_sending_data_ptr;
  uint16_t ep0_sending_data_len;
};

struct UsbDevice __usb_device;

uint8_t __ep0_buffer[USBD_BUFFER_MAX_SIZE + 1];
uint8_t* ep0_buffer = __ep0_buffer;
uint8_t __ep1_buffer[USBD_BUFFER_MAX_SIZE + 1];
uint8_t* ep1_buffer = __ep1_buffer;
uint8_t __ep2_buffer[USBD_BUFFER_MAX_SIZE + 1];
uint8_t* ep2_buffer = __ep2_buffer;
uint8_t __ep3_buffer[USBD_BUFFER_MAX_SIZE + 1];
uint8_t* ep3_buffer = __ep3_buffer;

void usbd_bus_reset(void) {
  uint8_t use_ep1 = __usb_device.ep_flags & (USBD_USE_EP1_OUT | USBD_USE_EP1_IN);
  uint8_t use_ep2 = __usb_device.ep_flags & (USBD_USE_EP2_OUT | USBD_USE_EP2_IN);
  uint8_t use_ep3 = __usb_device.ep_flags & (USBD_USE_EP3_OUT | USBD_USE_EP3_IN);

  UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
  if (use_ep1) {
    UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
  }
  if (use_ep2) {
    UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
  }
  if (use_ep3) {
    UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
  }
  USB_DEV_AD = 0x00;
  DEBUG("bus reset\n");
}

void usbd_stall(void) {
  UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;
  DEBUG("stall\n");
}

void ep_data_send(uint8_t ep_num, uint8_t* data, uint8_t len) {
  switch (ep_num) {
    case 0:
      memcpy(ep0_buffer, data, len);
      UEP0_T_LEN = len;
      UEP0_CTRL = UEP0_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK;
      UEP0_CTRL ^= bUEP_T_TOG;
      break;
    case 1:
      memcpy(ep1_buffer, data, len);
      UEP1_T_LEN = len;
      UEP1_CTRL = UEP1_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK;
      UEP1_CTRL ^= bUEP_T_TOG;
      break;
    case 2:
      memcpy(ep2_buffer, data, len);
      UEP2_T_LEN = len;
      UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK;
      UEP2_CTRL ^= bUEP_T_TOG;
      break;
    case 3:
      memcpy(ep3_buffer, data, len);
      UEP3_T_LEN = len;
      UEP3_CTRL = UEP3_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK;
      UEP3_CTRL ^= bUEP_T_TOG;
      break;
  }
}

void ep0_send_first(void* ptr, uint16_t len, uint16_t max_len) {
  uint8_t tx_len = (len <= USBD_BUFFER_MAX_SIZE) ? len : USBD_BUFFER_MAX_SIZE;
  tx_len = (tx_len <= max_len) ? tx_len : max_len;
  memcpy(ep0_buffer, ptr, tx_len);
  UEP0_T_LEN = tx_len;
  UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
  __usb_device.ep0_sending_data_ptr = ptr + tx_len;
  __usb_device.ep0_sending_data_len = len - tx_len;
}

void ep0_send_next(uint16_t max_len) {
  void* ptr = __usb_device.ep0_sending_data_ptr;
  uint16_t len = __usb_device.ep0_sending_data_len;
  uint8_t tx_len = (len <= USBD_BUFFER_MAX_SIZE) ? len : USBD_BUFFER_MAX_SIZE;
  tx_len = (tx_len <= max_len) ? tx_len : max_len;
  memcpy(ep0_buffer, ptr, tx_len);
  UEP0_T_LEN = tx_len;
  UEP0_CTRL ^= (bUEP_R_TOG | bUEP_T_TOG);
  __usb_device.ep0_sending_data_ptr = ptr + tx_len;
  __usb_device.ep0_sending_data_len = len - tx_len;
}

void ep_in(uint8_t ep_num) {
  if (ep_num == 1) {
    UEP1_T_LEN = 0;
    UEP1_CTRL = UEP1_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK;
  } else if (ep_num == 2) {
    UEP2_T_LEN = 0;
    UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK;
  } else if (ep_num == 3) {
    UEP3_T_LEN = 0;
    UEP3_CTRL = UEP3_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK;
  }
}

uint8_t is_ep_ready(uint8_t ep_num) {
  switch (ep_num) {
    case 0:
      return (UEP0_CTRL & MASK_UEP_T_RES) != UEP_T_RES_ACK;
    case 1:
      return (UEP1_CTRL & MASK_UEP_T_RES) != UEP_T_RES_ACK;
    case 2:
      return (UEP2_CTRL & MASK_UEP_T_RES) != UEP_T_RES_ACK;
    case 3:
      return (UEP3_CTRL & MASK_UEP_T_RES) != UEP_T_RES_ACK;
  }
  return false;
}

void ep0_out(void) {
  uint8_t request_type = __usb_device.last_setup_req.bRequestType;
  if ((request_type & SETUP_REQUEST_DIR_MASK) == SETUP_REQUEST_DIR_IN) {
    UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
    return;
  }
  if ((request_type & SETUP_REQUEST_TYPE_MASK) == SETUP_REQUEST_TYPE_STANDARD) {
    UEP0_CTRL ^= bUEP_R_TOG;
    return;
  }
}

void ep0_in(void) {
  struct SetupRequest* last_setup_req = &(__usb_device.last_setup_req);
  if ((last_setup_req->bRequestType & SETUP_REQUEST_TYPE_MASK) == SETUP_REQUEST_TYPE_STANDARD) {
    switch (last_setup_req->bRequest) {
      case SET_ADDRESS:
        USB_DEV_AD = USB_DEV_AD & 0x80 | __usb_device.address;
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        DEBUG("set address: %d\n", USB_DEV_AD);
        break;
      case GET_DESCRIPTOR:
        ep0_send_next(last_setup_req->wLength);
        break;
      case SET_CONFIGURATION:
        ep0_send_next(last_setup_req->wLength);
        break;
      default:
        DEBUG("unknown standard request 0x%02X\n", last_setup_req->bRequest);
        break;
    }
  }
}

void ep0_setup(void) {
  UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_NAK | UEP_T_RES_NAK;

  uint8_t rx_len = USB_RX_LEN;
  if (rx_len != sizeof(struct SetupRequest)) { return; }
  struct SetupRequest* last_setup_req = &(__usb_device.last_setup_req);
  memcpy(last_setup_req, ep0_buffer, rx_len);

  if ((last_setup_req->bRequestType & SETUP_REQUEST_TYPE_MASK) == SETUP_REQUEST_TYPE_STANDARD) {
  switch (last_setup_req->bRequest) {
      case SET_ADDRESS:
        __usb_device.address = last_setup_req->wValue & 0xFF;
        ep0_send_first(NULL, 0, last_setup_req->wLength);
        break;
      case GET_DESCRIPTOR:
        void* desc_ptr;
        uint16_t desc_len;
        uint8_t is_error = __usb_device.get_descriptor(last_setup_req, &desc_ptr, &desc_len);
        if (is_error) {
          usbd_stall();
          return;
        }
        uint16_t max_len = last_setup_req->wLength;
        ep0_send_first(desc_ptr, desc_len, max_len);
        break;
      case GET_CONFIGURATION:
        void* num_ptr = &(__usb_device.configuration_num);
        ep0_send_first(num_ptr, 1, last_setup_req->wLength);
        break;
      case SET_CONFIGURATION:
        __usb_device.configuration_num = last_setup_req->wValue & 0xFF;
        ep0_send_first(NULL, 0, last_setup_req->wLength);
        DEBUG("set configuration %d\n", __usb_device.configuration_num);
        break;
      default:
        DEBUG("unknown standard request 0x%02X\n", last_setup_req->bRequest);
        break;
    }
  } else {
    void* data_ptr;
    uint16_t data_len;
    uint8_t is_error = __usb_device.setup(last_setup_req, &data_ptr, &data_len);
    if (is_error) {
      usbd_stall();
      return;
    }
    ep0_send_first(data_ptr, data_len, last_setup_req->wLength);
  }
}


struct UsbDevice* usbd_init(
  uint8_t ep_flags,
  uint8_t (*setup)(struct SetupRequest*, void**, uint16_t*),
  uint8_t (*get_descriptor)(struct SetupRequest*, void**, uint16_t*)
) {
  IE_USB = 0;
  USB_CTRL = 0;

  __usb_device.ep_flags = ep_flags;
  __usb_device.setup = setup;
  __usb_device.get_descriptor = get_descriptor;

  uint8_t use_ep1_out = __usb_device.ep_flags & USBD_USE_EP1_OUT;
  uint8_t use_ep1_in = __usb_device.ep_flags & USBD_USE_EP1_IN;
  uint8_t use_ep2_out = __usb_device.ep_flags & USBD_USE_EP2_OUT;
  uint8_t use_ep2_in = __usb_device.ep_flags & USBD_USE_EP2_IN;
  uint8_t use_ep3_out = __usb_device.ep_flags & USBD_USE_EP3_OUT;
  uint8_t use_ep3_in = __usb_device.ep_flags & USBD_USE_EP3_IN;

  // 16bit alignment
  if ((uint16_t)ep0_buffer & 1) { ep0_buffer++; }
  if ((uint16_t)ep1_buffer & 1) { ep1_buffer++; }
  if ((uint16_t)ep2_buffer & 1) { ep2_buffer++; }
  if ((uint16_t)ep3_buffer & 1) { ep3_buffer++; }

  UEP4_1_MOD = 0;
  UEP2_3_MOD = 0;
  if (use_ep1_out) {
    UEP4_1_MOD |= bUEP1_RX_EN;
    DEBUG("endpoint1 set out\n");
  } else if (use_ep1_in) {
    UEP4_1_MOD |= bUEP1_TX_EN;
    DEBUG("endpoint1 set in\n");
  }
  if (use_ep2_out) {
    UEP2_3_MOD |= bUEP2_RX_EN;
    DEBUG("endpoint2 set out\n");
  } else if (use_ep2_in) {
    UEP2_3_MOD |= bUEP2_TX_EN;
    DEBUG("endpoint2 set in\n");
  }
  if (use_ep3_out) {
    UEP2_3_MOD |= bUEP3_RX_EN;
    DEBUG("endpoint3 set out\n");
  } else if (use_ep3_in) {
    UEP2_3_MOD |= bUEP3_TX_EN;
    DEBUG("endpoint3 set in\n");
  }

  UEP0_DMA_H = (uint16_t)ep0_buffer >> 8;
  UEP0_DMA_L = (uint16_t)ep0_buffer & 0xFF;
  if (use_ep1_out || use_ep1_in) {
    UEP1_DMA_H = (uint16_t)ep1_buffer >> 8;
    UEP1_DMA_L = (uint16_t)ep1_buffer & 0xFF;
  }
  if (use_ep2_out || use_ep2_in) {
    UEP2_DMA_H = (uint16_t)ep2_buffer >> 8;
    UEP2_DMA_L = (uint16_t)ep2_buffer & 0xFF;
  }
  if (use_ep3_out || use_ep3_in) {
    UEP3_DMA_H = (uint16_t)ep3_buffer >> 8;
    UEP3_DMA_L = (uint16_t)ep3_buffer & 0xFF;
  }

  usbd_bus_reset();

  USB_DEV_AD = 0x00;
  UDEV_CTRL = bUD_DP_PD_DIS | bUD_DM_PD_DIS;
  USB_CTRL = UC_SYS_CTRL_PU | bUC_INT_BUSY | bUC_DMA_EN;
  UDEV_CTRL |= bUD_PORT_EN;
  USB_INT_FG = 0xFF;
  USB_INT_EN = bUIE_TRANSFER | bUIE_BUS_RST;
  IE_USB = 1;

  DEBUG("usb device initialized.\n");
  return &__usb_device;
}

void usbd_interrupt(void) __interrupt(8) __using(1) {
  if (UIF_TRANSFER) {
    if (U_IS_NAK) { return; }
    uint8_t interrupt_status = USB_INT_ST;
    switch (interrupt_status & 0b00111111) {
      case TOKEN_OUT | 0:
        ep0_out();
        break;
      case TOKEN_IN | 0:
        ep0_in();
        break;
      case TOKEN_SETUP | 0:
        ep0_setup();
        break;
      case TOKEN_OUT | 1:
      case TOKEN_OUT | 2:
      case TOKEN_OUT | 3:
        break;
      case TOKEN_IN | 1:
      case TOKEN_IN | 2:
      case TOKEN_IN | 3:
        ep_in(interrupt_status & 0x0F);
        break;
    }
    UIF_TRANSFER = 0;
  } else if (UIF_BUS_RST) {
    usbd_bus_reset();
    UIF_TRANSFER = 0;
    UIF_BUS_RST = 0;
  }
}
