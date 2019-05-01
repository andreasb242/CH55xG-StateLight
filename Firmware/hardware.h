/**
 * Hardware initialisation
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#pragma once

#include "inc.h"

// USB BUFFER -----------------------------------------------------------------

#undef DEFAULT_ENDP0_SIZE
#define DEFAULT_ENDP0_SIZE	64
#undef DEFAULT_ENDP1_SIZE
#define DEFAULT_ENDP1_SIZE	64

// Endpoint 0 OUT & IN buffer, must be an even address
extern __xdata __at (0x0000) uint8_t  Ep0Buffer[DEFAULT_ENDP0_SIZE];

// Endpoint 1 upload buffer
extern __xdata __at (0x0040) uint8_t  Ep1Buffer[DEFAULT_ENDP1_SIZE];

// Endpoint 2 IN & OUT buffer, must be an even address
extern __xdata __at (0x0080) uint8_t  Ep2Buffer[2 * MAX_PACKET_SIZE];

// ----------------------------------------------------------------------------


/**
 * USB device mode configuration
 */
void USBDeviceCfg();

/**
 * Enable USB Endpoint
 */
void USBDeviceEndPointCfg();

/**
 * USB device mode interrupt initialization
 */
void USBDeviceIntCfg();

/**
 * Print string to Serial 0
 */
void print(const char* str);

/**
 * This function provided a way to access the internal bootloader
 */
void jumpToBootloader();

