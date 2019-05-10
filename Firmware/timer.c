/**
 * Timer library used to blink or handle timeouts
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#include "inc.h"
#include "timer.h"

/**
 * Internal timer
 */
volatile uint32_t g_Timer = 0;

/**
 * Setup Timer
 */
void timerSetup() {
	// set timer 0 mode 1
	TMOD = 0x01;
	TH0 = 0xFF;
	TL0 = 0xFF;

	// start timer 0
	TR0 = 1;

	// enable timer 0 interrupt
	ET0 = 1;
}

/**
 * Called from timer interrupt
 */
inline void timer0clock() {
	g_Timer++;
}
