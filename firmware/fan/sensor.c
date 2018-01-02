
#include "sensor.h"
#include "types.h"
#include "fan.h"
#include <avr/io.h>

#if SNS_ANALOG_SUPPORTED
// Conversion table for 10K NTC
const int8_t NTC_Conv_Table[] = {100, 85, 75, 67, 61, 56, 51, 47, 43, 40, 37, 34, 31, 28, 26, 23, 20, 18, 15};
#define NTC_CONV_TABLE_FIRST_ADC_VALUE (56)
#define NTC_CONV_TABLE_STEP (32)
#define NTC_CONV_TABLE_STEP_SHIFT (5)
#define NTC_CONV_TABLE_SIZE (sizeof(NTC_Conv_Table) / sizeof(NTC_Conv_Table[0]))
#endif

Sensor sns[MAX_SNS];
Sensor EEMEM sns_eeprom[MAX_SNS];

#if SNS_I2C_SUPPORTED
static uint8_t Sns_Temp_I2C_Present = 0; // bits 0..7 represent which i2c sensor SnsType_Temp_I2C_Addr0..SnsType_Temp_I2C_Addr7 is detected.
#endif

// Convert analog to digital, 10 bits accuracy.
// The result is right-adjusted, meaning the lower 10bits are used.
static uint16_t adConvert10bits(uint8_t channel)
{
    ADMUX = _BV(REFS0)               // Voltage reference: AVCC with external capacitor at AREF pin
            | channel;               // Analog channel: MUX3..MUX0
    ADCSRA |= _BV(ADIF) | _BV(ADSC); // clear hardware "conversion complete" flag & start conversion
    while (bit_is_set(ADCSRA, ADSC))
        ; // wait until conversion complete

    // read ADC (full 10 bits) (ADCL must be read first!)
    return ADCL | (ADCH << 8);
}

#if SNS_ANALOG_SUPPORTED
// Convert adc value to temperature, in degrees Celcius.
int8_t ntcAdcToTemp(uint16_t adc)
{
    uint8_t i;
    int8_t hi, lo;
    int16_t frac;

    // Clip to upper/lower table values
    if (adc < NTC_CONV_TABLE_FIRST_ADC_VALUE)
        return NTC_Conv_Table[0];
    if (adc >= NTC_CONV_TABLE_FIRST_ADC_VALUE + (NTC_CONV_TABLE_SIZE - 1) * NTC_CONV_TABLE_STEP)
        return NTC_Conv_Table[NTC_CONV_TABLE_SIZE - 1];

    adc -= NTC_CONV_TABLE_FIRST_ADC_VALUE;
    i = adc >> NTC_CONV_TABLE_STEP_SHIFT;   // calculate index in table
    frac = adc & (NTC_CONV_TABLE_STEP - 1); // calculate fraction
    lo = NTC_Conv_Table[i];
    hi = NTC_Conv_Table[i + 1];
    return lo + ((frac * (hi - lo)) >> NTC_CONV_TABLE_STEP_SHIFT);
}
#endif

// Update value for each sensor.
void updateSns()
{
    uint8_t i;
    for (i = 0; i < MAX_SNS; i++)
    {
        sns[i].status.valid = 1; // Temp value is valid by default.
        if (sns[i].type == SNSTYPE_NONE)
        {
            sns[i].status.valid = 0;
        }
#if SNS_ANALOG_SUPPORTED
        else if (sns[i].type >= SNSTYPE_TEMP_NTC0 && sns[i].type <= SNSTYPE_TEMP_NTC5)
        {
            // TODO: ADC channel selection
            uint8_t channel = sns[i].type - SNSTYPE_TEMP_NTC0 + 2;
            sns[i].value = ntcAdcToTemp(adConvert10bits(channel));
        }
#endif
#if SNS_I2C_SUPPORTED
        else if (sns[i].type >= SNSTYPE_TEMP_I2C_ADDR0 && sns[i].type <= SNSTYPE_TEMP_I2C_ADDR7)
        {
            uint8_t addr = sns[i].type - SNSTYPE_TEMP_I2C_ADDR0;
            // Only a sensor present at the bus can be read!
            if (Sns_Temp_I2C_Present & (1 << addr))
            {
                sns[i].value = readI2CMcp980x(addr);
            }
            else
            {
                sns[i].status.valid = 0;
            }
        }
#endif
        else if (sns[i].type >= SNSTYPE_RPS_FAN0 && sns[i].type <= SNSTYPE_RPS_FAN5)
        {
            uint8_t channel = sns[i].type - SNSTYPE_RPS_FAN0;
            sns[i].value = fans[channel].rps >> 8; // lower 8 bits represent the fraction, so take the full nr of revolutions.
        }
#if FAN_OUT_SUPPORTED
        else if (sns[i].type >= SNSTYPE_DUTY_IN0 && sns[i].type <= SNSTYPE_DUTY_IN1)
        {
            // TODO: ADC channel selection
            uint16_t advalue =
                adConvert10bits(i - SNSTYPE_DUTY_IN0); // ADC0/ADC1 receives the analog representation of the input duty cycle
            sns[i].value = (advalue * 25) >> 8;        // Convert value (1024 represents 100%, 0 represents 0%) to [0..100]% range.
                                                       // multiply by 100, then divide by 1024 (won't fit in 16 bits) so first
                                                       // multiply by 25, then divide by 256.
        }
#endif
        // Sensors SNSTYPE_EXT0..SNSTYPE_EXT3 can be written over USB.

        // If sensor value is invalid, set the value to 0.
        if (!sns[i].status.valid)
        {
            sns[i].value = 0;
        }
    }
}

void initSns()
{
// Configure AD conversion
#if SNS_ANALOG_SUPPORTED || FAN_OUT_SUPPORTED
    ADCSRA = _BV(ADEN)             // enable ADC
             | ADC_PRESCALER_BITS; // prescaler
#endif
}
