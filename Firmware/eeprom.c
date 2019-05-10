/**
 * Timer library used to blink or handle timeouts
 *
 * Andreas Butti, (c) 2019
 * License: GPL
 */

#include "inc.h"
#include "eeprom.h"

/**
 * Write data flash (EEPROM)
 *
 * @param addr Address to write, 0 ... 127
 * @param buf Buffer to write
 * @param len Length in bytes
 *
 * @return Bytes written
 */
uint8_t WriteDataFlash(uint8_t addr, uint8_t* buf, uint8_t len) {
	uint8_t i;
	SAFE_MOD = 0x55;
	SAFE_MOD = 0xAA;

	GLOBAL_CFG |= bDATA_WE;

	SAFE_MOD = 0;

	ROM_ADDR_H = DATA_FLASH_ADDR >> 8;
	addr <<= 1;

	for (i = 0; i < len; i++) {
		ROM_ADDR_L = addr + i * 2;
		ROM_DATA_L = *(buf + i);
		if (ROM_STATUS & bROM_ADDR_OK) {
			ROM_CTRL = ROM_CMD_WRITE;
		}
		if ((ROM_STATUS ^ bROM_ADDR_OK) > 0) {
			return i;
		}
	}

	SAFE_MOD = 0x55;
	SAFE_MOD = 0xAA;
	GLOBAL_CFG &= ~bDATA_WE;
	SAFE_MOD = 0;

	return i;
}

/**
 * Read data flash (EEPROM)
 *
 * @param addr Address to write, 0 ... 127
 * @param buf Buffer to put read data to
 * @param len Length in bytes
 *
 * @return Bytes read
 */
uint8_t ReadDataFlash(uint8_t addr, uint8_t len, uint8_t* buf) {
	uint8_t i;

	ROM_ADDR_H = DATA_FLASH_ADDR >> 8;
	addr <<= 1;
	for (i = 0; i < len; i++) {
		ROM_ADDR_L = addr + i * 2;
		ROM_CTRL = ROM_CMD_READ;
		*(buf + i) = ROM_DATA_L;
	}
	return i;
}
