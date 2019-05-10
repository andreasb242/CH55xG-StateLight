/**
 * EEPROM Access Library
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#pragma once

#include "inc.h"


/**
 * Write data flash (EEPROM)
 *
 * @param addr Address to write, 0 ... 127
 * @param buf Buffer to write
 * @param len Length in bytes
 *
 * @return Bytes written
 */
uint8_t WriteDataFlash(uint8_t addr, uint8_t* buf, uint8_t len);

/**
 * Read data flash (EEPROM)
 *
 * @param addr Address to write, 0 ... 127
 * @param buf Buffer to put read data to
 * @param len Length in bytes
 *
 * @return Bytes read
 */
uint8_t ReadDataFlash(uint8_t addr, uint8_t len, uint8_t* buf);
