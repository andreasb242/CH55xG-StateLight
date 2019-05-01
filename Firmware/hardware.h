/**
 * Hardware initialisation
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#pragma once

#include "inc.h"

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
