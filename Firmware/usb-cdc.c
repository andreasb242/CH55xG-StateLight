/**
 * USB CDC Implementation
 * Based on the example from
 * https://github.com/Blinkinlabs/ch554_sdcc/tree/master/examples/usb_device_cdc_i2c
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#include "inc.h"
#include "usb-cdc.h"
#include "hardware.h"
#include "logic.h"
#include "usb-descriptor.h"

/**
 * Setup length
 */
uint16_t g_SetupLen;

/**
 * Last setup request
 */
uint8_t g_SetupReq;

/**
 * USB Configuration, can be read and written by the host
 */
uint8_t g_UsbConfig;

/**
 * Pointer to the current configuration data
 */
const uint8_t* g_pDescr;

/**
 * Use the received data as Setup request
 */
#define UsbSetupBuf	((PUSB_SETUP_REQ)Ep0Buffer)

// Configures DTE rate, stop-bits, parity, and number-of-character
#define SET_LINE_CODING		0X20

// This request allows the host to find out the currently configured line coding.
#define GET_LINE_CODING		0X21

// This request generates RS-232/V.24 style control signals
#define SET_CONTROL_LINE_STATE	0X22

/**
 * Baud rate, not needed for Virtual USB without hardware Serial
 * But may this is needed for another project, therefore this
 * will not be removed
 */
uint32_t Baud = 0;

// Serial receive buffer size
#define UART_REV_LEN  64

// Serial receive buffer
__idata uint8_t Receive_Uart_Buf[UART_REV_LEN];

// The circular buffer write pointer, the bus reset needs to be initialized to 0
volatile __idata uint8_t Uart_Input_Point = 0;

// The circular buffer fetches the pointer, and the bus reset needs to be initialized to 0.
volatile __idata uint8_t Uart_Output_Point = 0;

// Current buffer remaining bytes to be fetched
volatile __idata uint8_t UartByteCount = 0;

// Data received on behalf of the USB endpoint
volatile __idata uint8_t USBByteCount = 0;

// Data pointer
volatile __idata uint8_t USBBufOutPoint = 0;

// Upload endpoint is busy flag
volatile __idata uint8_t UpPoint2_Busy = 0;

/**
 * Handle USB Reset
 */
inline void usbResetInterrupt() {
	UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
	UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;
	UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
	USB_DEV_AD = 0x00;
	UIF_SUSPEND = 0;
	UIF_TRANSFER = 0;

	// Clear interrupt flag
	UIF_BUS_RST = 0;

	// Circular buffer input pointer
	Uart_Input_Point = 0;

	// Circular buffer read pointer
	Uart_Output_Point = 0;

	// Current buffer remaining bytes to be fetched
	UartByteCount = 0;

	// Length received by the USB endpoint
	USBByteCount = 0;

	// Clear configuration value
	g_UsbConfig = 0;
	UpPoint2_Busy = 0;
}

/**
 * Handle USB Suspend
 */
inline void usbWakeupSuspendInterrupt() {
	if (USB_MIS_ST & bUMS_SUSPEND) {
		while (XBUS_AUX & bUART0_TX) {
			; // Waiting for transmission to complete
		}
		SAFE_MOD = 0x55;
		SAFE_MOD = 0xAA;

		// USB or RXD0/1 can be woken up when there is a signal
		WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO | bWAK_RXD1_LO;

		// Sleep
		PCON |= PD;
		SAFE_MOD = 0x55;
		SAFE_MOD = 0xAA;
		WAKE_CTRL = 0x00;
	}
}

/**
 * Process USB Standard setup request
 *
 * @return Length
 */
inline uint8_t processUsbDescriptionRequest() {
	uint8_t len = 0;

	switch (UsbSetupBuf->wValueH) {
	// Device descriptor
	case 1:
		// Send the device descriptor to the buffer to be sent
		g_pDescr = DevDesc;
		len = sizeof(DevDesc);
		break;

	// Configuration descriptor
	case 2:
		// Send the device descriptor to the buffer to be sent
		g_pDescr = CfgDesc;
		len = sizeof(CfgDesc);
		break;

	case 3:
		if (UsbSetupBuf->wValueL == 0) {
			g_pDescr = g_DescriptorLanguage;
			len = sizeof(g_DescriptorLanguage);
		} else if (UsbSetupBuf->wValueL == 1) {
			g_pDescr = g_DescriptorManufacturer;
			len = sizeof(g_DescriptorManufacturer);
		} else if (UsbSetupBuf->wValueL == 2) {
			g_pDescr = g_DescriptorProduct;
			len = sizeof(g_DescriptorProduct);
		} else {
			g_pDescr = g_DescriptorSerial;
			len = sizeof(g_DescriptorSerial);
		}
		break;

	default:
		// Unsupported command or error
		len = 0xff;
		break;
	}

	if (g_SetupLen > len) {
		// Limit total length
		g_SetupLen = len;
	}

	// This transmission length
	len = g_SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : g_SetupLen;

	// Load upload data
	memcpy(Ep0Buffer, g_pDescr, len);
	g_SetupLen -= len;
	g_pDescr += len;

	return len;
}

/**
 * Process USB Standard setup clear request
 *
 * @return Length
 */
inline uint8_t processStandardSetupClearRequest() {
	uint8_t len = 0;

	// Clear device
	if (( UsbSetupBuf->bRequestType & 0x1F) == USB_REQ_RECIP_DEVICE) {
		if ((((uint16_t) UsbSetupBuf->wValueH << 8) | UsbSetupBuf->wValueL) == 0x01) {
			if (CfgDesc[7] & 0x20) {
				// wake
			} else {
				// operation failed
				len = 0xff;
			}
		} else {
			// operation failed
			len = 0xff;
		}

	} else if (( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) {
		switch ( UsbSetupBuf->wIndexL) {
		case 0x83:
			UEP3_CTRL = UEP3_CTRL & ~( bUEP_T_TOG | MASK_UEP_T_RES) | UEP_T_RES_NAK;
			break;
		case 0x03:
			UEP3_CTRL = UEP3_CTRL & ~( bUEP_R_TOG | MASK_UEP_R_RES) | UEP_R_RES_ACK;
			break;
		case 0x82:
			UEP2_CTRL = UEP2_CTRL & ~( bUEP_T_TOG | MASK_UEP_T_RES) | UEP_T_RES_NAK;
			break;
		case 0x02:
			UEP2_CTRL = UEP2_CTRL & ~( bUEP_R_TOG | MASK_UEP_R_RES) | UEP_R_RES_ACK;
			break;
		case 0x81:
			UEP1_CTRL = UEP1_CTRL & ~( bUEP_T_TOG | MASK_UEP_T_RES) | UEP_T_RES_NAK;
			break;
		case 0x01:
			UEP1_CTRL = UEP1_CTRL & ~( bUEP_R_TOG | MASK_UEP_R_RES) | UEP_R_RES_ACK;
			break;

		default:
			// Unsupported endpoint
			len = 0xff;
			break;
		}

	} else {
		// No endpoints are not supported
		len = 0xff;
	}
	return len;
}

/**
 * Process USB Standard setup set feature request
 *
 * @return Length
 */
inline uint8_t processStandardSetupSetFeatureRequest() {
	// defaults to operation failed
	uint8_t len = 0xff;

	// Setting up the device
	if (( UsbSetupBuf->bRequestType & 0x1F) == USB_REQ_RECIP_DEVICE) {
		if ((((uint16_t) UsbSetupBuf->wValueH << 8) | UsbSetupBuf->wValueL) == 0x01) {
			if (CfgDesc[7] & 0x20) {
				// Sleep
				while (XBUS_AUX & bUART0_TX) {
					;	// Waiting for transmission to complete
				}

				SAFE_MOD = 0x55;
				SAFE_MOD = 0xAA;

				// USB or RXD0 / 1 can be woken up when there is a signal
				WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO | bWAK_RXD1_LO;

				// Sleep
				PCON |= PD;
				SAFE_MOD = 0x55;
				SAFE_MOD = 0xAA;
				WAKE_CTRL = 0x00;

				// result success
				len = 0;
			}
		}

		// Set endpoint
	} else if (( UsbSetupBuf->bRequestType & 0x1F) == USB_REQ_RECIP_ENDP) {
		if ((((uint16_t) UsbSetupBuf->wValueH << 8) | UsbSetupBuf->wValueL) == 0x00) {
			// result success
			len = 0;

			switch (((uint16_t) UsbSetupBuf->wIndexH << 8) | UsbSetupBuf->wIndexL) {
			case 0x83:
				// Set endpoint 3 IN STALL
				UEP3_CTRL = UEP3_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;
				break;

			case 0x03:
				// Set Endpoint 3 OUT Stall
				UEP3_CTRL = UEP3_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;
				break;

			case 0x82:
				// Set endpoint 2 IN STALL
				UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;
				break;

			case 0x02:
				// Set Endpoint 2 OUT Stall
				UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;
				break;

			case 0x81:
				// Set endpoint 1 IN STALL
				UEP1_CTRL = UEP1_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;
				break;

			case 0x01:
				// Set endpoint 1 OUT Stall
				UEP1_CTRL = UEP1_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;
				break;

			default:
				// operation failed
				len = 0xff;
				break;
			}
		}
	}

	return len;
}

/**
 * Process USB Standard setup request
 *
 * @return Length
 */
inline uint8_t processStandardSetupRequest() {
	uint8_t len = 0;

	// Request code
	switch (g_SetupReq)
	{
	case USB_GET_DESCRIPTOR:
		len = processUsbDescriptionRequest();
		break;

	case USB_SET_ADDRESS:
		// Staging USB device address
		g_SetupLen = UsbSetupBuf->wValueL;
		break;

	case USB_GET_CONFIGURATION:
		Ep0Buffer[0] = g_UsbConfig;
		if (g_SetupLen >= 1) {
			len = 1;
		}
		break;

	case USB_SET_CONFIGURATION:
		g_UsbConfig = UsbSetupBuf->wValueL;
		break;

	case USB_GET_INTERFACE:
		break;

		//Clear Feature
	case USB_CLEAR_FEATURE:
		len = processStandardSetupClearRequest();
		break;

	// Set Feature
	case USB_SET_FEATURE:
		len = processStandardSetupSetFeatureRequest();
		break;

	case USB_GET_STATUS:
		Ep0Buffer[0] = 0x00;
		Ep0Buffer[1] = 0x00;
		if (g_SetupLen >= 2) {
			len = 2;
		} else {
			len = g_SetupLen;
		}
		break;

	// operation failed
	default:
		len = 0xff;
		break;
	}

	return len;
}

/**
 * Process USB Non-Standard setup request
 *
 * @return Length
 */
inline uint8_t processNonStandardSetupRequest() {
	uint8_t len = 0;

	switch (g_SetupReq) {
	// This request allows the host to find out the currently configured line coding.
	case GET_LINE_CODING:
		g_pDescr = LineCoding;
		len = sizeof(LineCoding);
		// This transmission length
		len = g_SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : g_SetupLen;
		memcpy(Ep0Buffer, g_pDescr, len);
		g_SetupLen -= len;
		g_pDescr += len;
		break;

	// This request generates RS-232/V.24 style control signals
	case SET_CONTROL_LINE_STATE:
		break;

	case SET_LINE_CODING:
		break;

	default:
		// Command not supported
		len = 0xff;
		break;
	}

	return len;
}

/**
 * Process the Setup request, and return the length of the packet
 *
 * @return Length
 */
uint8_t processSetupRequest() {
	// The default is success and upload 0 length
	uint8_t len = 0;

	g_SetupLen = ((uint16_t) UsbSetupBuf->wLengthH << 8) | (UsbSetupBuf->wLengthL);
	g_SetupReq = UsbSetupBuf->bRequest;

	// Non-standard request
	if (( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD) {
		len = processNonStandardSetupRequest();
	} else { // Standard request
		len = processStandardSetupRequest();
	}

	return len;
}

/**
 * USB Setup Handler
 */
inline void usbSetupInterrupt() {
	uint8_t len;
	if (USB_RX_LEN == (sizeof(USB_SETUP_REQ))) {
		len = processSetupRequest();
	} else {
		// Wrong packet length
		len = 0xff;
	}

	if (len == 0xff) {
		g_SetupReq = 0xff;

		// STALL
		UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;

	} else if (len <= DEFAULT_ENDP0_SIZE) {
		//Upload data or status stage returns 0 length package

		UEP0_T_LEN = len;
		// The default packet is DATA1, which returns a response ACK.
		UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;

	} else {
		// Although it has not yet reached the status stage,
		// it uploads 0 length data packets in advance to prevent
		// the host from entering the status stage in advance.
		UEP0_T_LEN = 0;

		// The default packet is DATA1, which returns a response ACK.
		UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
	}
}

/**
 * Handle USB Data transfer
 */
inline void usbTransferInterrupt() {
	uint16_t len;

	switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP)) {
	// Endpoint 1# Endpoint interrupt upload
	case UIS_TOKEN_IN | 1:
		// Pre-use send length must be cleared
		UEP1_T_LEN = 0;

		// Default response NAK
		UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;
		break;

	// Endpoint 2# Endpoint bulk upload
	case UIS_TOKEN_IN | 2:
		// Pre-use send length must be cleared
		UEP2_T_LEN = 0;

		// Default response NAK
		UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;

		// Clear busy flag
		UpPoint2_Busy = 0;
		break;

	// Endpoint 3# Endpoint Batch Down
	case UIS_TOKEN_OUT | 2:
		// Out of sync packets will be dropped
		if (U_TOG_OK) {
			USBByteCount = USB_RX_LEN;

			// Take data pointer reset
			USBBufOutPoint = 0;

			// Receive a packet of data on the NAK,
			// the main function is processed,
			// and the main function modifies the response mode.
			UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_NAK;
		}
		break;

	// SETUP transaction
	case UIS_TOKEN_SETUP | 0:
		usbSetupInterrupt();
		break;

	// endpoint0 IN
	case UIS_TOKEN_IN | 0:
		switch (g_SetupReq) {
		case USB_GET_DESCRIPTOR:

			// This transmission length
			len = g_SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : g_SetupLen;

			// Load upload data
			memcpy(Ep0Buffer, g_pDescr, len);
			g_SetupLen -= len;
			g_pDescr += len;
			UEP0_T_LEN = len;

			// Sync flag bit flip
			UEP0_CTRL ^= bUEP_T_TOG;
			break;

		case USB_SET_ADDRESS:
			USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | g_SetupLen;
			UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
			break;

		default:
			// The status phase completes the interrupt or is forced to upload 0 length packet end control transmission
			UEP0_T_LEN = 0;
			UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
			break;
		}
		break;

	// endpoint0 OUT
	case UIS_TOKEN_OUT | 0:
		//Set the serial port properties
		if (g_SetupReq == SET_LINE_CODING) {
			if (U_TOG_OK) {
				memcpy(LineCoding, UsbSetupBuf, USB_RX_LEN);
				*((uint8_t *) &Baud) = LineCoding[0];
				*((uint8_t *) &Baud + 1) = LineCoding[1];
				*((uint8_t *) &Baud + 2) = LineCoding[2];
				*((uint8_t *) &Baud + 3) = LineCoding[3];

				if (Baud > 999999) {
					Baud = 57600;
				}

				UEP0_T_LEN = 0;

				// Ready to upload 0 packages
				UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_ACK;
			}
		} else {
			UEP0_T_LEN = 0;
			// Just ACK is fine.
			UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_ACK;
		}
		break;

	default:
		break;
	}
}

/**
 * USB Interrupt Handler, this is called by main.c,
 * the main Hardware interrupt handler
 */
void usbInterrupt() {
	// USB transfer completion flag
	if (UIF_TRANSFER) {
		usbTransferInterrupt();

		// Write 0 to clear interrupt
		UIF_TRANSFER = 0;
	}

	// Device Mode USB Bus Reset Interrupt
	if (UIF_BUS_RST) {
		usbResetInterrupt();
	}

	// USB bus suspend / wake up
	if (UIF_SUSPEND) {
		UIF_SUSPEND = 0;

		usbWakeupSuspendInterrupt();
	} else {
		// Unexpected interruption, impossible situation

		// Clear interrupt flag
		USB_INT_FG = 0xFF;
	}
}

/**
 * Send one byte over USB CDC Serial port
 */
void UsbCdc_putc(uint8_t tdata) {
	Receive_Uart_Buf[Uart_Input_Point++] = tdata;

	// Current buffer remaining bytes to be fetched
	UartByteCount++;
	if (Uart_Input_Point >= UART_REV_LEN) {
		Uart_Input_Point = 0;
	}
}

/**
 * Send 0 terminated string over USB CDC Serial port
 */
void UsbCdc_puts(char* str) {
	while (*str) {
		UsbCdc_putc(*(str++));
	}
}

/**
 * Send usb data from buffer
 */
void UsbCdc_processOutput() {
	static uint8_t uartTimeout = 0;

	if (g_UsbConfig) {
		if (UartByteCount) {
			uartTimeout++;
		}

		// The endpoint is not busy (the first packet of data after idle, only used to trigger the upload)
		if (!UpPoint2_Busy) {
			uint8_t length = UartByteCount;
			if (length > 0) {
				if (length > 39 || uartTimeout > 100) {
					uartTimeout = 0;
					if (Uart_Output_Point + length > UART_REV_LEN) {
						length = UART_REV_LEN - Uart_Output_Point;
					}

					UartByteCount -= length;
					// Write upload endpoint
					memcpy(Ep2Buffer + MAX_PACKET_SIZE, &Receive_Uart_Buf[Uart_Output_Point], length);
					Uart_Output_Point += length;
					if (Uart_Output_Point >= UART_REV_LEN) {
						Uart_Output_Point = 0;
					}

					// Pre-use send length must be cleared
					UEP2_T_LEN = length;

					// Answer ACK
					UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;
					UpPoint2_Busy = 1;
				}
			}
		}
	}
}

/**
 * Receive data from USB and process it, process only one byte at once
 */
void UsbCdc_processInput() {
	if (USBByteCount) {
		processCommandByte(Ep2Buffer[USBBufOutPoint++]);

		USBByteCount--;

		if (USBByteCount == 0) {
			UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK;
		}
	}
}
