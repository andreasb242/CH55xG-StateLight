/**
 * USB CDC controlled W2812b LEDs
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#include "inc.h"
#include "usb-cdc.h"
#include "hardware.h"

/**
 * Programm main
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

	// Main loop
	while (1) {
		usbCdcLoop();
	}
}
