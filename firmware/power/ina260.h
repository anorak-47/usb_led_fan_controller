#pragma once
#include "config.h"
#include <inttypes.h>

/**************************************************************************/
/*!
        @file     Thanos-INA260.h
        @author   Thanos Kontogiannis
        @license  BSD (see license.txt)

        v1.0  - First release
*/
/**************************************************************************/

#if POWER_METER_INA260

/*=========================================================================
    I2C ADDRESS/BITS
    Adress range is 1000 0000 (A0+A1=GND)
                 to 1001 1110 (A0+A1=SCL)
    -----------------------------------------------------------------------*/
#define INA260_ADDRESS (0x80) // 1000 0000 (A0+A1=GND)
/*=========================================================================*/

/*=========================================================================
    CONFIG REGISTER (R/W)
    -----------------------------------------------------------------------*/
#define INA260_REG_CONFIG (0x00)
/*---------------------------------------------------------------------*/
#define INA260_CONFIG_RESET (0x8000) // Reset Bit

#define INA260_CONFIG_AVGRANGE_MASK (0x0E00) // Averaging mode Mask
#define INA260_CONFIG_AVGRANGE_1 (0x0000)    // Average mode 1
#define INA260_CONFIG_AVGRANGE_4 (0x0200)    // Average mode 4
#define INA260_CONFIG_AVGRANGE_16 (0x0400)   // Average mode 16
#define INA260_CONFIG_AVGRANGE_64 (0x0600)   // Average mode 64
#define INA260_CONFIG_AVGRANGE_128 (0x0800)  // Average mode 128
#define INA260_CONFIG_AVGRANGE_256 (0x0A00)  // Average mode 256
#define INA260_CONFIG_AVGRANGE_512 (0x0C00)  // Average mode 512
#define INA260_CONFIG_AVGRANGE_1024 (0x0E00) // Average mode 1024

#define INA260_CONFIG_BVOLTAGETIME_MASK (0x01C0)   // Bus Voltage Conversion Time Mask
#define INA260_CONFIG_BVOLTAGETIME_140US (0x0000)  // 140us
#define INA260_CONFIG_BVOLTAGETIME_1100US (0x0100) // 1.1ms
#define INA260_CONFIG_BVOLTAGETIME_8244US (0x01C0) // 8.244ms

#define INA260_CONFIG_SCURRENTTIME_MASK (0x0038)   // Shunt Current Conversion Time Mask
#define INA260_CONFIG_SCURRENTTIME_140US (0x0000)  // 140us
#define INA260_CONFIG_SCURRENTTIME_1100US (0x0020) // 1.1ms
#define INA260_CONFIG_SCURRENTTIME_8244US (0x0038) // 8.244ms

#define INA260_CONFIG_MODE_MASK (0x0007) // Operating Mode Mask
#define INA260_CONFIG_MODE_POWERDOWN (0x0000)
#define INA260_CONFIG_MODE_SVOLT_TRIGGERED (0x0001)
#define INA260_CONFIG_MODE_BVOLT_TRIGGERED (0x0002)
#define INA260_CONFIG_MODE_SANDBVOLT_TRIGGERED (0x0003)
#define INA260_CONFIG_MODE_ADCOFF (0x0004)
#define INA260_CONFIG_MODE_SVOLT_CONTINUOUS (0x0005)
#define INA260_CONFIG_MODE_BVOLT_CONTINUOUS (0x0006)
#define INA260_CONFIG_MODE_SANDBVOLT_CONTINUOUS (0x0007)

/*=========================================================================*/

/*=========================================================================
    CURRENT REGISTER (R)
    -----------------------------------------------------------------------*/
#define INA260_REG_CURRENT (0x01)
/*=========================================================================*/

/*=========================================================================
    BUS VOLTAGE REGISTER (R)
    -----------------------------------------------------------------------*/
#define INA260_REG_BUSVOLTAGE (0x02)
/*=========================================================================*/

/*=========================================================================
    POWER REGISTER (R)
    -----------------------------------------------------------------------*/
#define INA260_REG_POWER (0x03)
/*=========================================================================*/

/*=========================================================================
    CALIBRATION REGISTER (R/W)
    -----------------------------------------------------------------------*/
#define INA260_MASK_ENABLE (0x06)
/*=========================================================================*/

/*=========================================================================
    ALERT LIMIT REGISTER (R/W)
    -----------------------------------------------------------------------*/
#define INA260_ALERT_LIMIT (0x07)
/*=========================================================================*/

/*=========================================================================
    ID REGISTER (R	)
    -----------------------------------------------------------------------*/
#define INA260_MANUFACTURER_ID (0xFE)
#define INA260_DIE_ID (0xFF)

#define INA260_DEFAULT_MANUFACTURER_ID 0x5449 // TI in ASCII
#define INA260_DEFAULT_DIE_ID 0x2270

/*=========================================================================*/

struct _s_ina260_device
{
	uint8_t i2caddr;
};

typedef struct _s_ina260_device _ina260_device;

extern _ina260_device ina260_device[MAX_POWERMETER];

void ina260_init(_ina260_device *dev, uint8_t addr);

uint32_t ina260_getBusVoltage_mV(_ina260_device *dev);
uint32_t ina260_getCurrent_mA(_ina260_device *dev);
uint32_t ina260_getPower_mW(_ina260_device *dev);

#endif
