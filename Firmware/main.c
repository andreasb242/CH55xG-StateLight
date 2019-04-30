/**
 * State light main
 *
 * USB Serial controller to switch WS2812b LEDs
 *
 * Andreas Butt, (c) 2019
 * License: GPL
 */

#include "logic.h"

#include <debug.h>


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
	mainLoop();
}
