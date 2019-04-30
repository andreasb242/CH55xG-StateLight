/**
 * State light logic
 *
 * General logic
 *
 * Andreas Butt, (c) 2019
 * License: GPL
 */

#include "logic.h"

// Endpoint 0 OUT&IN buffer, must be an even address
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
	// TODO: Is casting the right thing here? What about endianness?
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

//USB配置标志
const uint8_t *  pDescr;

//暂存Setup包
USB_SETUP_REQ   SetupReqBuf;
#define UsbSetupBuf     ((PUSB_SETUP_REQ)Ep0Buffer)


// Configures DTE rate, stop-bits, parity, and number-of-character
#define  SET_LINE_CODING                0X20

// This request allows the host to find out the currently configured line coding.
#define  GET_LINE_CODING                0X21

// This request generates RS-232/V.24 style control signals.
#define  SET_CONTROL_LINE_STATE         0X22


//设备描述符
__code uint8_t DevDesc[] = {0x12,0x01,0x10,0x01,0x02,0x00,0x00,DEFAULT_ENDP0_SIZE,
                            0x86,0x1a,0x22,0x57,0x00,0x01,0x01,0x02,
                            0x03,0x01
                           };
__code uint8_t CfgDesc[] ={

		//配置描述符（两个接口）
    0x09,0x02,0x43,0x00,0x02,0x01,0x00,0xa0,0x32,

	//以下为接口0（CDC接口）描述符

	  //CDC接口描述符(一个端点)
    0x09,0x04,0x00,0x00,0x01,0x02,0x02,0x01,0x00,
    //以下为功能描述符


	 //功能描述符(头)
	0x05,0x24,0x00,0x10,0x01,

	 //管理描述符(没有数据类接口) 03 01
    0x05,0x24,0x01,0x00,0x00,

    //支持Set_Line_Coding、Set_Control_Line_State、Get_Line_Coding、Serial_State
    0x04,0x24,0x02,0x02,

	 //编号为0的CDC接口;编号1的数据类接口
    0x05,0x24,0x06,0x00,0x01,

	 //中断上传端点描述符
    0x07,0x05,0x81,0x03,0x08,0x00,0xFF,

	//以下为接口1（数据接口）描述符

	 //数据接口描述符
	0x09,0x04,0x01,0x00,0x02,0x0a,0x00,0x00,0x00,


	  //端点描述符
    0x07,0x05,0x02,0x02,0x40,0x00,0x00,

	  //端点描述符
    0x07,0x05,0x82,0x02,0x40,0x00,0x00,
};

//字符串描述符

//语言描述符
unsigned char  __code LangDes[]={0x04,0x03,0x09,0x04};
unsigned char  __code SerDes[]={                                 //序列号字符串描述符
                                                                 0x14,0x03,
                                                                 0x32,0x00,0x30,0x00,0x31,0x00,0x37,0x00,0x2D,0x00,
                                                                 0x32,0x00,0x2D,0x00,
                                                                 0x32,0x00,0x35,0x00
                               };
unsigned char  __code Prod_Des[]={                                //产品字符串描述符
                                                                  0x14,0x03,
                                                                  0x43,0x00,0x48,0x00,0x35,0x00,0x35,0x00,0x34,0x00,0x5F,0x00,
                                                                  0x43,0x00,0x44,0x00,0x43,0x00,
                                 };

unsigned char  __code Manuf_Des[]={
    0x0A,0x03,
    0x5F,0x6c,0xCF,0x82,0x81,0x6c,0x52,0x60,
};

//cdc参数

//初始化波特率为57600，1停止位，无校验，8数据位。
__xdata uint8_t LineCoding[7]={0x00,0xe1,0x00,0x00,0x00,0x00,0x08};


//串口接收缓冲区大小
#define UART_REV_LEN  64


//串口接收缓冲区
__idata uint8_t Receive_Uart_Buf[UART_REV_LEN];

//循环缓冲区写入指针，总线复位需要初始化为0
volatile __idata uint8_t Uart_Input_Point = 0;

//循环缓冲区取出指针，总线复位需要初始化为0
volatile __idata uint8_t Uart_Output_Point = 0;

//当前缓冲区剩余待取字节数
volatile __idata uint8_t UartByteCount = 0;


//代表USB端点接收到的数据
volatile __idata uint8_t USBByteCount = 0;

//取数据指针
volatile __idata uint8_t USBBufOutPoint = 0;

//上传端点是否忙标志
volatile __idata uint8_t UpPoint2_Busy  = 0;



/**
 * USB Setup call
 */
inline void usbSetupCall() {
	uint8_t len = USB_RX_LEN;
	if (len == (sizeof(USB_SETUP_REQ))) {
		SetupLen = ((uint16_t) UsbSetupBuf->wLengthH << 8)
				| (UsbSetupBuf->wLengthL);

		// 默认为成功并且上传0长度
		len = 0;
		SetupReq = UsbSetupBuf->bRequest;

		//非标准请求
		if ((UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK)
				!= USB_REQ_TYP_STANDARD) {
			switch (SetupReq) {

			//0x21  currently configured
			case GET_LINE_CODING:
				pDescr = LineCoding;
				len = sizeof(LineCoding);

				// 本次传输长度
				len = SetupLen >= DEFAULT_ENDP0_SIZE ?
						DEFAULT_ENDP0_SIZE : SetupLen;
				memcpy(Ep0Buffer, pDescr, len);
				SetupLen -= len;
				pDescr += len;
				break;

				//0x22  generates RS-232/V.24 style control signals
			case SET_CONTROL_LINE_STATE:
				break;

				//0x20  Configure
			case SET_LINE_CODING:
				break;
			default:

				//命令不支持
				len = 0xFF;
				break;
			}
		} else {
			//标准请求

			//请求码
			switch (SetupReq) {
			case USB_GET_DESCRIPTOR:
				switch (UsbSetupBuf->wValueH) {

				//设备描述符
				case 1:

					//把设备描述符送到要发送的缓冲区
					pDescr = DevDesc;
					len = sizeof(DevDesc);
					break;

					//配置描述符
				case 2:

					//把设备描述符送到要发送的缓冲区
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
					//不支持的命令或者出错
					len = 0xff;
					break;
				}
				if (SetupLen > len) {

					//限制总长度
					SetupLen = len;
				}

				//本次传输长度
				len = SetupLen >= DEFAULT_ENDP0_SIZE ?
						DEFAULT_ENDP0_SIZE : SetupLen;

				//加载上传数据
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

				// 清除设备
				if ((UsbSetupBuf->bRequestType & 0x1F)
						== USB_REQ_RECIP_DEVICE) {
					if ((((uint16_t) UsbSetupBuf->wValueH << 8)
							| UsbSetupBuf->wValueL) == 0x01) {
						if (CfgDesc[7] & 0x20) {
							// 唤醒
						} else {

							// 操作失败
							len = 0xFF;
						}
					} else {

						// 操作失败
						len = 0xFF;
					}
				}

				// 端点
				else if ((UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK)
						== USB_REQ_RECIP_ENDP) {
					switch (UsbSetupBuf->wIndexL) {
					case 0x83:
						UEP3_CTRL =
								UEP3_CTRL
										& ~( bUEP_T_TOG | MASK_UEP_T_RES)| UEP_T_RES_NAK;
						break;
					case 0x03:
						UEP3_CTRL =
								UEP3_CTRL
										& ~( bUEP_R_TOG | MASK_UEP_R_RES)| UEP_R_RES_ACK;
						break;
					case 0x82:
						UEP2_CTRL =
								UEP2_CTRL
										& ~( bUEP_T_TOG | MASK_UEP_T_RES)| UEP_T_RES_NAK;
						break;
					case 0x02:
						UEP2_CTRL =
								UEP2_CTRL
										& ~( bUEP_R_TOG | MASK_UEP_R_RES)| UEP_R_RES_ACK;
						break;
					case 0x81:
						UEP1_CTRL =
								UEP1_CTRL
										& ~( bUEP_T_TOG | MASK_UEP_T_RES)| UEP_T_RES_NAK;
						break;
					case 0x01:
						UEP1_CTRL =
								UEP1_CTRL
										& ~( bUEP_R_TOG | MASK_UEP_R_RES)| UEP_R_RES_ACK;
						break;
					default:

						// 不支持的端点
						len = 0xFF;
						break;
					}
				} else {

					// 不是端点不支持
					len = 0xFF;
				}
				break;

				// Set Feature
			case USB_SET_FEATURE:

				// 设置设备
				if ((UsbSetupBuf->bRequestType & 0x1F)
						== USB_REQ_RECIP_DEVICE) {
					if ((((uint16_t) UsbSetupBuf->wValueH << 8)
							| UsbSetupBuf->wValueL) == 0x01) {
						if (CfgDesc[7] & 0x20) {
							// 休眠
							while (XBUS_AUX & bUART0_TX) {
								;    //等待发送完成
							}
							SAFE_MOD = 0x55;
							SAFE_MOD = 0xAA;

							//USB或者RXD0/1有信号时可被唤醒
							WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO
									| bWAK_RXD1_LO;

							//睡眠
							PCON |= PD;
							SAFE_MOD = 0x55;
							SAFE_MOD = 0xAA;
							WAKE_CTRL = 0x00;
						} else {

							// 操作失败
							len = 0xFF;
						}
					} else {

						// 操作失败
						len = 0xFF;
					}
				}

				// 设置端点
				else if ((UsbSetupBuf->bRequestType & 0x1F)
						== USB_REQ_RECIP_ENDP) {
					if ((((uint16_t) UsbSetupBuf->wValueH << 8)
							| UsbSetupBuf->wValueL) == 0x00) {
						switch (((uint16_t) UsbSetupBuf->wIndexH << 8)
								| UsbSetupBuf->wIndexL) {
						case 0x83:

							/* 设置端点3 IN STALL */
							UEP3_CTRL = UEP3_CTRL
									& (~bUEP_T_TOG)| UEP_T_RES_STALL;
							break;
						case 0x03:

							/* 设置端点3 OUT Stall */
							UEP3_CTRL = UEP3_CTRL
									& (~bUEP_R_TOG)| UEP_R_RES_STALL;
							break;
						case 0x82:

							/* 设置端点2 IN STALL */
							UEP2_CTRL = UEP2_CTRL
									& (~bUEP_T_TOG)| UEP_T_RES_STALL;
							break;
						case 0x02:

							/* 设置端点2 OUT Stall */
							UEP2_CTRL = UEP2_CTRL
									& (~bUEP_R_TOG)| UEP_R_RES_STALL;
							break;
						case 0x81:

							/* 设置端点1 IN STALL */
							UEP1_CTRL = UEP1_CTRL
									& (~bUEP_T_TOG)| UEP_T_RES_STALL;
							break;
						case 0x01:

							/* 设置端点1 OUT Stall */
							UEP1_CTRL = UEP1_CTRL
									& (~bUEP_R_TOG)| UEP_R_RES_STALL;
						default:

							/* 操作失败 */
							len = 0xFF;
							break;
						}
					} else {

						/* 操作失败 */
						len = 0xFF;
					}
				} else {

					/* 操作失败 */
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

				//操作失败
				len = 0xff;
				break;
			}
		}
	} else {

		//包长度错误
		len = 0xff;
	}


	if (len == 0xff) {
		SetupReq = 0xFF;

		//STALL
		UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL
				| UEP_T_RES_STALL;
	}

	//上传数据或者状态阶段返回0长度包
	else if (len <= DEFAULT_ENDP0_SIZE) {
		UEP0_T_LEN = len;

		//默认数据包是DATA1，返回应答ACK
		UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
	} else {

		//虽然尚未到状态阶段，但是提前预置上传0长度数据包以防主机提前进入状态阶段
		UEP0_T_LEN = 0;

		//默认数据包是DATA1,返回应答ACK
		UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
	}
}

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
inline void usbInterruptHandler() {
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
* Description    : 串口接收中断函数，实现循环缓冲接收
*******************************************************************************/
/*
void Uart1_ISR(void) __interrupt (INT_NO_UART1)
{
    if(U1RI)   //收到数据
    {
        Receive_Uart_Buf[Uart_Input_Point++] = SBUF1;
        UartByteCount++;                    //当前缓冲区剩余待取字节数
        if(Uart_Input_Point>=UART_REV_LEN)
            Uart_Input_Point = 0;           //写入指针
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
            if(USBByteCount)   //USB接收端点有数据
            {
                CH554UART1SendByte(Ep2Buffer[USBBufOutPoint++]);
                USBByteCount--;
                if(USBByteCount==0)
                    UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK;

            }
            if(UartByteCount)
                Uart_Timeout++;
            if(!UpPoint2_Busy)   //端点不繁忙（空闲后的第一包数据，只用作触发上传）
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
                        //写上传端点
                        memcpy(Ep2Buffer+MAX_PACKET_SIZE,&Receive_Uart_Buf[Uart_Output_Point],length);
                        Uart_Output_Point+=length;
                        if(Uart_Output_Point>=UART_REV_LEN)
                            Uart_Output_Point = 0;
                        UEP2_T_LEN = length;                                                    //预使用发送长度一定要清空
                        UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;            //应答ACK
                        UpPoint2_Busy = 1;
                    }
                }
            }
        }
    }
}


/**
 * CH559 USB interrupt handler
 */
void DeviceInterrupt(void) __interrupt (INT_NO_USB) {
	usbInterruptHandler();
}

