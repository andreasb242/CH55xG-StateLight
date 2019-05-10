/**
 * Hardware initialisation
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#include "inc.h"

// USB BUFFER -----------------------------------------------------------------

// Endpoint 0 OUT & IN buffer, must be an even address
__xdata __at (0x0000) uint8_t  Ep0Buffer[DEFAULT_ENDP0_SIZE];

// Endpoint 1 upload buffer
__xdata __at (0x0040) uint8_t  Ep1Buffer[DEFAULT_ENDP1_SIZE];

// Endpoint 2 IN & OUT buffer, must be an even address
__xdata __at (0x0080) uint8_t  Ep2Buffer[2 * MAX_PACKET_SIZE];

// ----------------------------------------------------------------------------

/**
 * USB device mode configuration
 */
void USBDeviceCfg() {
	// Clear USB control register
	USB_CTRL = 0x00;

	// This bit is the device selection mode
	USB_CTRL &= ~bUC_HOST_MODE;

	// USB device and internal pull-up enable, automatically return to NAK before interrupt flag is cleared during interrupt
	USB_CTRL |= bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;

	// Device address initialization
	USB_DEV_AD = 0x00;

	// USB_CTRL |= bUC_LOW_SPEED;
	// Choose low speed 1.5M mode
	// UDEV_CTRL |= bUD_LOW_SPEED;

	USB_CTRL &= ~bUC_LOW_SPEED;
	// Select full speed 12M mode, default mode
	UDEV_CTRL &= ~bUD_LOW_SPEED;

	// Disable DP/DM pull-down resistor
	UDEV_CTRL = bUD_PD_DIS;

	// Enable physical port
	UDEV_CTRL |= bUD_PORT_EN;
}

/**
 * Enable USB Endpoint
 */
void USBDeviceEndPointCfg() {
	// Endpoint 1 sends the data transfer address
	UEP1_DMA = (uint16_t) Ep1Buffer;

	// Endpoint 2 IN data transfer address
	UEP2_DMA = (uint16_t) Ep2Buffer;

	// Endpoint 2/3 Single Buffer Transceiver Enable
	UEP2_3_MOD = 0xCC;

	// Endpoint 2 automatically flips the sync flag, IN transaction returns NAK, OUT returns ACK
	UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;

	// Endpoint 1 automatically flips the sync flag, and IN transaction returns NAK
	UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;

	// Endpoint 0 data transfer address
	UEP0_DMA = (uint16_t) Ep0Buffer;

	// Endpoint 1 upload buffer; endpoint 0 single 64 byte send and receive buffer
	UEP4_1_MOD = 0X40;

	// Manual flip, OUT transaction returns ACK, IN transaction returns NAK
	UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
}

/**
 * USB device mode interrupt initialization
 */
void USBDeviceIntCfg() {
	// Enable device hang interrupt
	USB_INT_EN |= bUIE_SUSPEND;

	// Enable USB transfer completion interrupt
	USB_INT_EN |= bUIE_TRANSFER;

	// Enable device mode USB bus reset interrupt
	USB_INT_EN |= bUIE_BUS_RST;

	// Clear interrupt flag
	USB_INT_FG |= 0x1F;

	// Enable USB interrupt
	IE_USB = 1;

	// Enable global interrupts
	EA = 1;
}

/**
 * Print string to Serial 0
 */
void print(const char* str) {
	while(*str) {
		CH554UART0SendByte(*str);
		str++;
	}
}

#define BOOT_ADDR  0x3800

/**
 * This function provided a way to access the internal bootloader
 */
void jumpToBootloader() {
	USB_INT_EN = 0;
	USB_CTRL = 0x06;

	mDelaymS(100);

	// Disable all interrupts
	EA = 0;

	__asm
		LJMP BOOT_ADDR /* Jump to bootloader */
	__endasm;
	while(1);
}


