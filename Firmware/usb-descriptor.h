/**
 * USB Descriptors
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#pragma once

#include "inc.h"


// Device descriptor
__code uint8_t DevDesc[] = { 0x12, 0x01, 0x10, 0x01, 0x02, 0x00, 0x00,
		DEFAULT_ENDP0_SIZE, 0x86, 0x1a, 0x22, 0x57, 0x00, 0x01, 0x01, 0x02,
		0x03, 0x01 };

__code uint8_t CfgDesc[] ={
	0x09,0x02,0x43,0x00,0x02,0x01,0x00,0xa0,0x32,			 //配置描述符（两个接口）
	//以下为接口0（CDC接口）描述符
	0x09,0x04,0x00,0x00,0x01,0x02,0x02,0x01,0x00,			 //CDC接口描述符(一个端点)
	//以下为功能描述符
	0x05,0x24,0x00,0x10,0x01,								 //功能描述符(头)
	0x05,0x24,0x01,0x00,0x00,								 //管理描述符(没有数据类接口) 03 01
	0x04,0x24,0x02,0x02,									  //支持Set_Line_Coding、Set_Control_Line_State、Get_Line_Coding、Serial_State
	0x05,0x24,0x06,0x00,0x01,								 //编号为0的CDC接口;编号1的数据类接口
	0x07,0x05,0x81,0x03,0x10,0x00,0x40,					   //中断上传端点描述符
	//以下为接口1（数据接口）描述符
	0x09,0x04,0x01,0x00,0x02,0x0a,0x00,0x00,0x00,			 //数据接口描述符
	0x07,0x05,0x02,0x02,0x40,0x00,0x00,					   //端点描述符
	0x07,0x05,0x82,0x02,0x40,0x00,0x00,					   //端点描述符
};

// ----------------------------------------------------------------------------
// String descriptor
// ----------------------------------------------------------------------------

// Language descriptor
// TODO Spain / English
unsigned char __code g_DescriptorLanguage[] = { 0x04, 0x03, 0x09, 0x04 };

// Serial number string descriptor
unsigned char __code g_DescriptorSerial[] = {
	20, // Length of the whole array including this byte
	3,
	'2', 0, '0', 0, '1', 0, '9', 0, '-', 0, '3', 0, '-', 0, '1', 0, '7', 0
};

// Product string descriptor
unsigned char __code g_DescriptorProduct[] = {
	24, // Length of the whole array including this byte
	3,
	'S', 0, 't', 0, 'a', 0, 't', 0, 'e', 0, ' ', 0,
	'L', 0, 'i', 0, 'g', 0, 'h', 0, 't', 0
};

// Manufacturer string descriptor
unsigned char __code g_DescriptorManufacturer[] = {
	28, // Length of the whole array including this byte
	3,
	'A', 0, 'n', 0, 'd', 0, 'r', 0, 'e', 0, 'a', 0, 's', 0, ' ', 0,
	'B', 0, 'u', 0, 't', 0, 't', 0, 'i', 0
};

// CDC parameter
// The initial baud rate is 57600, 1 stop bit, no parity, 8 data bits.
__xdata uint8_t LineCoding[7] = { 0x00, 0xe1, 0x00, 0x00, 0x00, 0x00, 0x08 };



