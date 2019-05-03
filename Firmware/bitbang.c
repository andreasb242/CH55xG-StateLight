/**
 * WS2812b Implementation, from
 * https://github.com/Blinkinlabs/ch554_sdcc/tree/master/examples/ws2812
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */


#include "bitbang.h"
#include "inc.h"

/**
 * Use Pin 1.7 as LED Output
 */
#define LED_PIN 7

/**
 * Map LED Output
 */
SBIT(LED, 0x90, LED_PIN);

/**
 * Setup pin as output
 */
void bitbangSetup() {
	// Configure pin 1.7 as GPIO output
	P1_DIR_PU &= 0x0C;
	P1_MOD_OC = P1_MOD_OC & ~(1 << LED_PIN);
	P1_DIR_PU = P1_DIR_PU | (1 << LED_PIN);
}

/**
 * Write out the LED Datat to the defined pin
 */
void bitbangWs2812(uint8_t ledCount, __xdata uint8_t* ledData) {
	ledCount;
	ledData;

	// Bitbang routine
	// Input parameters: (determined by compilation)
	// * byteCount should be allocated in dpl
	// * ledData should be allocated with name '_bitbangWs2812_PARAM_2'

	// Strategy:
	// * Keep the data memory pointer in DPTR
	// * Keep ledCount in r2
	// * Store bitCount in r3
	// * Store the current data variable in ACC

	__asm

	// Load the LED count into r2
	mov r2, dpl

	// Load the LED data start address into DPTR
	mov dpl, _bitbangWs2812_PARM_2
	mov dph, (_bitbangWs2812_PARM_2 + 1)

	// byte loop
	00001$:

	// ***Red byte***
	// Load the current LED data value into the accumulator (1)
	movx a,@dptr
	// and advance the counter for the next LED data value (1)
	inc dptr

	// Set up the bit loop (2)
	mov r3, #8
	// red bit loop
	00002$:
	// Begin bit cycle- set bit high (2)
	setb _LED

	// Tune this count by hand, want ~.4uS (1*2)
	nop
	nop

	// Shift the LED data value left to get the high bit (1)
	rlc A
	// Set the output bit high if the current bit is high, (2)
	// otherwise set it low
	mov _LED, C

	// Tune this count by hand, want ~.4uS (1*2)
	nop
	nop

	// final part of bit cycle, set bit low (2)
	clr _LED

	// Tune this count by hand, want ~.45uS
	// nop
	// nop

	// If there are more bits in this byte (2, ?)
	djnz r3, 00002$

	// *** Green byte ***
	// Load the current LED data value into the accumulator (1)
	movx a,@dptr
	// and advance the counter for the next LED data value (1)
	inc dptr

	// Set up the bit loop (2)
	mov r3, #8
	// red bit loop
	00003$:
	// Begin bit cycle- set bit high (2)
	setb _LED

	// Tune this count by hand, want ~.4uS (1*2)
	nop
	nop

	// Shift the LED data value left to get the high bit (1)
	rlc A
	// Set the output bit high if the current bit is high, (2)
	// otherwise set it low
	mov _LED, C

	// Tune this count by hand, want ~.4uS (1*2)
	nop
	nop

	// final part of bit cycle, set bit low (2)
	clr _LED

	// Tune this count by hand, want ~.45uS
	// nop
	// nop

	// If there are more bits in this byte (2, ?)
	djnz r3, 00003$

	// *** Blue byte ***
	// Load the current LED data value into the accumulator (1)
	movx a,@dptr
	// and advance the counter for the next LED data value (1)
	inc dptr

	// Set up the bit loop (2)
	mov r3, #8
	// red bit loop
	00004$:
	// Begin bit cycle- set bit high (2)
	setb _LED

	// Tune this count by hand, want ~.4uS (1*2)
	nop
	nop

	// Shift the LED data value left to get the high bit (1)
	rlc A
	// Set the output bit high if the current bit is high, (2)
	// otherwise set it low
	mov _LED, C

	// Tune this count by hand, want ~.4uS (1*2)
	nop
	nop

	// final part of bit cycle, set bit low (2)
	clr _LED

	// Tune this count by hand, want ~.45uS
	// nop
	// nop

	// If there are more bits in this byte (2, ?)
	djnz r3, 00004$


	// If there are more LEDs (2, ?)
	djnz r2, 00001$

	__endasm;
}
