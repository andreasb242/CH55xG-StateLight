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
 * TODO Documentation
 */
uint16_t SetupLen;

/**
 * TODO Documentation
 */
uint8_t SetupReq;

/**
 * TODO Documentation
 */
uint8_t UsbConfig;

/**
 * USB configuration flag
 */
const uint8_t *  pDescr;

/**
 * TODO Documentation
 */
#define UsbSetupBuf	 ((PUSB_SETUP_REQ)Ep0Buffer)

// Configures DTE rate, stop-bits, parity, and number-of-character
#define SET_LINE_CODING		0X20

// This request allows the host to find out the currently configured line coding.
#define GET_LINE_CODING		0X21

// This request generates RS-232/V.24 style control signals.
#define SET_CONTROL_LINE_STATE	0X22

/**
 * Baud rate TODO Not needed for Virtual USB without hardware Serial
 */
uint32_t Baud = 0;

#define UART_REV_LEN  64				 //串口接收缓冲区大小
__idata uint8_t Receive_Uart_Buf[UART_REV_LEN];   //串口接收缓冲区
volatile __idata uint8_t Uart_Input_Point = 0;   //循环缓冲区写入指针，总线复位需要初始化为0
volatile __idata uint8_t Uart_Output_Point = 0;  //循环缓冲区取出指针，总线复位需要初始化为0
volatile __idata uint8_t UartByteCount = 0;	  //当前缓冲区剩余待取字节数


volatile __idata uint8_t USBByteCount = 0;	  //代表USB端点接收到的数据
volatile __idata uint8_t USBBufOutPoint = 0;	//取数据指针

volatile __idata uint8_t UpPoint2_Busy  = 0;   //上传端点是否忙标志

#define BOOT_ADDR  0x3800

/**
 * USB Interrupt Handler
 */
void usbInterrupt()
{
	uint16_t len;
	if(UIF_TRANSFER)															//USB传输完成标志
	{
		switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
		{
		case UIS_TOKEN_IN | 1:												  //endpoint 1# 端点中断上传
			UEP1_T_LEN = 0;
			UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;		   //默认应答NAK
			break;
		case UIS_TOKEN_IN | 2:												  //endpoint 2# 端点批量上传
		{
			UEP2_T_LEN = 0;													//预使用发送长度一定要清空
			UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;		   //默认应答NAK
			UpPoint2_Busy = 0;												  //清除忙标志
		}
			break;
		case UIS_TOKEN_OUT | 2:												 //endpoint 3# 端点批量下传
			if ( U_TOG_OK )													 // 不同步的数据包将丢弃
			{
				USBByteCount = USB_RX_LEN;
				USBBufOutPoint = 0;											 //取数据指针复位
				UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_NAK;	   //收到一包数据就NAK，主函数处理完，由主函数修改响应方式
			}
			break;
		case UIS_TOKEN_SETUP | 0:												//SETUP事务
			len = USB_RX_LEN;
			if(len == (sizeof(USB_SETUP_REQ)))
			{
				SetupLen = ((uint16_t)UsbSetupBuf->wLengthH<<8) | (UsbSetupBuf->wLengthL);
				len = 0;													  // 默认为成功并且上传0长度
				SetupReq = UsbSetupBuf->bRequest;
				if ( ( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )//非标准请求
				{
					switch( SetupReq )
					{
					case GET_LINE_CODING:   //0x21  currently configured
						pDescr = LineCoding;
						len = sizeof(LineCoding);
						len = SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : SetupLen;  // 本次传输长度
						memcpy(Ep0Buffer,pDescr,len);
						SetupLen -= len;
						pDescr += len;
						break;
					case SET_CONTROL_LINE_STATE:  //0x22  generates RS-232/V.24 style control signals
						break;
					case SET_LINE_CODING:	  //0x20  Configure
						break;
					default:
						len = 0xFF;  								 									 /*命令不支持*/
						break;
					}
				}
				else															 //标准请求
				{
					switch(SetupReq)											 //请求码
					{
					case USB_GET_DESCRIPTOR:
						switch(UsbSetupBuf->wValueH)
						{
						case 1:													   //设备描述符
							pDescr = DevDesc;										 //把设备描述符送到要发送的缓冲区
							len = sizeof(DevDesc);
							break;
						case 2:														//配置描述符
							pDescr = CfgDesc;										  //把设备描述符送到要发送的缓冲区
							len = sizeof(CfgDesc);
							break;
						case 3:
							if(UsbSetupBuf->wValueL == 0)
							{
								pDescr = g_DescriptorLanguage;
								len = sizeof(g_DescriptorLanguage);
							}
							else if(UsbSetupBuf->wValueL == 1)
							{
								pDescr = g_DescriptorManufacturer;
								len = sizeof(g_DescriptorManufacturer);
							}
							else if(UsbSetupBuf->wValueL == 2)
							{
								pDescr = g_DescriptorProduct;
								len = sizeof(g_DescriptorProduct);
							}
							else
							{
								pDescr = g_DescriptorSerial;
								len = sizeof(g_DescriptorSerial);
							}
							break;
						default:
							len = 0xff;												//不支持的命令或者出错
							break;
						}
						if ( SetupLen > len )
						{
							SetupLen = len;	//限制总长度
						}
						len = SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : SetupLen;							//本次传输长度
						memcpy(Ep0Buffer,pDescr,len);								  //加载上传数据
						SetupLen -= len;
						pDescr += len;
						break;
					case USB_SET_ADDRESS:
						SetupLen = UsbSetupBuf->wValueL;							  //暂存USB设备地址
						break;
					case USB_GET_CONFIGURATION:
						Ep0Buffer[0] = UsbConfig;
						if ( SetupLen >= 1 )
						{
							len = 1;
						}
						break;
					case USB_SET_CONFIGURATION:
						UsbConfig = UsbSetupBuf->wValueL;
						break;
					case USB_GET_INTERFACE:
						break;
					case USB_CLEAR_FEATURE:											//Clear Feature
						if( ( UsbSetupBuf->bRequestType & 0x1F ) == USB_REQ_RECIP_DEVICE )				  /* 清除设备 */
						{
							if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
							{
								if( CfgDesc[ 7 ] & 0x20 )
								{
									/* 唤醒 */
								}
								else
								{
									len = 0xFF;										/* 操作失败 */
								}
							}
							else
							{
								len = 0xFF;											/* 操作失败 */
							}
						}
						else if ( ( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )// 端点
						{
							switch( UsbSetupBuf->wIndexL )
							{
							case 0x83:
								UEP3_CTRL = UEP3_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
								break;
							case 0x03:
								UEP3_CTRL = UEP3_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
								break;
							case 0x82:
								UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
								break;
							case 0x02:
								UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
								break;
							case 0x81:
								UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
								break;
							case 0x01:
								UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
								break;
							default:
								len = 0xFF;										 // 不支持的端点
								break;
							}
						}
						else
						{
							len = 0xFF;												// 不是端点不支持
						}
						break;
					case USB_SET_FEATURE:										  /* Set Feature */
						if( ( UsbSetupBuf->bRequestType & 0x1F ) == USB_REQ_RECIP_DEVICE )				  /* 设置设备 */
						{
							if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
							{
								if( CfgDesc[ 7 ] & 0x20 )
								{
									/* 休眠 */
									while ( XBUS_AUX & bUART0_TX )
									{
										;	//等待发送完成
									}
									SAFE_MOD = 0x55;
									SAFE_MOD = 0xAA;
									WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO | bWAK_RXD1_LO;					  //USB或者RXD0/1有信号时可被唤醒
									PCON |= PD;																 //睡眠
									SAFE_MOD = 0x55;
									SAFE_MOD = 0xAA;
									WAKE_CTRL = 0x00;
								}
								else
								{
									len = 0xFF;										/* 操作失败 */
								}
							}
							else
							{
								len = 0xFF;											/* 操作失败 */
							}
						}
						else if( ( UsbSetupBuf->bRequestType & 0x1F ) == USB_REQ_RECIP_ENDP )			 /* 设置端点 */
						{
							if( ( ( ( uint16_t )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x00 )
							{
								switch( ( ( uint16_t )UsbSetupBuf->wIndexH << 8 ) | UsbSetupBuf->wIndexL )
								{
								case 0x83:
									UEP3_CTRL = UEP3_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* 设置端点3 IN STALL */
									break;
								case 0x03:
									UEP3_CTRL = UEP3_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* 设置端点3 OUT Stall */
									break;
								case 0x82:
									UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* 设置端点2 IN STALL */
									break;
								case 0x02:
									UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* 设置端点2 OUT Stall */
									break;
								case 0x81:
									UEP1_CTRL = UEP1_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* 设置端点1 IN STALL */
									break;
								case 0x01:
									UEP1_CTRL = UEP1_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* 设置端点1 OUT Stall */
								default:
									len = 0xFF;									/* 操作失败 */
									break;
								}
							}
							else
							{
								len = 0xFF;									  /* 操作失败 */
							}
						}
						else
						{
							len = 0xFF;										  /* 操作失败 */
						}
						break;
					case USB_GET_STATUS:
						Ep0Buffer[0] = 0x00;
						Ep0Buffer[1] = 0x00;
						if ( SetupLen >= 2 )
						{
							len = 2;
						}
						else
						{
							len = SetupLen;
						}
						break;
					default:
						len = 0xff;													//操作失败
						break;
					}
				}
			}
			else
			{
				len = 0xff;														 //包长度错误
			}
			if(len == 0xff)
			{
				SetupReq = 0xFF;
				UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;//STALL
			}
			else if(len <= DEFAULT_ENDP0_SIZE)													   //上传数据或者状态阶段返回0长度包
			{
				UEP0_T_LEN = len;
				UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//默认数据包是DATA1，返回应答ACK
			}
			else
			{
				UEP0_T_LEN = 0;  //虽然尚未到状态阶段，但是提前预置上传0长度数据包以防主机提前进入状态阶段
				UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//默认数据包是DATA1,返回应答ACK
			}
			break;
		case UIS_TOKEN_IN | 0:													  //endpoint0 IN
			switch(SetupReq)
			{
			case USB_GET_DESCRIPTOR:
				len = SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : SetupLen;								 //本次传输长度
				memcpy( Ep0Buffer, pDescr, len );								   //加载上传数据
				SetupLen -= len;
				pDescr += len;
				UEP0_T_LEN = len;
				UEP0_CTRL ^= bUEP_T_TOG;											 //同步标志位翻转
				break;
			case USB_SET_ADDRESS:
				USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
				UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
				break;
			default:
				UEP0_T_LEN = 0;													  //状态阶段完成中断或者是强制上传0长度数据包结束控制传输
				UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
				break;
			}
			break;
		case UIS_TOKEN_OUT | 0:  // endpoint0 OUT
			if(SetupReq == SET_LINE_CODING)  //设置串口属性
			{
				if( U_TOG_OK )
				{
					memcpy(LineCoding,UsbSetupBuf,USB_RX_LEN);
					*((uint8_t *)&Baud) = LineCoding[0];
					*((uint8_t *)&Baud+1) = LineCoding[1];
					*((uint8_t *)&Baud+2) = LineCoding[2];
					*((uint8_t *)&Baud+3) = LineCoding[3];

					if(Baud > 999999) Baud = 57600;

					UEP0_T_LEN = 0;
					UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_ACK;  // 准备上传0包
				}
			}
			else
			{
				UEP0_T_LEN = 0;
				UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_ACK;  // 只要ACK就好了
			}
			break;



		default:
			break;
		}
		UIF_TRANSFER = 0;														   //写0清空中断
	}

	// Device Mode USB Bus Reset Interrupt
	if (UIF_BUS_RST) {
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

	// USB bus suspend / wake up
	if (UIF_SUSPEND) {
		UIF_SUSPEND = 0;

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

void usb_poll() {
	static uint8_t Uart_Timeout = 0;
	if (UsbConfig) {
		if (UartByteCount) {
			Uart_Timeout++;
		}

		// The endpoint is not busy (the first packet of data after idle, only used to trigger the upload)
		if (!UpPoint2_Busy) {
			uint8_t length = UartByteCount;
			if (length > 0) {
				if (length > 39 || Uart_Timeout > 100) {
					Uart_Timeout = 0;
					if (Uart_Output_Point + length > UART_REV_LEN) {
						length = UART_REV_LEN - Uart_Output_Point;
					}

					UartByteCount -= length;
					// Write upload endpoint
					memcpy(Ep2Buffer + MAX_PACKET_SIZE,	&Receive_Uart_Buf[Uart_Output_Point], length);
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
