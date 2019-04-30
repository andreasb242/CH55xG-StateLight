/**
 * Handle all USB Steup calls
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#include "usb-setup.h"

/**
 * Handle custom USB Requests
 */
void handleNonStandardUsbRequest() {

}

/**
 * Handle standard USB Request
 * @return Result package length, 0xff in error case
 */
void handleStandardUsbReqeuest() {
	switch (SetupReq) {
	// 0x21  currently configured
	case GET_LINE_CODING:

		// USB configuration flag
		const uint8_t *  pDescr;

		pDescr = LineCoding;
		uint8_t len = sizeof(LineCoding);

		// This transmission length
		len = SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : SetupLen;
		memcpy(Ep0Buffer, pDescr, len);
		SetupLen -= len;
		pDescr += len;
		return len;

		// 0x22 generates RS-232/V.24 style control signals
	case SET_CONTROL_LINE_STATE:
		return 0;

		//0x20  Configure
	case SET_LINE_CODING:
		return 0;

	default:
		// Command not supported
		return 0xff;
	}
}

/**
 * Process a valid setup request
 *
 * @return Result package length, 0xff in error case
 */
uint8_t handleValidSetupPackage() {
	SetupLen = ((uint16_t) UsbSetupBuf->wLengthH << 8) | (UsbSetupBuf->wLengthL);

	// The default is success and upload 0 length
	uint8_t len = 0;
	SetupReq = UsbSetupBuf->bRequest;

	// Non-standard request
	if ((UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD) {
		return handleStandardUsbReqeuest();
	} else {
		// Standard request

		// Request code
		switch (SetupReq) {
		case USB_GET_DESCRIPTOR:
			switch (UsbSetupBuf->wValueH) {

			// Device descriptor
			case 1:

				// Send the device descriptor to the buffer to be sent
				pDescr = DevDesc;
				len = sizeof(DevDesc);
				break;

				// Configuration descriptor
			case 2:

				// Send the device descriptor to the buffer to be sent
				pDescr = CfgDesc;
				len = sizeof(CfgDesc);
				break;

			case 3:
				if (UsbSetupBuf->wValueL == 0) {
					pDescr = LangDes;
					len = sizeof(LangDes);
				} else if (UsbSetupBuf->wValueL == 1) {
					pDescr = Manuf_Des;
					len = sizeof(Manuf_Des);
				} else if (UsbSetupBuf->wValueL == 2) {
					pDescr = Prod_Des;
					len = sizeof(Prod_Des);
				} else {
					pDescr = SerDes;
					len = sizeof(SerDes);
				}
				break;

			default:
				// Unsupported command or error
				len = 0xff;
				break;
			}
			if (SetupLen > len) {

				//Limit total length
				SetupLen = len;
			}

			//This transmission length
			len = SetupLen >= DEFAULT_ENDP0_SIZE ?
			DEFAULT_ENDP0_SIZE :
													SetupLen;

			//Load upload data
			memcpy(Ep0Buffer, pDescr, len);
			SetupLen -= len;
			pDescr += len;
			break;
		case USB_SET_ADDRESS:

			//暂存USB设备地址
			SetupLen = UsbSetupBuf->wValueL;
			break;

		case USB_GET_CONFIGURATION:
			Ep0Buffer[0] = UsbConfig;
			if (SetupLen >= 1) {
				len = 1;
			}
			break;

		case USB_SET_CONFIGURATION:
			UsbConfig = UsbSetupBuf->wValueL;
			break;

		case USB_GET_INTERFACE:
			break;

			//Clear Feature
		case USB_CLEAR_FEATURE:

			// Clear device
			if ((UsbSetupBuf->bRequestType & 0x1F) == USB_REQ_RECIP_DEVICE) {
				if ((((uint16_t) UsbSetupBuf->wValueH << 8)
						| UsbSetupBuf->wValueL) == 0x01) {
					if (CfgDesc[7] & 0x20) {
						// wake
					} else {

						// operation failed
						len = 0xFF;
					}
				} else {

					// operation failed
					len = 0xFF;
				}
			}

			// End point
			else if ((UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK)
					== USB_REQ_RECIP_ENDP) {
				switch (UsbSetupBuf->wIndexL) {
				case 0x83:
					UEP3_CTRL = UEP3_CTRL
							& ~( bUEP_T_TOG | MASK_UEP_T_RES)| UEP_T_RES_NAK;
					break;
				case 0x03:
					UEP3_CTRL = UEP3_CTRL
							& ~( bUEP_R_TOG | MASK_UEP_R_RES)| UEP_R_RES_ACK;
					break;
				case 0x82:
					UEP2_CTRL = UEP2_CTRL
							& ~( bUEP_T_TOG | MASK_UEP_T_RES)| UEP_T_RES_NAK;
					break;
				case 0x02:
					UEP2_CTRL = UEP2_CTRL
							& ~( bUEP_R_TOG | MASK_UEP_R_RES)| UEP_R_RES_ACK;
					break;
				case 0x81:
					UEP1_CTRL = UEP1_CTRL
							& ~( bUEP_T_TOG | MASK_UEP_T_RES)| UEP_T_RES_NAK;
					break;
				case 0x01:
					UEP1_CTRL = UEP1_CTRL
							& ~( bUEP_R_TOG | MASK_UEP_R_RES)| UEP_R_RES_ACK;
					break;
				default:

					// Unsupported endpoint
					len = 0xFF;
					break;
				}
			} else {

				// Not endpoints are not supported
				len = 0xFF;
			}
			break;

			// Set Feature
		case USB_SET_FEATURE:

			// Setting up the device
			if ((UsbSetupBuf->bRequestType & 0x1F) == USB_REQ_RECIP_DEVICE) {
				if ((((uint16_t) UsbSetupBuf->wValueH << 8) | UsbSetupBuf->wValueL) == 0x01) {
					if (CfgDesc[7] & 0x20) {
						// Sleep
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
					} else {
						// operation failed
						len = 0xFF;
					}
				} else {
					// operation failed
					len = 0xFF;
				}
			}

			// Set endpoint
			else if ((UsbSetupBuf->bRequestType & 0x1F) == USB_REQ_RECIP_ENDP) {
				if ((((uint16_t) UsbSetupBuf->wValueH << 8)
						| UsbSetupBuf->wValueL) == 0x00) {
					switch (((uint16_t) UsbSetupBuf->wIndexH << 8)
							| UsbSetupBuf->wIndexL) {
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
					default:

						// operation failed
						len = 0xFF;
						break;
					}
				} else {

					// operation failed
					len = 0xFF;
				}
			} else {

				// operation failed
				len = 0xFF;
			}
			break;
		case USB_GET_STATUS:
			Ep0Buffer[0] = 0x00;
			Ep0Buffer[1] = 0x00;
			if (SetupLen >= 2) {
				len = 2;
			} else {
				len = SetupLen;
			}
			break;
		default:

			// operation failed
			len = 0xff;
			break;
		}
	}
}

/**
 * USB Setup call
 */
void usbSetupCall() {
	uint8_t len;
	if (USB_RX_LEN == (sizeof(USB_SETUP_REQ))) {
		len = handleValidSetupPackage();
	} else {
		// Wrong packet length
		len = 0xff;
	}

	if (len == 0xff) {
		SetupReq = 0xFF;
		// STALL
		UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;
	} else if (len <= DEFAULT_ENDP0_SIZE) {
		// Upload data or status stage returns 0 length package

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
