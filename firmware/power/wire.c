
#include "wire.h"

/**************************************************************************/
/*!
    @brief  Sends a single command byte over I2C
*/
/**************************************************************************/
void wireWriteRegister(uint8_t addr, uint8_t reg, uint16_t value)
{
    i2c_start_wait(addr + I2C_WRITE); // set device address and write mode

    i2c_write(reg);                 // Register
    i2c_write((value >> 8) & 0xFF); // Upper 8-bits
    i2c_write(value & 0xFF);        // Lower 8-bits

    i2c_stop();

    /*
        Wire.beginTransmission(addr);
        Wire.write(reg);                 // Register
        Wire.write((value >> 8) & 0xFF); // Upper 8-bits
        Wire.write(value & 0xFF);        // Lower 8-bits
        Wire.endTransmission();
    */
}

/**************************************************************************/
/*!
    @brief  Reads a 16 bit values over I2C
*/
/**************************************************************************/
void wireReadRegister(uint8_t addr, uint8_t reg, uint16_t *value)
{
	i2c_start_wait(addr + I2C_WRITE); // set device address and write mode
	i2c_write(reg);                 // Register

    i2c_rep_start(addr + I2C_READ); // set device address and read mode
    *value = (((uint16_t)i2c_readAck() << 8) | i2c_readNak());
    i2c_stop();

    /*
    Wire.beginTransmission(addr);
    Wire.write(reg); // Register
    Wire.endTransmission();

    delay(1); // Max 12-bit conversion time is 586us per sample

    Wire.requestFrom(ina219_i2caddr, (uint8_t)2);
    // Shift values to create properly formed integer
    *value = ((Wire.read() << 8) | Wire.read());
	*/
}

