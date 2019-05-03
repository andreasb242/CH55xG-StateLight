/**
 * WS2812b Implementation, from
 * https://github.com/Blinkinlabs/ch554_sdcc/tree/master/examples/ws2812
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#pragma once

#include "inc.h"

/**
 * Setup pin as output
 */
void bitbangSetup();

/**
 * Write out the LED Datat to the defined pin
 */
void bitbangWs2812(uint8_t ledCount, __xdata uint8_t* ledData);
