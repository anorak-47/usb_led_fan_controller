
#include "ctrl.h"
#include "sensor.h"
#include "fan.h"

#if FAN_MODE_PI_SUPPORTED
// Restart control loop
void restartCtrl(const uint8_t channel)
{
    fans[channel].i = 0;
    fans[channel].es = 0;
}
#endif

// Calculate the new dutycycle for given channel.
void updateCtrl(const uint8_t i /* channel */)
{
    uint8_t snsIdx = fans[i].config.snsIdx;

    // Safety: If fan is under automatic control (mode != fixed),
    // then an invalid sensor readout will lead to 100% dutycycle
    // for the fan.
    if ((fans[i].config.fanMode != FANMODE_FIXED_DUTY) && (!sns[snsIdx].status.valid))
    {
        fans[i].duty = fans[i].dutyMax;
        return;
    }

    // Determine new dutycycle for fan, depending on the fan's control mode.
    switch (fans[i].config.fanMode)
    {
    case FANMODE_FIXED_DUTY:
        fans[i].duty = fans[i].dutyFixed;
        break;
#if FAN_MODE_TRIP_POINTS_SUPPORTED
    case FANMODE_TRIP_POINTS:
    {
        int16_t v = 0;

        if (sns[snsIdx].value <= fans[i].trip_point[0].value)
        {
            v = fans[i].trip_point[0].duty;
        }
        else if (sns[snsIdx].value > fans[i].trip_point[MAX_TRIP_POINTS - 1].value)
        {
            v = fans[i].trip_point[MAX_TRIP_POINTS - 1].duty;
        }
        else
        {
            for (uint8_t t = 0; t < MAX_TRIP_POINTS - 1; t++)
            {
                if (sns[snsIdx].value > fans[i].trip_point[t].value && sns[snsIdx].value <= fans[i].trip_point[t + 1].value)
                {
                    v = fans[i].trip_point[t + 1].duty;
                    break;
                }
            }
        }
        // Limit output to allowed range
        if (v < (int16_t)fans[i].dutyMin)
            v = (int16_t)fans[i].dutyMin; // clip to lower duty cycle
        else if (v > (int16_t)fans[i].dutyMax)
            v = (int16_t)fans[i].dutyMax; // clip to upper duty cycle
        fans[i].duty = v;
    }
    break;
#endif
#if FAN_MODE_LIN_TRIP_POINTS_SUPPORTED
    case FANMODE_LINEAR_TRIP_POINTS:
    {
        int16_t v = 0;

        if (sns[snsIdx].value <= fans[i].trip_point[0].value)
        {
            v = fans[i].trip_point[0].duty;
        }
        else if (sns[snsIdx].value > fans[i].trip_point[MAX_TRIP_POINTS - 1].value)
        {
            v = fans[i].trip_point[MAX_TRIP_POINTS - 1].duty;
        }
        else
        {
            for (uint8_t t = 0; t < MAX_TRIP_POINTS - 1; t++)
            {
                if (sns[snsIdx].value > fans[i].trip_point[t].value && sns[snsIdx].value <= fans[i].trip_point[t + 1].value)
                {
                    uint8_t dy = (fans[i].trip_point[t + 1].duty - fans[i].trip_point[t].duty);
                    int16_t dx = (fans[i].trip_point[t + 1].value - fans[i].trip_point[t].value);
                    v = ((sns[snsIdx].value - fans[i].trip_point[t].value) * dy / dx) + fans[i].trip_point[t].duty;
                    break;
                }
            }
        }
        // Limit output to allowed range
        if (v < (int16_t)fans[i].dutyMin)
            v = (int16_t)fans[i].dutyMin; // clip to lower duty cycle
        else if (v > (int16_t)fans[i].dutyMax)
            v = (int16_t)fans[i].dutyMax; // clip to upper duty cycle
        fans[i].duty = v;
    }
    break;
#endif
#if FAN_MODE_LINEAR_SUPPORTED
    case FANMODE_LINEAR:
    {
        int16_t v = ((fans[i].dutyGain * sns[snsIdx].value) >> 6) +
                    fans[i].dutyOffs; // gain*value shift right 6 bits, as lower 6 bits in gain contain the fraction
        // Limit output to allowed range
        if (v < (int16_t)fans[i].dutyMin)
            v = (int16_t)fans[i].dutyMin; // clip to lower duty cycle
        else if (v > (int16_t)fans[i].dutyMax)
            v = (int16_t)fans[i].dutyMax; // clip to upper duty cycle
        fans[i].duty = v;
    }
    break;
#endif
#if FAN_MODE_PI_SUPPORTED
    case FANMODE_PI:
    {
        // Calculate new duty cycle.
        int8_t setp;
        int16_t v, u;
        // Calculate the setpoint.
        setp = sns[fans[i].snsSetp.snsIdx].value + fans[i].snsSetp.delta;
        // Calculate error e
        int8_t e = sns[snsIdx].value - setp;
#if CTRL_DEBUG
        fans_dbg[i].e = e;
#endif
        // Calculate P & I, store in v
        v = fans[i].Kp * e; // P
#if CTRL_DEBUG
        fans_dbg[i].p = v;
#endif
        fans[i].i += fans[i].Ki * e + fans[i].Kt * fans[i].es; // I, kt*es for anti-windup
        v += fans[i].i;                                        // v = P + I

        // Limit output to allowed range
        if (v < (int16_t)fans[i].dutyMin)
            u = (int16_t)fans[i].dutyMin; // clip to lower duty cycle
        else if (v > (int16_t)fans[i].dutyMax)
            u = (int16_t)fans[i].dutyMax; // clip to upper duty cycle
        else
            u = v;

        // when error becomes 0 and integrator is still != 0, slowly reduce integrator to 0
        // Otherwise calculate saturation amount of output wrt input.
        if (e == 0 && fans[i].i != 0)
            if (fans[i].i > 0)
                fans[i].es = -1;
            else
                fans[i].es = +1;
        else
            fans[i].es = u - v;

        // Store the calculated duty cycle. The next update (every second)
        // the new duty cycle will be set.
        fans[i].duty = u;
    }
    break;
#endif
#if FAN_MODE_FUZZY_SUPPORTED
    case FANMODE_FUZZY:
    {
        // TODO
    }
    break;
#endif
    }
}
