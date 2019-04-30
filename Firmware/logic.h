/**
 * Main USB Logic
 */

#pragma once

#include <stdint.h>
#include <string.h>

#include <ch554.h>
#include <ch554_usb.h>
#include <debug.h>

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
