/**
 * Parser for commands
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#pragma once

#include <stdint.h>
#include <string.h>

/**
 * Parser Struct result
 */
typedef struct {

	/**
	 * Command to execute
	 */
	uint8_t cmd;

	/**
	 * First parameter
	 */
	uint8_t a;

	/**
	 * Red
	 */
	uint8_t r;

	/**
	 * Green
	 */
	uint8_t g;

	/**
	 * Blue / Second parameter
	 */
	uint8_t b;


} ParserResult;

/**
 * Last parsed command / parameter
 */
extern ParserResult parserResult;

/**
 * Parse a character
 *
 * @param c Char to parse
 */
void parseCmd(char c);

/**
 * Command parsed successfully, execute it
 */
void parserExecuteCommand();


