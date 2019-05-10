/**
 * USB CDC Implementation
 * Based on the example from
 * https://github.com/Blinkinlabs/ch554_sdcc/tree/master/examples/usb_device_cdc_i2c
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#pragma once

#include "inc.h"

/**
 * Send usb data from buffer
 */
void UsbCdc_processOutput();

/**
 * Receive data from USB and process it, process only one byte at once
 */
void UsbCdc_processInput();

/**
 * Send one byte over USB CDC Serial port
 */
void UsbCdc_putc(uint8_t tdata);

/**
 * Send uint8_t over CDC Serial port
 */
void UsbCdc_puti(uint8_t value);

/**
 * Send 0 terminated string over USB CDC Serial port
 */
void UsbCdc_puts(char* str);

/**
 * USB Interrupt Handler
 */
void usbInterrupt();
