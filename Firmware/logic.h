/**
 * Main Logic to control LEDs
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#pragma once

#include "inc.h"

/**
 * Called from the main loop
 */
void logicLoop();

/**
 * Update the LEDs
 */
void updateLeds();

/**
 * Load colors from EEPROM, if valid data is there
 */
void loadColorsFromEeprom();
