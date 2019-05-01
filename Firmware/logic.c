/**
 * Main Logic to control LEDs
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#include "inc.h"
#include "logic.h"
#include "bitbang.h"


#define LED_COUNT (6)
__xdata uint8_t led_data[LED_COUNT * 3] = {
	0x55, 0, 0,
	0x55, 0, 0,
	0x55, 0x55, 0x55,
	0x55, 0x55, 0x55,
	0x55, 0, 0,
	0x55, 0, 0
};

/**
 * Received command bytes, process it
 */
void processCommandByte(char cmd) {
	if (cmd == 'r') {
		uint8_t i;
		for (i = 0; i < 18; i++) {
			led_data[i] = 0;
		}

		led_data[0] = 0xff;
		v_uart_puts("red\r\n");

	} else if (cmd == 'g') {
		uint8_t i;
		for (i = 0; i < 18; i++) {
			led_data[i] = 0;
		}

		led_data[1] = 0xff;
		v_uart_puts("green\r\n");

	} else if (cmd == 'b') {
		uint8_t i;
		for (i = 0; i < 18; i++) {
			led_data[i] = 0;
		}

		led_data[2] = 0xff;
		v_uart_puts("blue\r\n");
	}

	v_uart_puts("OK\r\n");
    bitbangWs2812(LED_COUNT, led_data);
}


/**
 * Called from the main loop
 */
void logicLoop() {
}


