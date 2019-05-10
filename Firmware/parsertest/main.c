/**
 * Parser Tests
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#include <stdio.h>

void UsbCdc_puts(char* s) {
	printf("%s", s);
}

void UsbCdc_putc(char c) {
	printf("%c", c);
}

#define _UNIT_TEST

#include "../parser.h"
#include "../parser.c"


uint8_t parseSuccess = 0;

/**
 * Parse a command
 */
void parseCommand(const char* str) {
	printf(">%s<\n", str);

	parseSuccess = 0;

	while(*str) {
		parseCmd(*str);
		str++;
	}
	parseCmd('\n');

	if (parseSuccess == 1) {
		printf("Parse success\n");
	} else {
		printf("#Parse failed!\n");
	}
}

/**
 * Command parsed successfully, execute it
 */
void parserExecuteCommand() {
	parseSuccess++;
}


/**
 * Test main
 */
void main() {
	// Info
	parseCommand("i");

	if (parserResult.cmd != 'i') {
		printf("#Command type wrong! %i\n", __LINE__);
	}

	// Turn off hints
	parseCommand("n");

	if (parserResult.cmd != 'n') {
		printf("#Command type wrong! %i\n", __LINE__);
	}

	// Ping
	parseCommand("p");

	if (parserResult.cmd != 'p') {
		printf("#Command type wrong! %i\n", __LINE__);
	}

	// Set all LEDs to HEX 112233
	parseCommand("a112233");
	if (parserResult.cmd != 'a') {
		printf("#Command type wrong! %i\n", __LINE__);
	}
	if (parserResult.r != 0x11 || parserResult.g != 0x22 || parserResult.b != 0x33) {
		printf("#RGB wrong! (%02x%02x%02x)%i\n", parserResult.r, parserResult.g, parserResult.b, __LINE__);
	}

	parseCommand("aAA00FF");
	if (parserResult.cmd != 'a') {
		printf("#Command type wrong! %i\n", __LINE__);
	}
	if (parserResult.r != 0xAA || parserResult.g != 0x00 || parserResult.b != 0xff) {
		printf("#RGB wrong! (%02x%02x%02x)%i\n", parserResult.r, parserResult.g, parserResult.b, __LINE__);
	}

	parseCommand("s1 AA00FF");
	if (parserResult.cmd != 's') {
		printf("#Command type wrong! %i\n", __LINE__);
	}
	if (parserResult.a != 1) {
		printf("#LED ID wrong! %i\n", __LINE__);
	}
	if (parserResult.r != 0xAA || parserResult.g != 0x00 || parserResult.b != 0xff) {
		printf("#RGB wrong! (%02x%02x%02x)%i\n", parserResult.r, parserResult.g, parserResult.b, __LINE__);
	}

	parseCommand("s25 AA00FF");
	if (parserResult.cmd != 's') {
		printf("#Command type wrong! %i\n", __LINE__);
	}
	if (parserResult.a != 25) {
		printf("#LED ID wrong! %i\n", __LINE__);
	}
	if (parserResult.r != 0xAA || parserResult.g != 0x00 || parserResult.b != 0xff) {
		printf("#RGB wrong! %i\n", __LINE__);
	}

	parseCommand("t20");
	if (parserResult.cmd != 't') {
		printf("#Command type wrong! %i\n", __LINE__);
	}
	if (parserResult.a != 20) {
		printf("#Blink time wrong! %i\n", __LINE__);
	}

	parseCommand("b255");
	if (parserResult.cmd != 'b') {
		printf("#Command type wrong! %i\n", __LINE__);
	}
	if (parserResult.a != 255) {
		printf("#Blink ID! %i\n", __LINE__);
	}

	parseCommand("b1");
	if (parserResult.cmd != 'b') {
		printf("#Command type wrong! %i\n", __LINE__);
	}
	if (parserResult.a != 1) {
		printf("#Blink ID! %i\n", __LINE__);
	}

	parseCommand("c1 2");
	if (parserResult.cmd != 'c') {
		printf("#Command type wrong! %i\n", __LINE__);
	}
	if (parserResult.a != 1) {
		printf("#Data wrong! (data=%i) %i\n", parserResult.a, __LINE__);
	}
	if (parserResult.b != 2) {
		printf("#Data wrong! (data=%i) %i\n", parserResult.b, __LINE__);
	}

	parseCommand("c255 255");
	if (parserResult.cmd != 'c') {
		printf("#Command type wrong! %i\n", __LINE__);
	}
	if (parserResult.a != 255) {
		printf("#Data wrong! %i\n", __LINE__);
	}
	if (parserResult.b != 255) {
		printf("#Data wrong! %i\n", __LINE__);
	}

	parseCommand("r5");
	if (parserResult.cmd != 'r') {
		printf("#Command type wrong! %i\n", __LINE__);
	}
	if (parserResult.a != 5) {
		printf("#Data wrong! %i\n", __LINE__);
	}


	printf("Finished.\n");
}
