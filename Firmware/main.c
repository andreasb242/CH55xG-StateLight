/**
 * USB CDC controlled W2812b LEDs
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#include "inc.h"
#include "hardware.h"
#include "usb-cdc.h"

/**
 * This function is called in the loop
 */
void loop() {
    P3_2 = 1;
    P3_2 = 0;
	v_uart_puts("test1\r\n");
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

	// Enable USB Port
	USBDeviceCfg();

	// Endpoint configuration
	USBDeviceEndPointCfg();

	// Interrupt initialization
	USBDeviceIntCfg();

	UEP0_T_LEN = 0;

	// Pre-use send length must be cleared
	UEP1_T_LEN = 0;

	// Pre-use send length must be cleared
	UEP2_T_LEN = 0;

	// Main Loop
	while(1) {
		loop();
		usb_poll();
		uart_poll();
	}
}
