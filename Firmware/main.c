/**
 * USB CDC controlled W2812b LEDs
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#include "inc.h"
#include "hardware.h"
#include "bitbang.h"
#include "logic.h"
#include "usb-cdc.h"

/**
 * Interrupt needs to be here in the Main file
 * else it simple won't be called.
 */
void DeviceInterrupt(void) __interrupt (INT_NO_USB) {
	usbInterrupt();
}

/**
 * Firmware main
 */
void main() {
	// CH55x clock selection configuration
	CfgFsys();

	// Modify the main frequency and wait for the internal crystal to stabilize.
	mDelaymS(5);

	// Serial port 0, can be used for debugging
	mInitSTDIO();

	PRINT_DBG("UART init");

    bitbangSetup();
    updateLeds();

	PRINT_DBG("bitbangSetup");

	// Enable USB Port
	USBDeviceCfg();

	PRINT_DBG("USBDeviceCfg");

	// Endpoint configuration
	USBDeviceEndPointCfg();

	PRINT_DBG("USBDeviceEndPointCfg");

	// Interrupt initialization
	USBDeviceIntCfg();

	PRINT_DBG("USBDeviceIntCfg");

	UEP0_T_LEN = 0;

	// Pre-use send length must be cleared
	UEP1_T_LEN = 0;

	// Pre-use send length must be cleared
	UEP2_T_LEN = 0;

	// Main Loop
	while(1) {
		PRINT_DBG("l");

	    P3_2 = 1;
		usb_poll();
	    P3_2 = 0;

//		v_uart_puts("test1\r\n");
		uart_poll();

		logicLoop();
	}
}
