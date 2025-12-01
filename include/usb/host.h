#pragma once

#include <string.h>
#include "../clock.h"
#include "../register.h"
#include "../util.h"
#include "descriptor.h"

#define USBH_BUFFER_MAX_SIZE 64
#define USBH_TRANSFER_BUFFER_SIZE 512
#define EP0_DEFAULT_SIZE 8

#define USBH_USE_HUB0 0b00000001
#define USBH_USE_HUB1 0b00000010

#define AUTO_TOGGLE (bUH_R_TOG | bUH_R_AUTO_TOG | bUH_T_TOG | bUH_T_AUTO_TOG)

enum HubStatus {
  DEVICE_DISCONNECT, /* 接続されてない */
  DEVICE_CONNECTED,  /* 接続されてる(未設定) */
  DEVICE_READY,      /* 接続されてる(設定済み) */
  DEVICE_ERROR       /* 接続されてるが通信はしない */
};

struct Hub {
  enum HubStatus status;
  uint8_t address;
  bool is_full_speed;
  uint8_t ep0_size;
};

struct UsbHost {
  uint8_t flags;
  uint8_t (*connected_handler)(uint8_t hub, struct DeviceDesc* device_desc);
  void (*disconnected_handler)(uint8_t hub);
  void (*poll_handler)(uint8_t hub);
  struct Hub hub[2];
};

struct UsbHost __usb_host;

uint8_t __tx_buffer[USBH_BUFFER_MAX_SIZE + 1];
uint8_t* tx_buffer = __tx_buffer;
uint8_t __rx_buffer[USBH_BUFFER_MAX_SIZE + 1];
uint8_t* rx_buffer = __rx_buffer;

uint8_t transfer_buffer[USBH_TRANSFER_BUFFER_SIZE];

struct SetupRequest set_address_req = {
  .bRequestType = SETUP_REQUEST_DIR_OUT,
  .bRequest = SET_ADDRESS,
  .wValue = 0,
  .wIndex = 0,
  .wLength = 0
};

__code struct SetupRequest device_desc_req = {
  .bRequestType = SETUP_REQUEST_DIR_IN,
  .bRequest = GET_DESCRIPTOR,
  .wValue = DEVICE_DESCRIPTOR << 8,
  .wIndex = 0,
  .wLength = sizeof(struct DeviceDesc)
};


bool usbh_is_attach(uint8_t hub) {
  if (hub == 0) {
    return (USB_HUB_ST & bUHS_H0_ATTACH) != 0;
  } else if (hub == 1) {
    return (USB_HUB_ST & bUHS_H1_ATTACH) != 0;
  }
  return false;
}

void usbh_check_attach(uint8_t hub) {
  if (__usb_host.hub[hub].status == DEVICE_DISCONNECT && usbh_is_attach(hub)) {
    __usb_host.hub[hub].status = DEVICE_CONNECTED;
    __usb_host.hub[hub].address = 0x00;
    __usb_host.hub[hub].is_full_speed = false;
    __usb_host.hub[hub].ep0_size = EP0_DEFAULT_SIZE;
    DEBUG("hub%d: device connected\n", hub);
  }
}

void usbh_disable_port(uint8_t hub) {
  __usb_host.hub[hub].status = DEVICE_DISCONNECT;
  __usb_host.hub[hub].address = 0;
  if (hub == 0) {
    UHUB0_CTRL = 0x00;
  } else if (hub == 1) {
    UHUB1_CTRL = 0x00;
  }
  DEBUG("hub%d: port disabled\n", hub);
}

void usbh_check_detach(uint8_t hub) {
  if (__usb_host.hub[hub].status != DEVICE_DISCONNECT && !usbh_is_attach(hub)) {
    __usb_host.hub[hub].status = DEVICE_DISCONNECT;
    usbh_disable_port(hub);
    __usb_host.disconnected_handler(hub);
    DEBUG("hub%d: device disconnected\n", hub);
  }
}

inline void usbh_select_device(uint8_t address) {
  USB_DEV_AD = (USB_DEV_AD & bUDA_GP_BIT) | (address & 0x7F);
}

void usbh_set_speed(uint8_t is_full_speed) {
  if (is_full_speed) {
    USB_CTRL &= ~bUC_LOW_SPEED;
    UH_SETUP &= ~bUH_PRE_PID_EN;
  } else {
    USB_CTRL |= bUC_LOW_SPEED;
  }
}

bool usbh_enable_port(uint8_t hub) {
	if (hub == 0 && usbh_is_attach(0)) {
		if ((UHUB0_CTRL & bUH_PORT_EN) == 0x00) {
			if (USB_HUB_ST & bUHS_DM_LEVEL) {
        __usb_host.hub[0].is_full_speed = false;
				UHUB0_CTRL |= bUH_LOW_SPEED;
			} else {
        __usb_host.hub[0].is_full_speed = true;
      }
		}
		UHUB0_CTRL |= bUH_PORT_EN;
    DEBUG("hub0: port enabled\n");
		return false;
	} else if (hub == 1 && usbh_is_attach(1)) {
		if ((UHUB1_CTRL & bUH_PORT_EN) == 0x00) {
			if (USB_HUB_ST & bUHS_HM_LEVEL) {
        __usb_host.hub[1].is_full_speed = false;
				UHUB1_CTRL |= bUH_LOW_SPEED;
			} else {
        __usb_host.hub[1].is_full_speed = true;
      }
		}
		UHUB1_CTRL |= bUH_PORT_EN;
    DEBUG("hub1: port enabled\n");
		return false;
	}
	return true;
}

void usbh_select_port(uint8_t hub) {
  usbh_select_device(__usb_host.hub[hub].address);
  usbh_set_speed(__usb_host.hub[hub].is_full_speed);
}

void usbh_bus_reset(uint8_t hub) {
  usbh_select_device(0x00);
  usbh_set_speed(true);
  if (hub == 0) {
    UHUB0_CTRL = UHUB0_CTRL & ~bUH_LOW_SPEED | bUH_BUS_RESET;
    delay_ms(15);
    UHUB0_CTRL &= ~bUH_BUS_RESET;
  } else if (hub == 1) {
    UHUB1_CTRL = UHUB1_CTRL & ~bUH_LOW_SPEED | bUH_BUS_RESET;
    delay_ms(15);
    UHUB1_CTRL &= ~bUH_BUS_RESET;
  }
  delay_us(250);
  DEBUG("hub%d: bus reset\n", hub);
}

uint8_t usbh_transfer(uint8_t ep_pid, uint8_t tog, uint16_t timeout) {
    uint8_t	r;
    UH_RX_CTRL = UH_TX_CTRL = tog;
    for (uint8_t retries = 0; retries < 200; retries++) {
        UH_EP_PID = ep_pid;
        UIF_TRANSFER = 0;
        for (uint8_t i = 200; i != 0 && UIF_TRANSFER == 0; i--) { delay_us(1); }
        UH_EP_PID = 0x00;
        if ( UIF_TRANSFER == 0 ) { return true; }
        if ( UIF_TRANSFER ) {
            if ( U_TOG_OK ) { return false; }
            r = USB_INT_ST & MASK_UIS_H_RES;
            if ( r == PACKET_ID_STALL ) { return true; }
            if ( r == PACKET_ID_NAK ) {
                if ( timeout == 0 ) { return true; }
                if ( timeout < 0xFFFF ) { timeout --; }
                retries--;
            } else {
				switch (ep_pid >> 4) {
                case PACKET_ID_SETUP:
                case PACKET_ID_OUT:
                    if ( U_TOG_OK ) { return false; }
                    if ( r == PACKET_ID_ACK ) { return false; }
                    if ( r == PACKET_ID_STALL || r == PACKET_ID_NAK ) { return true; }
                    if ( r ) { return true; }
                    break;
                case PACKET_ID_IN:
                    if ( U_TOG_OK ) { return false; }
                    if ( tog ? r == PACKET_ID_DATA1 : r == PACKET_ID_DATA0 ) { return false; }
                    if ( r == PACKET_ID_STALL || r == PACKET_ID_NAK ) { return true; }
                    if ( r == PACKET_ID_DATA0 && r == PACKET_ID_DATA1 ) {
                    } else if (r) {
                        return true;
                    }
                    break;
                default:
                    return true;
                    break;
                }
			}
        } else {
            USB_INT_FG = 0xFF;
        }
        delay_us(15);
    }
	return true;
}

uint8_t usbh_transfer_control(
  uint8_t hub,
  struct SetupRequest* setup_req,
  uint8_t *buffer,
  uint16_t *len,
  uint16_t buffer_size
) {
  uint8_t error;
	if (len) { *len = 0; }
  usbh_select_port(hub);
  delay_us(200);
	memcpy(tx_buffer, setup_req, sizeof(struct SetupRequest));
	UH_TX_LEN = sizeof(struct SetupRequest);
	error = usbh_transfer((uint8_t)(PACKET_ID_SETUP << 4), 0, 10000);
	if (error) { return true; }
	UH_RX_CTRL = UH_TX_CTRL = bUH_R_TOG | bUH_R_AUTO_TOG | bUH_T_TOG | bUH_T_AUTO_TOG;
	UH_TX_LEN = 0x01;
	uint16_t remaining = setup_req->wLength;
	if (remaining && buffer) {
		if (setup_req->bRequestType & SETUP_REQUEST_DIR_IN) {
			while (remaining) {
				delay_us(300);
				error = usbh_transfer((uint8_t)(PACKET_ID_IN << 4), UH_RX_CTRL, 10000);
				if (error) { return true; }
				uint16_t rx_len = USB_RX_LEN < remaining ? USB_RX_LEN : remaining;
				remaining -= rx_len;
				if (len) { *len += rx_len; }
				memcpy(buffer, rx_buffer, rx_len);
				buffer += rx_len;
				if (USB_RX_LEN == 0 || (USB_RX_LEN < __usb_host.hub[hub].ep0_size )) { break; }
			}
			UH_TX_LEN = 0;
		} else {
			// TODO: rework this tx_buffer overwritten
			while (remaining) {
				delay_us(200);
				UH_TX_LEN = remaining >= __usb_host.hub[hub].ep0_size ? __usb_host.hub[hub].ep0_size : remaining;
				//memcpy(tx_buffer, buffer, UH_TX_LEN);
				buffer += UH_TX_LEN;
				error = usbh_transfer(PACKET_ID_OUT << 4, UH_TX_CTRL, 10000);
				if (error) { return true; }
				remaining -= UH_TX_LEN;
				if (len) { *len += UH_TX_LEN; }
			}
		}
	}
	delay_us(200);
	error = usbh_transfer((UH_TX_LEN ? PACKET_ID_IN << 4 : PACKET_ID_OUT << 4), bUH_R_TOG | bUH_T_TOG, 10000);
	if (error) { return true; }
	if (UH_TX_LEN == 0) { return false; }
	if (USB_RX_LEN == 0) { return false; }
	return true;
}

uint8_t usbh_transfer_in(
  uint8_t hub,
  uint8_t ep,
  uint8_t* buffer,
  uint16_t* len,
  uint16_t timeout
) {
  uint8_t ep_pid = (PACKET_ID_IN << 4) | ep;
  usbh_select_port(hub);
  UH_TX_LEN = 0;
  uint8_t error = usbh_transfer(ep_pid, AUTO_TOGGLE, timeout);
  if (error) { return true; }
  *len = USB_RX_LEN;
  memcpy(buffer, rx_buffer, USB_RX_LEN);
  return false;
}

void usbh_device_init(uint8_t hub) {
  uint8_t error;
  uint8_t i;
  usbh_disable_port(hub);
  for (uint8_t retry = 0; retry < 10; retry++) {
    delay_ms(200);
    usbh_bus_reset(hub);
    for (i = 0; i < 100; i++) {
      delay_ms(1);
      if (!usbh_enable_port(hub)) { break; }
    }
    if (i == 100) {
      usbh_disable_port(hub);
      DEBUG("hub%d: could not enable port\n", hub);
      continue;
    }
    __usb_host.hub[hub].address = 0;

    uint16_t len;
    error = usbh_transfer_control(hub, &device_desc_req, transfer_buffer, &len, USBH_TRANSFER_BUFFER_SIZE);
    if (error) { continue; }
    if (len != sizeof(struct DeviceDesc)) { continue; }
    struct DeviceDesc device_desc;
    memcpy(&device_desc, transfer_buffer, sizeof(struct DeviceDesc));
    DEBUG("hub%d: VID 0x%04X , PID 0x%04X\n", hub, device_desc.idVendor, device_desc.idProduct);

    uint8_t address = hub + 1;
    set_address_req.wValue = address;
    error = usbh_transfer_control(hub, &set_address_req, 0, 0, 0);
    if (error) { continue; }
    __usb_host.hub[hub].address = address;
    DEBUG("hub%d: address 0x%02X\n", hub, __usb_host.hub[hub].address);
    delay_ms(100);

    error = __usb_host.connected_handler(hub, &device_desc);
    if (error) { continue; }

    __usb_host.hub[hub].status = DEVICE_READY;
    DEBUG("hub%d: device initialized\n", hub);
    return;
  }
  __usb_host.hub[hub].status = DEVICE_ERROR;
  usbh_set_speed(true);
  DEBUG("hub%d: device could not initialized\n", hub);
}

void usbh_poll(uint8_t hub) {
  if (hub != 0 && hub != 1) { return; }
  if (hub == 0 && (__usb_host.flags & USBH_USE_HUB0) == 0) { return; }
  if (hub == 1 && (__usb_host.flags & USBH_USE_HUB1) == 0) { return; }
  usbh_check_detach(hub);
  switch (__usb_host.hub[hub].status) {
    case DEVICE_DISCONNECT:
      usbh_check_attach(hub);
      break;
    case DEVICE_CONNECTED:
      usbh_device_init(hub);
      break;
    case DEVICE_READY:
      __usb_host.poll_handler(hub);
      break;
    case DEVICE_ERROR:
      // nop
      break;
  }
}

struct UsbHost* usbh_init(
  uint8_t flags,
  uint8_t (*connected_handler)(uint8_t hub, struct DeviceDesc* device_desc),
  void (*disconnected_handler)(uint8_t hub),
  void (*poll_handler)(uint8_t hub)
) {
  __usb_host.flags = flags;
  __usb_host.connected_handler = connected_handler;
  __usb_host.disconnected_handler = disconnected_handler;
  __usb_host.poll_handler = poll_handler;
  for (uint8_t i = 0; i < 2; i++) {
    __usb_host.hub[i].address = 0;
    __usb_host.hub[i].status = DEVICE_DISCONNECT;
    __usb_host.hub[i].is_full_speed = false;
    __usb_host.hub[i].ep0_size = EP0_DEFAULT_SIZE;
  }
  // 16bit alignment
  if ((uint16_t)tx_buffer & 1) { tx_buffer++; }
  if ((uint16_t)rx_buffer & 1) { rx_buffer++; }

  IE_USB = 0;
	USB_CTRL = bUC_HOST_MODE;
	USB_DEV_AD = 0x00;
	UH_EP_MOD = bUH_EP_TX_EN | bUH_EP_RX_EN ;
	UH_RX_DMA_L = (uint16_t)rx_buffer & 0xFF;
  UH_RX_DMA_H = (uint16_t)rx_buffer >> 8;
	UH_TX_DMA_L = (uint16_t)tx_buffer & 0xFF;
  UH_TX_DMA_H = (uint16_t)tx_buffer >> 8;
	UH_RX_CTRL = 0x00;
	UH_TX_CTRL = 0x00;
	USB_CTRL = bUC_HOST_MODE | bUC_INT_BUSY | bUC_DMA_EN;
	UH_SETUP = bUH_SOF_EN;
	USB_INT_FG = 0xFF;
  if (__usb_host.flags & USBH_USE_HUB0) {
    UHUB1_CTRL = bUH_RECV_DIS | bUH_DP_PD_DIS | bUH_DM_PD_DIS;
    usbh_disable_port(0);
    usbh_poll(0);
    usbh_poll(0);
    usbh_poll(0);
  } else {
    UHUB0_CTRL = bUH_RECV_DIS | bUH_DP_PD_DIS | bUH_DM_PD_DIS;
  }
  if (__usb_host.flags & USBH_USE_HUB1) {
    usbh_disable_port(1);
    usbh_poll(1);
    usbh_poll(1);
    usbh_poll(1);
  } else {
    UHUB1_CTRL = bUH_RECV_DIS | bUH_DP_PD_DIS | bUH_DM_PD_DIS;
  }
	USB_INT_EN = bUIE_TRANSFER | bUIE_DETECT;
  // IE_USB = 1;
  DEBUG("usb host initialized.\n");

  return &__usb_host;
}

// void usbh_interrupt(void) __interrupt(8) __using(1) {}