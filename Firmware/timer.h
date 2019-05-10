/**
 * Timer library used to blink or handle timeouts
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#pragma once

#include "inc.h"

/**
 * Internal timer
 */
extern volatile uint32_t g_Timer;

/**
 * Setup Timer
 */
void timerSetup();

/**
 * Called from timer interrupt
 */
inline void timer0clock();
