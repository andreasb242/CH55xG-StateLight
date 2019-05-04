/**
 * Main Logic to control LEDs
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#include "inc.h"
#include "logic.h"
#include "bitbang.h"
#include "usb-cdc.h"

#define LED_COUNT 6

__xdata uint8_t g_LedData[LED_COUNT * 3] = {
	0, 0x05, 0,
	0, 0x05, 0,
	0x05, 0x05, 0x05,
	0x05, 0x05, 0x05,
	0, 0x05, 0,
	0, 0x05, 0
};

/**
 * Turn help feedback on / off
 */
uint8_t g_helpFeedback = 1;

/**
 * Update the LEDs
 */
void updateLeds() {
    bitbangWs2812(LED_COUNT, g_LedData);
}

/**
 * Received command bytes, process it
 *
 * Accept the following commands:
 * i: Print the information
 * s12=AABBCC\n: Set LED 12 Color to AABBCC
 * aAABBCC\n: Set all LEDs to AABBCC
 */
void processCommandByte(char cmd) {
	static uint8_t state = 0;
	static uint8_t ledId = 0;
	static uint8_t tmpCol = 0;
	static uint8_t colIndex = 0;
	static uint8_t allLeds = 0;

	uint8_t error = 0;

	if (state == 1) {
		if (cmd == '=') {
			ledId--;
			if (ledId >= LED_COUNT) {
				error = 1;
			} else {
				state = 2;
				if (g_helpFeedback) {
					UsbCdc_puts("\r\nColor:\r\n");
				}
				return;
			}
		}

		if (cmd >= '0' && cmd <= '9') {
			ledId *= 10;
			ledId += cmd - '0';

			if (g_helpFeedback) {
				UsbCdc_putc(cmd);
			}
			return;
		} else {
			error = 1;
		}
	}

	if (state == 2) {
		tmpCol *= 16;

		if (cmd == '\r' || cmd == '\n') {
			if (allLeds) {
				uint8_t i;
				for (i = 1; i < LED_COUNT; i++) {
					g_LedData[i * 3 + 0] = g_LedData[0];
					g_LedData[i * 3 + 1] = g_LedData[1];
					g_LedData[i * 3 + 2] = g_LedData[2];
				}
			}

			updateLeds();
			UsbCdc_puts("LEDs set\r\n");
			state = 0;
			ledId = 0;
			tmpCol = 0;
			colIndex = 0;
			allLeds = 0;
			return;
		}

		if (cmd >= '0' && cmd <= '9') {
			tmpCol += cmd - '0';
		} else if (cmd >= 'A' && cmd <= 'F') {
			tmpCol += cmd - 'A' + 10;
		} else {
			error = 1;
		}

		if (colIndex > 5) {
			error = 1;
		}

		if (!error) {
			colIndex++;

			if (g_helpFeedback) {
				UsbCdc_putc(cmd);
			}

			if (colIndex == 2) {
				g_LedData[ledId * 3 + 1] = tmpCol;
				tmpCol = 0;
			}
			if (colIndex == 4) {
				g_LedData[ledId * 3 + 0] = tmpCol;
				tmpCol = 0;
			}
			if (colIndex == 6) {
				g_LedData[ledId * 3 + 2] = tmpCol;
				tmpCol = 0;
			}

			return;
		}
	}

	if (error) {
		UsbCdc_puts("Syntax Error\r\n");
		state = 0;
		ledId = 0;
		tmpCol = 0;
		colIndex = 0;
		allLeds = 0;
	}

	if (cmd == 'i') {
		UsbCdc_puts("Ampel 1.0, protcol: 1,Andreas Butti - 2019\r\n");
	} else if (cmd == 'n') {
		g_helpFeedback = 0;
		UsbCdc_puts("OK\r\n");
	} else if (cmd == 's') {
		state = 1;
		if (g_helpFeedback) {
			UsbCdc_puts("Set LED:\r\n");
		}
	} else if (cmd == 'a') {
		state = 2;
		if (g_helpFeedback) {
			UsbCdc_puts("All Color:\r\n");
		}
		allLeds = 1;
	} else if (cmd == '\n' || cmd == '\r') {
		// Ignore
	} else {
		UsbCdc_puts("Unknown CMD >");
		UsbCdc_putc(cmd);
		UsbCdc_puts("<\r\n");
	}
}


/**
 * Called from the main loop
 */
void logicLoop() {
}


