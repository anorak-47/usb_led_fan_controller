
#include "fan_out.h"
#include "fan.h"
#include "timer.h"
#include "utils.h"

#if FAN_OUT_SUPPORTED
Fan_out fan_out[MAX_FAN_OUTS];
Fan_out EEMEM fan_out_eeprom[MAX_FAN_OUTS];

static uint16_t rps_out_counter[MAX_FAN_OUTS];
static uint16_t rps_out_tmr1_ovf_interval[MAX_FAN_OUTS];

void initFanOut()
{
    // Configure generated TACHO output
    // According to Intel 4-Wire Pulse Width Modulation Controlled Fans, rev 1.2:
    // Fan shall provide tachometer output signal with the following characteristics:
    // * Two pulses per revolution
    // * Open-collector or open-drain type output
    // * Motherboard will have a pull up to 12V, maximum 13.2V

    MAKE_INPUT(DDR_TACHO_OUT_0, PIN_TACHO_OUT_0);  // Tristate pin
    CLR_OUTPUT(PORT_TACHO_OUT_0, PIN_TACHO_OUT_0); // ..

#if MAX_FAN_OUTS >= 1
    MAKE_INPUT(DDR_TACHO_OUT_1, PIN_TACHO_OUT_1);  // Tristate pin
    CLR_OUTPUT(PORT_TACHO_OUT_1, PIN_TACHO_OUT_1); // ..
#endif
}

void setFanOut()
{
    if (timer1_ovf_counter >= rps_out_counter[0])
    {
        rps_out_counter[0] += rps_out_tmr1_ovf_interval[0];
        if (IS_OUTPUT(DDR_TACHO_OUT_0, PIN_TACHO_OUT_0))
            MAKE_INPUT(DDR_TACHO_OUT_0, PIN_TACHO_OUT_0); // Tristate pin; motherboard will pull up to Vcc.
        else
            MAKE_OUTPUT(DDR_TACHO_OUT_0, PIN_TACHO_OUT_0); // Output low
    }

#if MAX_FAN_OUTS >= 1
    if (timer1_ovf_counter >= rps_out_counter[1])
    {
        rps_out_counter[1] += rps_out_tmr1_ovf_interval[1];
        if (IS_OUTPUT(DDR_TACHO_OUT_1, PIN_TACHO_OUT_1))
            MAKE_INPUT(DDR_TACHO_OUT_1, PIN_TACHO_OUT_1); // Tristate pin; motherboard will pull up to Vcc.
        else
            MAKE_OUTPUT(DDR_TACHO_OUT_1, PIN_TACHO_OUT_1); // Output low
    }
#endif
}

void resetFanOutCounter()
{
    for (uint8_t fo = 0; fo < MAX_FAN_OUTS; fo++)
        rps_out_counter[fo] = rps_out_tmr1_ovf_interval[fo];
}

void updateFanOut()
{
    uint8_t i;
    uint8_t rps = 0; // rps for fan which is used as input sensor (e.g. going to the motherboard to act on fan failures)
    uint16_t rpm = 0;
    uint8_t first = 1;

    for (uint8_t fo = 0; fo < MAX_FAN_OUTS; fo++)
    {
        // A fan will be flagged as stalled when:
        // * it has tacho feedback
        // * min_rps > 0
        // * current rps < min_rps

        if ((fan_out[fo].mode >= FANOUTMODE_RPS_CLONEFAN0) && (fan_out[fo].mode <= FANOUTMODE_RPS_CLONEFAN5))
        {
            rps = fans[fan_out[fo].mode - FANOUTMODE_RPS_CLONEFAN0].rps >> 8;
            rpm = fans[fan_out[fo].mode - FANOUTMODE_RPS_CLONEFAN0].rpm;
        }
        else if (fan_out[fo].mode == FANOUTMODE_RPS_FIXED_50)
        {
            rps = 50;
            rpm = 50 * 60;
        }

        // Figure out if any fan has stalled, and determine min/max
        // rps when requested.
        for (i = 0; i < MAX_FANS; i++)
        {
            if (fans[i].config.fanType >= MIN_FANTYPE_WITH_TACHO) // fan has speed feedback
            {
                uint8_t rps_fan = fans[i].rps >> 8;
                uint16_t rpm_fan = fans[i].rpm;

                if ((fan_out[fo].fanStallDetect & (1 << i)) // fan i included in stall detection?
                    && fans[i].status.stalled)              // fan i is stalled?
                {
                    rps = 0; // Fan stalled: output rps of 0 and break out of loop
                    rpm = 0;
                    break;
                }
                else if (fan_out[fo].mode == FANOUTMODE_RPS_MINFAN)
                {
                    if (first || rps > rps_fan)
                    {
                        rps = rps_fan;
                        rpm = rpm_fan;
                        first = 0;
                    }
                }
                else if (fan_out[fo].mode == FANOUTMODE_RPS_MAXFAN)
                {
                    if (first || rps < rps_fan)
                    {
                        rps = rps_fan;
                        rpm = rpm_fan;
                        first = 0;
                    }
                }
            }
        }
        fan_out[fo].rps = rps;
        fan_out[fo].rpm = rpm;

        // To simulate the fan rotation to e.g. the motherboard, the fan tacho signal
        // has to generate 2 pulses per revolution, and rps revolutions a second.
        // The interval, using timer1 overflow counter, between each toggle
        // is FREQ_PWM/(4*rps)
        rps_out_tmr1_ovf_interval[fo] = (FREQ_PWM >> 2) / rps;
    }
}

uint16_t ovf_interval(uint8_t channel)
{
    if (channel > MAX_FAN_OUTS)
        return 0;
    return rps_out_tmr1_ovf_interval[channel];
}

#endif
