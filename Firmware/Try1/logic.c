/**
 * State light logic
 *
 * General logic
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#include "logic.h"
#include "usb-setup.h"

// Endpoint 0 OUT & IN buffer, must be an even address
__xdata __at (0x0000) uint8_t Ep0Buffer[DEFAULT_ENDP0_SIZE];

// Endpoint 1 upload buffer
__xdata __at (0x0040) uint8_t Ep1Buffer[DEFAULT_ENDP1_SIZE];

// Endpoint 2 IN & OUT buffer, must be an even address
__xdata __at (0x0080) uint8_t Ep2Buffer[2 * MAX_PACKET_SIZE];

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

	// Allow microcontroller interrupt
	EA = 1;
}













































uint16_t SetupLen;
uint8_t   SetupReq,Count,UsbConfig;

// Staging the Setup package
USB_SETUP_REQ   SetupReqBuf;
#define UsbSetupBuf     ((PUSB_SETUP_REQ)Ep0Buffer)


// Configures DTE rate, stop-bits, parity, and number-of-character
#define  SET_LINE_CODING                0X20

// This request allows the host to find out the currently configured line coding.
#define  GET_LINE_CODING                0X21

// This request generates RS-232/V.24 style control signals.
#define  SET_CONTROL_LINE_STATE         0X22


// Device descriptor
__code uint8_t DevDesc[] = {0x12,0x01,0x10,0x01,0x02,0x00,0x00,DEFAULT_ENDP0_SIZE,
                            0x86,0x1a,0x22,0x57,0x00,0x01,0x01,0x02,
                            0x03,0x01
                           };

__code uint8_t CfgDesc[] ={

		//Configuration descriptor (two interfaces)
    0x09,0x02,0x43,0x00,0x02,0x01,0x00,0xa0,0x32,

	//The following is the interface 0 (CDC interface) descriptor

	  //CDC interface descriptor (one endpoint)
    0x09,0x04,0x00,0x00,0x01,0x02,0x02,0x01,0x00,
    //The following is the function descriptor


	 //Function descriptor (header)
	0x05,0x24,0x00,0x10,0x01,

	 //Management descriptor (no data class interface) 03 01
    0x05,0x24,0x01,0x00,0x00,

    // stand by Set_Line_Coding、Set_Control_Line_State、Get_Line_Coding、Serial_State
    0x04,0x24,0x02,0x02,

	 //CDC interface numbered 0; data class interface number 1
    0x05,0x24,0x06,0x00,0x01,

	 //Interrupt upload endpoint descriptor
    0x07,0x05,0x81,0x03,0x08,0x00,0xFF,

	//The following is the interface 1 (data interface) descriptor

	 //Data interface descriptor
	0x09,0x04,0x01,0x00,0x02,0x0a,0x00,0x00,0x00,


	  //Endpoint descriptor
    0x07,0x05,0x02,0x02,0x40,0x00,0x00,

	  //Endpoint descriptor
    0x07,0x05,0x82,0x02,0x40,0x00,0x00,
};

//String descriptor

//Language descriptor
unsigned char  __code LangDes[]={0x04,0x03,0x09,0x04};
unsigned char  __code SerDes[]={                                 //Serial number string descriptor
                                                                 0x14,0x03,
                                                                 0x32,0x00,0x30,0x00,0x31,0x00,0x37,0x00,0x2D,0x00,
                                                                 0x32,0x00,0x2D,0x00,
                                                                 0x32,0x00,0x35,0x00
                               };
unsigned char  __code Prod_Des[]={                                //Product string descriptor
                                                                  0x14,0x03,
                                                                  0x43,0x00,0x48,0x00,0x35,0x00,0x35,0x00,0x34,0x00,0x5F,0x00,
                                                                  0x43,0x00,0x44,0x00,0x43,0x00,
                                 };

unsigned char  __code Manuf_Des[]={
    0x0A,0x03,
    0x5F,0x6c,0xCF,0x82,0x81,0x6c,0x52,0x60,
};

//CDC parameter

//The initial baud rate is 57600, 1 stop bit, no parity, 8 data bits.
__xdata uint8_t LineCoding[7]={0x00,0xe1,0x00,0x00,0x00,0x00,0x08};


//Serial receive buffer size
#define UART_REV_LEN  64


//Serial receive buffer
__idata uint8_t Receive_Uart_Buf[UART_REV_LEN];

//The circular buffer write pointer, the bus reset needs to be initialized to 0
volatile __idata uint8_t Uart_Input_Point = 0;

//The circular buffer fetches the pointer, and the bus reset needs to be initialized to 0.
volatile __idata uint8_t Uart_Output_Point = 0;

//Current buffer remaining bytes to be fetched
volatile __idata uint8_t UartByteCount = 0;


//Data received on behalf of the USB endpoint
volatile __idata uint8_t USBByteCount = 0;

//Data pointer
volatile __idata uint8_t USBBufOutPoint = 0;

//Upload endpoint is busy flag
volatile __idata uint8_t UpPoint2_Busy  = 0;

/**
 * Handle complete USB Transfer
 */
inline void usbTransferComplete() {
	uint16_t len;

	switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP)) {

	// Endpoint 1# Endpoint interrupt upload
	case UIS_TOKEN_IN | 1:
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

			// Receive a packet of data on the NAK, the main function is processed, and the main function modifies the response mode.
			UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_NAK;
		}
		break;

		// SETUP transaction
	case UIS_TOKEN_SETUP | 0:
		usbSetupCall();
		break;

		// endpoint0 IN
	case UIS_TOKEN_IN | 0:
		switch (SetupReq) {
		case USB_GET_DESCRIPTOR:

			// This transmission length
			len = SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : SetupLen;

			// Load upload data
			memcpy(Ep0Buffer, pDescr, len);
			SetupLen -= len;
			pDescr += len;
			UEP0_T_LEN = len;

			// Sync flag bit flip
			UEP0_CTRL ^= bUEP_T_TOG;
			break;

		case USB_SET_ADDRESS:
			USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
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

		// Set the serial port properties
		if (SetupReq == SET_LINE_CODING) {
			if (U_TOG_OK) {
				memcpy(LineCoding, UsbSetupBuf, USB_RX_LEN);

				// TODO Only virtual UART, nothing to configure here
				//Config_Uart1(LineCoding);
				UEP0_T_LEN = 0;

				// Ready to upload 0 packages
				UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_ACK;
			}
		} else {
			UEP0_T_LEN = 0;

			// State stage, responding to NAK in IN
			UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_NAK;
		}
		break;

	default:
		break;
	}

	// Write 0 to clear interrupt
	UIF_TRANSFER = 0;
}

/**
 * USB Bus Reset
 */
inline void usbResetCall() {
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
    UsbConfig = 0;
    UpPoint2_Busy = 0;
}

/**
 * USB Suspend
 */
inline void usbSuspendCall() {
	// Hang
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
 * Handle USB Interrupts
 */
void usbInterruptHandler() __interrupt (INT_NO_USB) {
	// USB transfer completion flag
	if (UIF_TRANSFER) {
		usbTransferComplete();
	}

	// Device Mode USB Bus Reset Interrupt
	if (UIF_BUS_RST) {
		usbResetCall();
	}

	// USB bus suspend/wake up
	if (UIF_SUSPEND) {
		UIF_SUSPEND = 0;

		usbSuspendCall();
	} else {
		// Unexpected interruption, impossible situation

		// Clear interrupt flag
		USB_INT_FG = 0xFF;
	}
}



/*******************************************************************************
* Function Name  : Uart1_ISR()
* Description    : The serial port receives the interrupt function to implement circular buffer reception.
*******************************************************************************/
/*
void Uart1_ISR(void) __interrupt (INT_NO_UART1)
{
    if(U1RI)   //Received data
    {
        Receive_Uart_Buf[Uart_Input_Point++] = SBUF1;
        UartByteCount++;                    //Current buffer remaining bytes to be fetched
        if(Uart_Input_Point>=UART_REV_LEN)
            Uart_Input_Point = 0;           //Write pointer
        U1RI =0;
    }

}
*/






/**
 * Main Loop
 */
void mainLoop() {
    uint8_t Uart_Timeout = 0;
    while(1)
    {
        if(UsbConfig)
        {
        	// USB receiving endpoint has data
            if(USBByteCount)
            {
                CH554UART1SendByte(Ep2Buffer[USBBufOutPoint++]);
                USBByteCount--;
                if(USBByteCount==0)
                    UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK;

            }
            if(UartByteCount)
                Uart_Timeout++;

            // The endpoint is not busy (the first packet of data after idle, only used to trigger the upload)
            if(!UpPoint2_Busy)
            {
            	uint8_t length = UartByteCount;
                if(length>0)
                {
                    if(length>39 || Uart_Timeout>100)
                    {
                        Uart_Timeout = 0;
                        if(Uart_Output_Point+length>UART_REV_LEN)
                            length = UART_REV_LEN-Uart_Output_Point;
                        UartByteCount -= length;

                        // Write upload endpoint
                        memcpy(Ep2Buffer+MAX_PACKET_SIZE,&Receive_Uart_Buf[Uart_Output_Point],length);
                        Uart_Output_Point+=length;
                        if(Uart_Output_Point>=UART_REV_LEN)
                            Uart_Output_Point = 0;

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
}
