/**
 * Main USB Logic
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#pragma once

#include <stdint.h>
#include <string.h>

#include <ch554.h>
#include <ch554_usb.h>
#include <debug.h>

#include <8051.h>


// Ignore some defines for the IDE,
// make sure this is defined in your IDE,
// but not in the makefile, so this is
// ignored for the build
#ifdef IDE_ENVIRONMENT
#define __xdata
#define __at(x)
#define __code
#define __idata
#define __interrupt(x)
#define SBIT(a, b, c)
#endif

//#define PRINT_DBG(str) print(str "\r\n")
#define PRINT_DBG(str)

/**
 * USB device mode configuration
 */
void USBDeviceCfg();

/**
 * Enable USB Endpoint
 */
void USBDeviceEndPointCfg();

/**
 * Main Loop
 */
void mainLoop();

/**
 * USB device mode interrupt initialization
 */
void USBDeviceIntCfg();
