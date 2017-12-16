#pragma once
#include "i2cmaster.h"
#include <inttypes.h>

/**************************************************************************/
/*!
    @brief  Sends a single command byte over I2C
*/
/**************************************************************************/
void wireWriteRegister(uint8_t addr, uint8_t reg, uint16_t value);

/**************************************************************************/
/*!
    @brief  Reads a 16 bit values over I2C
*/
/**************************************************************************/
void wireReadRegister(uint8_t addr, uint8_t reg, uint16_t *value);
