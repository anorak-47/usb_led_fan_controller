/**************************************************************************/
/*!
    @file     Adafruit_INA219.cpp
    @author   K.Townsend (Adafruit Industries)
        @license  BSD (see license.txt)

        Driver for the INA219 current sensor

        This is a library for the Adafruit INA219 breakout
        ----> https://www.adafruit.com/products/???

        Adafruit invests time and resources providing this open source code,
        please support Adafruit and open-source hardware by purchasing
        products from Adafruit!

        @section  HISTORY

    v1.0 - First release
*/
/**************************************************************************/

#include "ina219.h"
#include "wire.h"
#include "debug.h"

#if POWER_METER_INA219

_ina219_device ina219_device[MAX_POWERMETER];

/**************************************************************************/
/*!
    @brief  Configures to INA219 to be able to measure up to 32V and 2A
            of current.  Each unit of current corresponds to 100uA, and
            each unit of power corresponds to 2mW. Counter overflow
            occurs at 3.2A.

    @note   These calculations assume a 0.1 ohm resistor is present
*/
/**************************************************************************/
void ina219_setCalibration_32V_2A(_ina219_device *dev)
{
    // By default we use a pretty huge range for the input voltage,
    // which probably isn't the most appropriate choice for system
    // that don't use a lot of power.  But all of the calculations
    // are shown below if you want to change the settings.  You will
    // also need to change any relevant register settings, such as
    // setting the VBUS_MAX to 16V instead of 32V, etc.

    // VBUS_MAX = 32V             (Assumes 32V, can also be set to 16V)
    // VSHUNT_MAX = 0.32          (Assumes Gain 8, 320mV, can also be 0.16, 0.08, 0.04)
    // RSHUNT = 0.1               (Resistor value in ohms)

    // 1. Determine max possible current
    // MaxPossible_I = VSHUNT_MAX / RSHUNT
    // MaxPossible_I = 3.2A

    // 2. Determine max expected current
    // MaxExpected_I = 2.0A

    // 3. Calculate possible range of LSBs (Min = 15-bit, Max = 12-bit)
    // MinimumLSB = MaxExpected_I/32767
    // MinimumLSB = 0.000061              (61uA per bit)
    // MaximumLSB = MaxExpected_I/4096
    // MaximumLSB = 0,000488              (488uA per bit)

    // 4. Choose an LSB between the min and max values
    //    (Preferrably a roundish number close to MinLSB)
    // CurrentLSB = 0.0001 (100uA per bit)

    // 5. Compute the calibration register
    // Cal = trunc (0.04096 / (Current_LSB * RSHUNT))
    // Cal = 4096 (0x1000)

    dev->calValue = 4096;

    // 6. Calculate the power LSB
    // PowerLSB = 20 * CurrentLSB
    // PowerLSB = 0.002 (2mW per bit)

    // 7. Compute the maximum current and shunt voltage values before overflow
    //
    // Max_Current = Current_LSB * 32767
    // Max_Current = 3.2767A before overflow
    //
    // If Max_Current > Max_Possible_I then
    //    Max_Current_Before_Overflow = MaxPossible_I
    // Else
    //    Max_Current_Before_Overflow = Max_Current
    // End If
    //
    // Max_ShuntVoltage = Max_Current_Before_Overflow * RSHUNT
    // Max_ShuntVoltage = 0.32V
    //
    // If Max_ShuntVoltage >= VSHUNT_MAX
    //    Max_ShuntVoltage_Before_Overflow = VSHUNT_MAX
    // Else
    //    Max_ShuntVoltage_Before_Overflow = Max_ShuntVoltage
    // End If

    // 8. Compute the Maximum Power
    // MaximumPower = Max_Current_Before_Overflow * VBUS_MAX
    // MaximumPower = 3.2 * 32V
    // MaximumPower = 102.4W

    // Set multipliers to convert raw current/power values
    dev->currentDivider_mA = 10; // Current LSB = 100uA per bit (1000/100 = 10)
    dev->powerDivider_mW = 2;    // Power LSB = 1mW per bit (2/1)

    // Set Calibration register to 'Cal' calculated above
    wireWriteRegister(dev->i2caddr, INA219_REG_CALIBRATION, dev->calValue);

    // Set Config register to take into account the settings above
    uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V | INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
                      INA219_CONFIG_SADCRES_12BIT_1S_532US | INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
    wireWriteRegister(dev->i2caddr, INA219_REG_CONFIG, config);
}

/**************************************************************************/
/*!
    @brief  Configures to INA219 to be able to measure up to 32V and 1A
            of current.  Each unit of current corresponds to 40uA, and each
            unit of power corresponds to 800µW. Counter overflow occurs at
            1.3A.

    @note   These calculations assume a 0.1 ohm resistor is present
*/
/**************************************************************************/
void ina219_setCalibration_32V_1A(_ina219_device *dev)
{
    // By default we use a pretty huge range for the input voltage,
    // which probably isn't the most appropriate choice for system
    // that don't use a lot of power.  But all of the calculations
    // are shown below if you want to change the settings.  You will
    // also need to change any relevant register settings, such as
    // setting the VBUS_MAX to 16V instead of 32V, etc.

    // VBUS_MAX = 32V		(Assumes 32V, can also be set to 16V)
    // VSHUNT_MAX = 0.32	(Assumes Gain 8, 320mV, can also be 0.16, 0.08, 0.04)
    // RSHUNT = 0.1			(Resistor value in ohms)

    // 1. Determine max possible current
    // MaxPossible_I = VSHUNT_MAX / RSHUNT
    // MaxPossible_I = 3.2A

    // 2. Determine max expected current
    // MaxExpected_I = 1.0A

    // 3. Calculate possible range of LSBs (Min = 15-bit, Max = 12-bit)
    // MinimumLSB = MaxExpected_I/32767
    // MinimumLSB = 0.0000305             (30.5µA per bit)
    // MaximumLSB = MaxExpected_I/4096
    // MaximumLSB = 0.000244              (244µA per bit)

    // 4. Choose an LSB between the min and max values
    //    (Preferrably a roundish number close to MinLSB)
    // CurrentLSB = 0.0000400 (40µA per bit)

    // 5. Compute the calibration register
    // Cal = trunc (0.04096 / (Current_LSB * RSHUNT))
    // Cal = 10240 (0x2800)

    dev->calValue = 10240;

    // 6. Calculate the power LSB
    // PowerLSB = 20 * CurrentLSB
    // PowerLSB = 0.0008 (800µW per bit)

    // 7. Compute the maximum current and shunt voltage values before overflow
    //
    // Max_Current = Current_LSB * 32767
    // Max_Current = 1.31068A before overflow
    //
    // If Max_Current > Max_Possible_I then
    //    Max_Current_Before_Overflow = MaxPossible_I
    // Else
    //    Max_Current_Before_Overflow = Max_Current
    // End If
    //
    // ... In this case, we're good though since Max_Current is less than MaxPossible_I
    //
    // Max_ShuntVoltage = Max_Current_Before_Overflow * RSHUNT
    // Max_ShuntVoltage = 0.131068V
    //
    // If Max_ShuntVoltage >= VSHUNT_MAX
    //    Max_ShuntVoltage_Before_Overflow = VSHUNT_MAX
    // Else
    //    Max_ShuntVoltage_Before_Overflow = Max_ShuntVoltage
    // End If

    // 8. Compute the Maximum Power
    // MaximumPower = Max_Current_Before_Overflow * VBUS_MAX
    // MaximumPower = 1.31068 * 32V
    // MaximumPower = 41.94176W

    // Set multipliers to convert raw current/power values
    dev->currentDivider_mA = 25; // Current LSB = 40uA per bit (1000/40 = 25)
    dev->powerDivider_mW = 1;    // Power LSB = 800µW per bit

    // Set Calibration register to 'Cal' calculated above
    wireWriteRegister(dev->i2caddr, INA219_REG_CALIBRATION, dev->calValue);

    // Set Config register to take into account the settings above
    uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V | INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
                      INA219_CONFIG_SADCRES_12BIT_1S_532US | INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
    wireWriteRegister(dev->i2caddr, INA219_REG_CONFIG, config);
}

void ina219_setCalibration_16V_400mA(_ina219_device *dev)
{
    // Calibration which uses the highest precision for
    // current measurement (0.1mA), at the expense of
    // only supporting 16V at 400mA max.

    // VBUS_MAX = 16V
    // VSHUNT_MAX = 0.04          (Assumes Gain 1, 40mV)
    // RSHUNT = 0.1               (Resistor value in ohms)

    // 1. Determine max possible current
    // MaxPossible_I = VSHUNT_MAX / RSHUNT
    // MaxPossible_I = 0.4A

    // 2. Determine max expected current
    // MaxExpected_I = 0.4A

    // 3. Calculate possible range of LSBs (Min = 15-bit, Max = 12-bit)
    // MinimumLSB = MaxExpected_I/32767
    // MinimumLSB = 0.0000122              (12uA per bit)
    // MaximumLSB = MaxExpected_I/4096
    // MaximumLSB = 0.0000977              (98uA per bit)

    // 4. Choose an LSB between the min and max values
    //    (Preferrably a roundish number close to MinLSB)
    // CurrentLSB = 0.00005 (50uA per bit)

    // 5. Compute the calibration register
    // Cal = trunc (0.04096 / (Current_LSB * RSHUNT))
    // Cal = 8192 (0x2000)

    dev->calValue = 8192;

    // 6. Calculate the power LSB
    // PowerLSB = 20 * CurrentLSB
    // PowerLSB = 0.001 (1mW per bit)

    // 7. Compute the maximum current and shunt voltage values before overflow
    //
    // Max_Current = Current_LSB * 32767
    // Max_Current = 1.63835A before overflow
    //
    // If Max_Current > Max_Possible_I then
    //    Max_Current_Before_Overflow = MaxPossible_I
    // Else
    //    Max_Current_Before_Overflow = Max_Current
    // End If
    //
    // Max_Current_Before_Overflow = MaxPossible_I
    // Max_Current_Before_Overflow = 0.4
    //
    // Max_ShuntVoltage = Max_Current_Before_Overflow * RSHUNT
    // Max_ShuntVoltage = 0.04V
    //
    // If Max_ShuntVoltage >= VSHUNT_MAX
    //    Max_ShuntVoltage_Before_Overflow = VSHUNT_MAX
    // Else
    //    Max_ShuntVoltage_Before_Overflow = Max_ShuntVoltage
    // End If
    //
    // Max_ShuntVoltage_Before_Overflow = VSHUNT_MAX
    // Max_ShuntVoltage_Before_Overflow = 0.04V

    // 8. Compute the Maximum Power
    // MaximumPower = Max_Current_Before_Overflow * VBUS_MAX
    // MaximumPower = 0.4 * 16V
    // MaximumPower = 6.4W

    // Set multipliers to convert raw current/power values
    dev->currentDivider_mA = 20; // Current LSB = 50uA per bit (1000/50 = 20)
    dev->powerDivider_mW = 1;    // Power LSB = 1mW per bit

    // Set Calibration register to 'Cal' calculated above
    wireWriteRegister(dev->i2caddr, INA219_REG_CALIBRATION, dev->calValue);

    // Set Config register to take into account the settings above
    uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V | INA219_CONFIG_GAIN_1_40MV | INA219_CONFIG_BADCRES_12BIT |
                      INA219_CONFIG_SADCRES_12BIT_1S_532US | INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
    wireWriteRegister(dev->i2caddr, INA219_REG_CONFIG, config);
}

/**************************************************************************/
/*!
    @brief  Instantiates a new INA219 class
    @brief  Setups the HW (defaults to 32V and 2A for calibration values)
*/
/**************************************************************************/
void ina219_init(_ina219_device *dev, uint8_t addr)
{
	dev->i2caddr = addr;
	dev->currentDivider_mA = 0;
	dev->powerDivider_mW = 0;

    // Set chip to large range config values to start
    ina219_setCalibration_32V_2A(dev);
}

/**************************************************************************/
/*!
    @brief  Gets the raw bus voltage (16-bit signed integer, so +-32767)
*/
/**************************************************************************/
static int16_t ina219_getBusVoltage_raw(_ina219_device *dev)
{
    uint16_t value;
    wireReadRegister(dev->i2caddr, INA219_REG_BUSVOLTAGE, &value);

    // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
    return (int16_t)((value >> 3) * 4);
}

/**************************************************************************/
/*!
    @brief  Gets the raw shunt voltage (16-bit signed integer, so +-32767)
*/
/**************************************************************************/
static int16_t ina219_getShuntVoltage_raw(_ina219_device *dev)
{
    uint16_t value;
    wireReadRegister(dev->i2caddr, INA219_REG_SHUNTVOLTAGE, &value);
    return (int16_t)value;
}

/**************************************************************************/
/*!
    @brief  Gets the raw power (16-bit signed integer, so +-32767)
*/
/**************************************************************************/
static int16_t ina219_getPower_raw(_ina219_device *dev)
{
    uint16_t value;
    wireReadRegister(dev->i2caddr, INA219_REG_POWER, &value);
    return (int16_t)value;
}

/**************************************************************************/
/*!
    @brief  Gets the raw current value (16-bit signed integer, so +-32767)
*/
/**************************************************************************/
static int16_t ina219_getCurrent_raw(_ina219_device *dev)
{
    uint16_t value;

    // Sometimes a sharp load will reset the INA219, which will
    // reset the cal register, meaning CURRENT and POWER will
    // not be available ... avoid this by always setting a cal
    // value even if it's an unfortunate extra step
    wireWriteRegister(dev->i2caddr, INA219_REG_CALIBRATION, dev->calValue);

    // Now we can safely read the CURRENT register!
    wireReadRegister(dev->i2caddr, INA219_REG_CURRENT, &value);

    return (int16_t)value;
}

/**************************************************************************/
/*!
    @brief  Gets the shunt voltage in mV (so +-327mV)
*/
/**************************************************************************/
uint32_t ina219_getShuntVoltage_mV(_ina219_device *dev)
{
    int16_t value;
    value = ina219_getShuntVoltage_raw(dev);
    return value / 100;
}

/**************************************************************************/
/*!
    @brief  Gets the shunt voltage in volts
*/
/**************************************************************************/
uint32_t ina219_getBusVoltage_mV(_ina219_device *dev)
{
    int16_t value = ina219_getBusVoltage_raw(dev);
    return value;
}

/**************************************************************************/
/*!
    @brief  Gets the current value in mA, taking into account the
            config settings and current LSB
*/
/**************************************************************************/
uint32_t ina219_getCurrent_mA(_ina219_device *dev)
{
	uint32_t valueDec = ina219_getCurrent_raw(dev);
    valueDec /= dev->currentDivider_mA;
    return valueDec;
}

/**************************************************************************/
/*!
    @brief  Gets the power value in mW
*/
/**************************************************************************/
uint32_t ina219_getPower_mW(_ina219_device *dev)
{
	uint32_t valueDec = ina219_getPower_raw(dev);

	//LV_(("291 pwr %u"), valueDec);
	//LV_(("291 div %u"), dev->powerDivider_mW);

	valueDec /= dev->powerDivider_mW;
	return valueDec;
}

#endif
