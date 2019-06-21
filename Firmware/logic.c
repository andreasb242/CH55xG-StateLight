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
#include "eeprom.h"
#include "parser.h"
#include "eeprom.h"
#include "timer.h"


#define LED_COUNT 6

/**
 * Blink time
 */
uint8_t g_BlinkTime = 5;

/**
 * Timeout for watchdog in Seconds
 */
uint8_t g_WatchdogTimeout = 0;

/**
 * When the watchdog timeout ends
 */
uint16_t g_NextWatchdogTimeout = 0;

/**
 * When blink needs to be performed next time
 */
uint16_t g_NextBlinkTime = 0;

/**
 * Blink state
 */
uint8_t g_BlinkState = 1;

/**
 * Standby / On
 */
uint8_t g_LedsOn = 1;

/**
 * Bitmask for LEDs who are blinking
 */
uint8_t g_LedBlink = 0;

/**
 * LED RGB Data
 */
__xdata uint8_t g_LedData[LED_COUNT * 3] = {
	0x00, 0x05, 0x00,
	0x00, 0x05, 0x00,
	0x05, 0x05, 0x05,
	0x05, 0x05, 0x05,
	0x00, 0x05, 0x00,
	0x00, 0x05, 0x00
};

/**
 * LED RGB Data
 */
__xdata uint8_t g_LedDataTmp[LED_COUNT * 3] = {
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0
};

/**
 * Load colors from EEPROM, if valid data is there
 */
void loadColorsFromEeprom() {
	uint8_t a = 0;
	uint8_t i;
	uint8_t tmp[3];
	ReadDataFlash(23, 1, &a);

	if (a != '>') {
		return;
	}

	for (i = 0; i < LED_COUNT; i++) {
		ReadDataFlash(5 + i * 3, 3, tmp);
		g_LedData[i * 3 + 0] = tmp[1];
		g_LedData[i * 3 + 1] = tmp[0];
		g_LedData[i * 3 + 2] = tmp[2];
	}
}

/**
 * Blink the LEDs
 */
void blinkLeds();

/**
 * Update the LEDs
 */
void updateLeds() {
    bitbangWs2812(LED_COUNT, g_LedData);
}

/**
 * Command parsed successfully, execute it
 */
void parserExecuteCommand() {
	uint8_t i;
	uint8_t v;

	g_NextWatchdogTimeout = g_Timer + g_WatchdogTimeout;
	if (!g_LedsOn) {
		g_LedsOn = 1;
		blinkLeds();
	}

	switch (parserResult.cmd) {
	case 'i':
		UsbCdc_puts("Ampel 1.1,protocol: 2,Andreas Butti - 2019\r\n");
		return;

	case 'a':
		for (i = 0; i < LED_COUNT; i++) {
			g_LedData[i * 3 + 0] = parserResult.g;
			g_LedData[i * 3 + 1] = parserResult.r;
			g_LedData[i * 3 + 2] = parserResult.b;
		}

		g_LedBlink = 0;

		updateLeds();
		break;

	case 's':
		if (parserResult.a >= LED_COUNT) {
			UsbCdc_puts("NOK\r\n");
			return;
		}

		g_LedBlink &= ~(1 << parserResult.a);

		g_LedData[parserResult.a * 3 + 0] = parserResult.g;
		g_LedData[parserResult.a * 3 + 1] = parserResult.r;
		g_LedData[parserResult.a * 3 + 2] = parserResult.b;
		updateLeds();
		break;

	case 't':
		g_BlinkTime = parserResult.a;
		break;

	case 'w':
		g_WatchdogTimeout = parserResult.a;
		break;

	case 'b':
		if (parserResult.a == 255) {
			g_LedBlink = 0xff;
		} else {
			g_LedBlink |= (1 << parserResult.a);
		}
		break;

	case 'c':
		if (ReadDataFlash(parserResult.a, 1, &v) == 1) {
			if (v == parserResult.b) {
				// Already the correct value there
				UsbCdc_puts("OK\r\n");
				return;
			}
		}

		if (WriteDataFlash(parserResult.a, &parserResult.b, 1) == 1) {
			UsbCdc_puts("OK\r\n");
		} else {
			UsbCdc_puts("NOK\r\n");
		}
		return;

	case 'r':
		if (ReadDataFlash(parserResult.a, 1, &v) == 1) {
			UsbCdc_puts("OK ");
			UsbCdc_puti(v);
			UsbCdc_puts("\r\n");
		} else {
			UsbCdc_puts("NOK\r\n");
		}
		return;

	case 'p':
		// Ping, does nothing
		break;

	default:
		UsbCdc_puts("NOK\r\n");
		return;
	}

	UsbCdc_puts("OK\r\n");
}

/**
 * Blink the LEDs
 */
void blinkLeds() {
	uint8_t i;

	// Clear tmp data
	for (i = 0; i < LED_COUNT * 3; i++) {
		g_LedDataTmp[i] = 0;
	}

	if (!g_LedsOn) {
		bitbangWs2812(LED_COUNT, g_LedDataTmp);
	} else if (!g_BlinkState) {
		for (i = 0; i < LED_COUNT; i++) {
			if ((g_LedBlink & (1 << i)) == 0) {
				g_LedDataTmp[i * 3 + 0] = g_LedData[i * 3 + 0];
				g_LedDataTmp[i * 3 + 1] = g_LedData[i * 3 + 1];
				g_LedDataTmp[i * 3 + 2] = g_LedData[i * 3 + 2];
			}
		}

		bitbangWs2812(LED_COUNT, g_LedDataTmp);
	} else {
		bitbangWs2812(LED_COUNT, g_LedData);
	}

	updateLeds();
}

/**
 * Turn OFF all LEDs, until an USB Command is received
 */
void turnOffLeds() {
	g_LedsOn = 0;
	blinkLeds();
}

/**
 * Called from the main loop
 */
void logicLoop() {
	// Handle LED blinking
	uint16_t diff = g_NextBlinkTime - g_Timer;
	if (diff < 1 || diff > 255) {
		g_NextBlinkTime = g_Timer + g_BlinkTime;
		g_BlinkState = !g_BlinkState;

		blinkLeds();
	}

	// Handle sleep, if there is a timeout reached
	if (g_WatchdogTimeout && g_NextWatchdogTimeout == g_Timer) {
		turnOffLeds();
	}
}


