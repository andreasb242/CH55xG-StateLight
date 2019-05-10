/**
 * Parser for commands
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#include "parser.h"
#ifndef _UNIT_TEST
#include "usb-cdc.h"
#endif

/**
 * Last parsed command / parameter
 */
ParserResult parserResult;

// GCC UnitTest or Productive firmware
#ifndef __code
#define __code
#endif

#define PAR_TYPE_RGB 1
#define PAR_TYPE_I_A 2
#define PAR_TYPE_I_B 3

// Product string descriptor
unsigned char __code g_Syntax[] = {
	'i',
	'a', PAR_TYPE_RGB,
	's', PAR_TYPE_I_A, PAR_TYPE_RGB,
	't', PAR_TYPE_I_A,
	'b', PAR_TYPE_I_A,
	'c', PAR_TYPE_I_A, PAR_TYPE_I_B,
	'r', PAR_TYPE_I_A,
	'w', PAR_TYPE_I_A,
	'p',
	0,
	255
};

/**
 * Parser State
 */
static uint8_t g_ParserState = 255;

/**
 * Parse a character
 *
 * @param c Char to parse
 */
void parseCmd(char c) {
	static uint8_t hexState = 0;
	static uint8_t tmpCol = 0;

	if (c == '\n' || c == '\r') {
		if (g_ParserState != 255) {
			parserExecuteCommand();

			g_ParserState = 255;
		}
		return;
	}

	if (g_ParserState == 255) {
		uint8_t i;

		parserResult.cmd = 0;
		parserResult.a = 0;
		parserResult.r = 0;
		parserResult.g = 0;
		parserResult.b = 0;
		hexState = 0;
		tmpCol = 0;

		for (i = 0; g_Syntax[i]; i++) {
			if (g_Syntax[i] == c) {
				parserResult.cmd = c;
				g_ParserState = i + 1;
				break;
			}
		}

		if (g_Syntax[g_ParserState] == 255) {
			UsbCdc_puts("Unknown CMD >");
			UsbCdc_putc(c);
			UsbCdc_puts("<\r\n");
		} else {
			g_ParserState == 255;
		}

		return;
	} else {
		if (g_Syntax[g_ParserState] > 5) {
			return;
		} else if (g_Syntax[g_ParserState] == PAR_TYPE_RGB) {
			tmpCol *= 16;
			if (c >= '0' && c <= '9') {
				tmpCol += c - '0';
			} else if (c >= 'A' && c <= 'F') {
				tmpCol += c - 'A' + 10;
			} else {
				parserResult.cmd = 0;
				g_ParserState == 255;
			}

			hexState++;

			if (hexState == 2) {
				parserResult.r = tmpCol;
				tmpCol = 0;
			}
			if (hexState == 4) {
				parserResult.g = tmpCol;
				tmpCol = 0;
			}
			if (hexState == 6) {
				parserResult.b = tmpCol;
				tmpCol = 0;
			}

			if (hexState >= 6) {
				g_ParserState++;
			}
		} else if (g_Syntax[g_ParserState] == PAR_TYPE_I_A) {
			if (c == ' ') {
				g_ParserState++;
				return;
			}

			if (c < '0' || c >= '9') {
				parserResult.cmd = 0;
				g_ParserState == 255;
			}

			parserResult.a *= 10;
			parserResult.a += c - '0';
		} else if (g_Syntax[g_ParserState] == PAR_TYPE_I_B) {
			if (c == ' ') {
				g_ParserState++;
				return;
			}

			if (c < '0' || c >= '9') {
				parserResult.cmd = 0;
				g_ParserState == 255;
			}

			parserResult.b *= 10;
			parserResult.b += c - '0';
		} else {
			UsbCdc_puts("PERR\r\n");
		}
	}
}


