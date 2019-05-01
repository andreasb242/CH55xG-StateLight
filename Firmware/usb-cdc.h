/**
 * USB CDC Implementation
 * Based on the WCH Example
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#pragma once

#include "inc.h"

/**
 * Send a string over the virtual UART
 *
 * @param str Null Terminated String
 */
void v_uart_puts(char* str);

/**
 * Sed a byte over the vurtual UART
 *
 * @param tdata Byte to send
 */
void virtual_uart_tx(uint8_t tdata);



