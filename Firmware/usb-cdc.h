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

void usb_poll();

/**
 * Receive data from USB and process it
 */
void uart_poll();
void v_uart_puts(char *str);
void virtual_uart_tx(uint8_t tdata);


/**
 * USB Interrupt Handler
 */
void usbInterrupt();
