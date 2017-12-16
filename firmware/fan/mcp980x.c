
#include "mcp980x.h"

#if SNS_I2C_SUPPORTED
extern uint8_t Sns_Temp_I2C_Present; // bits 0..7 represent which i2c sensor SnsType_Temp_I2C_Addr0..SnsType_Temp_I2C_Addr7 is detected.

// Wait for I2C (TWI) interface to become ready, or timeout.
// Returns 1 on ready, 0 on timeout.
static uint8_t waitI2CReady()
{
    uint16_t timeout_us = 10000;
    while (bit_is_clear(TWCR, TWINT))
    {
        if (timeout_us == 0)
            return 0;
        timeout_us--;
        _delay_us(1);
    };
    return 1;
}

// Read temperature via I2C. Returns -127 on error.
int8_t readI2CMcp980x(uint8_t addr)
{
    int8_t temp = -127;
    addr = (SNS_I2C_MCP980X_BASE_ADDR | (addr & 7)) << 1;

    waitI2CReady();
    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTA); // Send START condition
    waitI2CReady();
    TWDR = addr | 0;               // Send slave address & write operation
    TWCR = _BV(TWINT) | _BV(TWEN); // Clear TWINT bit in TWCR to start transmission of address
    if (!waitI2CReady())
        return temp;
    if ((TWSR & TW_STATUS_MASK) != TW_MT_SLA_ACK)
        return temp;               // Return error when slave did not ack.
    TWDR = 0x00;                   // Write byte to slave - temp. register
    TWCR = _BV(TWINT) | _BV(TWEN); // Clear TWINT bit in TWCR to start transmission of data
    waitI2CReady();
    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTA); // Send (rep.) START condition
    waitI2CReady();
    TWDR = addr | 1;               // Send slave address & read operation
    TWCR = _BV(TWINT) | _BV(TWEN); // Clear TWINT bit in TWCR to start transmission of address
    waitI2CReady();
    TWCR = _BV(TWINT) | _BV(TWEN) /* | _BV(TWEA)*/; // Clear TWINT bit in TWCR to start transmission of data
    waitI2CReady();
    temp = TWDR;
    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO); // Transmit STOP condition
    return temp;
}

void initI2C()
{
    uint8_t i;
// F_SCL = F_CPU / (16+2*TWBR*TWI_PRESCALE)
#define TWI_PRESCALE (1)
#define TWI_PRESCALE_BITS (0)

    TWSR = TWI_PRESCALE_BITS;
    TWBR = (F_CPU / FREQ_I2C_SCL - 16) / 2;

    // Determine which slaves are really present on the bus.
    // Try to read each one of them and the ones that respond
    // are marked as present.
    for (i = 0; i <= 7; i++)
    {
        if (readI2CMcp980x(i) > -127)
            Sns_Temp_I2C_Present |= 1 << i;
    }
}
#endif // SNS_I2C_SUPPORTED
