/**
 * Main Logic to control LEDs
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#pragma once

#include "inc.h"

/**
 * Received command bytes, process it
 */
void processCommandByte(char cmd);

/**
 * Called from the main loop
 */
void logicLoop();
