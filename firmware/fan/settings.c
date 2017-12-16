
#include "settings.h"
#include "config.h"
#include "fan.h"
#include "sensor.h"
#include "fan_out.h"
#include "ctrl.h"
#include <avr/eeprom.h>

#define EEPROM_CHK_MAGIC (0xA5) // magic number to 'assure' empty/garbage eeprom sections can be detected
uint8_t EEMEM chk_eeprom;       // assure this variable follows the eeprom section which must be checksum-checked!

// Validate or calculate eeprom checksum.
// When write is true, the eeprom checksum will be calculated and stored for the current contents
// and the returned value is undefined.
// When write is false, the eeprom checksum will be validated; false will be returned when the checkum
// is invalid, true otherwise.
static inline uint8_t eepromChecksum(const uint8_t write)
{
    uint8_t *adr = (uint8_t *)&fans_eeprom[0];
    uint8_t chk = 0;
    // Calculate xor of all bytes in checksum-checked eeprom area.
    do
    {
        chk ^= eeprom_read_byte(adr++);
    } while (adr != &chk_eeprom);
    // Write the checksum to the eeprom area, or calculate it.
    if (write)
    {
        chk ^= EEPROM_CHK_MAGIC;
        eeprom_write_byte(&chk_eeprom, chk);
    }
    else
    {
        chk ^= eeprom_read_byte(&chk_eeprom);
    }
    return chk == EEPROM_CHK_MAGIC;
}

// Try to load settings from eeprom.
// When eeprom data is invalid or parameter clear is set, new default values will be generated.
void loadSettings(const uint8_t clear)
{
    // Load eeprom contents when checksum is valid, or defaults otherwise.
    uint8_t i;
    uint8_t eeprom_valid = !clear && eepromChecksum(0);
    for (i = 0; i < MAX_FANS; i++)
    {
        // Clear the whole structure. Any records which must be set to 0
        // will not explicitly have to be cleared, to save some bytes codespace.
        memset((void *)&(fans[i]), 0, sizeof(fans[i]));
        if (eeprom_valid)
        {
            // Eeprom valid; load fan defaults from eeprom.
            eeprom_read_block((void *)&(fans[i]), &(fans_eeprom[i]), sizeof(fans[i]));
        }
        else
        {
            fans[i].config.fanType = FANTYPE_NONE;
            fans[i].config.fanMode = FANMODE_FIXED_DUTY;
            fans[i].dutyFixed = PWM_DUTY_100;
            fans[i].inc_per_pinrise = 256 / 2;
#if FAN_MODE_PI_SUPPORTED || FAN_MODE_FUZZY_SUPPORTED
            fans[i].snsSetp.delta = 20; // get a sane starting value
            fans[i].snsSetp.snsIdx = 0; // defaults to first reference sensor (which in turn defaults to no sensor). Set to 0 by memset
#endif
#if FAN_MODE_TRIP_POINTS_SUPPORTED || FAN_MODE_LIN_TRIP_POINTS_SUPPORTED
            for (uint8_t t = 0; t < MAX_TRIP_POINTS; ++t)
            {
                fans[i].trip_point[t].duty = 255;
                fans[i].trip_point[t].value = 0;
            }
#endif
#if FAN_MODE_PI_SUPPORTED
            fans[i].Kp = 2;
            fans[i].Ki = 1;
            fans[i].Kt = 1;
#endif
#if (PWM_DUTY_0 != 0) // set to 0 by memset
            fans[i].dutyMin = PWM_DUTY_0;
#endif
            fans[i].dutyMax = PWM_DUTY_100;
            fans[i].min_rps = 5; // 5 rps -> 300 rpm
        }
        //    fans[i].pinrise_count = 0;            // set to 0 by memset
        //    fans[i].rps = 0;                      // set to 0 by memset
        restartCtrl(i);
    }

#if FAN_OUT_SUPPORTED
    for (i = 0; i < MAX_FAN_OUTS; i++)
    {
        memset((void *)&(fan_out[i]), 0, sizeof(fan_out[i]));
        if (eeprom_valid)
        {
            // Eeprom valid; load fan_out defaults from eeprom.
            eeprom_read_block((void *)&(fan_out[i]), &(fan_out_eeprom[i]), sizeof(fan_out[i]));
        }
        else
        {
#if (FANOUTMODE_RPS_MINFAN != 0) // set to 0 by memset
            fan_out[i].mode = FANOUTMODE_RPS_MINFAN;
#endif
            fan_out[i].fanStallDetect = (1 << MAX_FANS) - 1; // include all fans in stall detection.
        }
    }
#endif

    for (i = 0; i < MAX_SNS; i++)
    {
        memset((void *)&(sns[i]), 0, sizeof(sns[i]));
        if (eeprom_valid)
        {
            // Eeprom valid; load sensor defaults from eeprom.
            eeprom_read_block((void *)&(sns[i]), &(sns_eeprom[i]), sizeof(sns[i]));
        }
        else
        {
#if (SNSTYPE_NONE != 0) // set to 0 by memset
            sns[i].type = SNSTYPE_NONE;
#endif
        }
        sns[i].status.valid = 0;
    }
}

void saveSettings()
{
    uint8_t i;
    // Write complete fan/sensor structs to eeprom, to be restored automatically
    // on AVR start.
    // Each struct contains too much data to be stored in eeprom, actually, but
    // it takes less code to simply store the complete struct.
    // Eeprom space is sufficient...
    for (i = 0; i < MAX_FANS; i++)
    {
        eeprom_write_block((void *)&(fans[i]), &(fans_eeprom[i]), sizeof(fans[i]));
    }
#if FAN_OUT_SUPPORTED
    for (i = 0; i < MAX_FAN_OUTS; i++)
	{
    	eeprom_write_block((void *)&(fan_out[i]), &(fan_out_eeprom[i]), sizeof(fan_out[i]));
	}
#endif
    for (i = 0; i < MAX_SNS; i++)
    {
        eeprom_write_block((void *)&(sns[i]), &(sns_eeprom[i]), sizeof(sns[i]));
    }
    // Calculate & store new eeprom checksum.
    eepromChecksum(1);
}
