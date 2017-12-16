
#include "pwm.h"
#include "config.h"
#include "utils.h"
#include "fan.h"
#include "max31790.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef FAN_PWM_MAX31790_SUPPORTED

void initPwm()
{
    max31790device.addr = 0x20;
    max31790_initialize(&max31790device);
}

void initTacho()
{
}

void updatePwm()
{
    for (uint8_t i = 0; i < MAX_FANS; i++)
    {
        // Only 3- or 4-pin fans have a tacho signal
        if (fans[i].config.fanType >= MIN_FANTYPE_WITH_TACHO)
        {
        	max31790_read_fan_status(&max31790device, i);
        	fans[i].rpm = max31790_get_fan_rpm(&max31790device, i);

            // Convert nr of pinrises to nr of revolutions/sec.
            // The lower 8 bits of fans[i].rps represent the fraction.
            fans[i].rps = (fans[i].rpm / 60) << 8;

            // A fan will be flagged as stalled when:
            // * it is a 3- or 4-wire fan
            // * min_rps > 0
            // * current rps < min_rps
            fans[i].status.stalled = (fans[i].min_rps > 0) && ((fans[i].rps >> 8) < fans[i].min_rps);

            if (max31790_is_fan_fault(&max31790device, i))
            	fans[i].status.stalled = 1;
        }
        else
        {
            fans[i].rps = 0;
            fans[i].rpm = 0;
            fans[i].status.stalled = 0;
        }
    }
}

void updatePwmOut()
{
    for (uint8_t i = 0; i < MAX_FANS; i++)
    {
    	max31790_set_fan_pwm(&max31790device, fans[0].duty << 1, i);
    }
}

#else

// pin change interrupt for PCINT7..0
// pin change interrupt for PCINT23..16 is aliased use to this interrupt handler
// define as NONBLOCK to allow USB INT0 to interrupt this code.
ISR(PCINT0_vect, ISR_NOBLOCK)
{
    static uint8_t prev_PINB = 0;
    static uint8_t prev_PIND = 0;
    uint8_t changes;

    // Figure out which pin changed and had a low/high transition
    // and increment corresponding TACHO number of pinchanges
    changes = PIND ^ prev_PIND; // xor - bits changed since previous check will become 1
    if ((changes & _BV(PIN_TACHO_FAN0)) && (PIND & _BV(PIN_TACHO_FAN0)))
        fans[0].pinrise_count++; // PD0 TACHO0   PCINT16
    if ((changes & _BV(PIN_TACHO_FAN1)) && (PIND & _BV(PIN_TACHO_FAN1)))
        fans[1].pinrise_count++; // PD1 TACHO1   PCINT17
    if ((changes & _BV(PIN_TACHO_FAN2)) && (PIND & _BV(PIN_TACHO_FAN2)))
        fans[2].pinrise_count++; // PD7 TACHO2   PCINT23

    changes = PINB ^ prev_PINB; // xor - bits changed since previous check will become 1
    if ((changes & _BV(PIN_TACHO_FAN3)) && (PINB & _BV(PIN_TACHO_FAN3)))
        fans[3].pinrise_count++; // PB0 TACHO3   PCINT0
    if ((changes & _BV(PIN_TACHO_FAN4)) && (PINB & _BV(PIN_TACHO_FAN4)))
        fans[4].pinrise_count++; // PB4 TACHO4   PCINT4
    if ((changes & _BV(PIN_TACHO_FAN5)) && (PINB & _BV(PIN_TACHO_FAN5)))
        fans[5].pinrise_count++; // PB5 TACHO5   PCINT5

    prev_PINB = PINB;
    prev_PIND = PIND;
}

// pin change interrupt for PCINT23..16
ISR(PCINT2_vect, ISR_ALIASOF(PCINT0_vect));

void initPwm()
{
    // in updatePwmOut() the Pwm generation on the output pins is enabled.

    // -- Timer1 (16bit), OC1A/OC1B -- Fan3/Fan4 -------------------------------
    TCCR1A = _BV(WGM11);                      // mode 10 (phase correct PWM)
    TCCR1B = _BV(WGM13) | TMR1_PRESCALE_BITS; // prescale: clkIO/8 (From prescaler)

    ICR1 = F_CPU / 2 / TMR1_PRESCALE / FREQ_PWM; // top, don't write F_CPU/(2*TMR1_PRESCALE*FREQ_PWM) as this fails in macro expansion.

    TIMSK1 |= _BV(TOIE1); // enable timer1 Overflow Interrupt.

    // -- Timer0 (8bit), OC0A/OC0B -- Fan2/Fan1 --------------------------------
    TCCR0A = _BV(WGM01) | _BV(WGM00); // fast pwm mode 3 (top=0xff)
    TCCR0B = TMR0_PRESCALE_BITS;

    // -- Timer2 (8bit), OC2A/OC2B -- Fan5/Fan0---------------------------------
    TCCR2A = _BV(WGM01) | _BV(WGM00); // fast pwm mode 3 (top=0xff)
    TCCR2B = TMR2_PRESCALE_BITS;

    // configure all PWM ports as outputs
    MAKE_OUTPUT(DDR_PWM_FAN0, PIN_PWM_FAN0);
    MAKE_OUTPUT(DDR_PWM_FAN1, PIN_PWM_FAN1);
    MAKE_OUTPUT(DDR_PWM_FAN2, PIN_PWM_FAN2);
    MAKE_OUTPUT(DDR_PWM_FAN3, PIN_PWM_FAN3);
    MAKE_OUTPUT(DDR_PWM_FAN4, PIN_PWM_FAN4);
    MAKE_OUTPUT(DDR_PWM_FAN5, PIN_PWM_FAN5);
}

void initTacho()
{
    // set TACHO's to input with internal pullup & enable pin change interrupts
    MAKE_INPUT(DDR_TACHO_FAN0, PIN_TACHO_FAN0);
    MAKE_INPUT(DDR_TACHO_FAN1, PIN_TACHO_FAN1);
    MAKE_INPUT(DDR_TACHO_FAN2, PIN_TACHO_FAN2);
    MAKE_INPUT(DDR_TACHO_FAN3, PIN_TACHO_FAN3);
    MAKE_INPUT(DDR_TACHO_FAN4, PIN_TACHO_FAN4);
    MAKE_INPUT(DDR_TACHO_FAN5, PIN_TACHO_FAN5);
    ENABLE_PULLUP(PORT_TACHO_FAN0, PIN_TACHO_FAN0);
    ENABLE_PULLUP(PORT_TACHO_FAN1, PIN_TACHO_FAN1);
    ENABLE_PULLUP(PORT_TACHO_FAN2, PIN_TACHO_FAN2);
    ENABLE_PULLUP(PORT_TACHO_FAN3, PIN_TACHO_FAN3);
    ENABLE_PULLUP(PORT_TACHO_FAN4, PIN_TACHO_FAN4);
    ENABLE_PULLUP(PORT_TACHO_FAN5, PIN_TACHO_FAN5);

    PCMSK2 = _BV(PCINT16) | _BV(PCINT17) | _BV(PCINT23); // enable pin change interrupt for TACHO's 0,1,2
    PCMSK0 = _BV(PCINT0) | _BV(PCINT4) | _BV(PCINT5);    // enable pin change interrupt for TACHO's 3,4,5
    PCICR |= _BV(PCIE0) | _BV(PCIE2);                    // enable Pin change interrupts 0 & 2
}

void updatePwmOut()
{
    uint8_t i;

    // Set duty cycle for PWM outputs.
    OCR_PWM_FAN0 = (fans[0].duty * TOP_PWM_FAN0) >> 8; // PWM0
    OCR_PWM_FAN1 = (fans[1].duty * TOP_PWM_FAN1) >> 8; // PWM1
    OCR_PWM_FAN2 = (fans[2].duty * TOP_PWM_FAN2) >> 8; // PWM2
    OCR_PWM_FAN3 = (fans[3].duty * TOP_PWM_FAN3) >> 8; // PWM3
    OCR_PWM_FAN4 = (fans[4].duty * TOP_PWM_FAN4) >> 8; // PWM4
    OCR_PWM_FAN5 = (fans[5].duty * TOP_PWM_FAN5) >> 8; // PWM5

// * PWM 0, 1, 2, 5 will still produce a pulse-train when set to 0 or 100%
//   dutycycle if we don't explicitly set them to low/high and disable PMW generation.
// * PWM 3, 4 will still produce a pulse-train when set to 100%
//   dutycycle if we don't explicitly set them to high and disable PMW generation.
// Defining PWM_EXACT_0_AND_100_PERCENT_DUTY to 1 will fix this, at the cost of more code.

#if !PWM_EXACT_0_AND_100_PERCENT_DUTY
    for (i = 0; i < MAX_FANS; i++)
    {
        // Enable PWM generation on output pin
        switch (i)
        {
        case 0:
            ENABLE_PWM(TCCR_PWM_FAN0, COM_PWM_FAN0);
            break;
        case 1:
            ENABLE_PWM(TCCR_PWM_FAN1, COM_PWM_FAN1);
            break;
        case 2:
            ENABLE_PWM(TCCR_PWM_FAN2, COM_PWM_FAN2);
            break;
        case 3:
            ENABLE_PWM(TCCR_PWM_FAN3, COM_PWM_FAN3);
            break;
        case 4:
            ENABLE_PWM(TCCR_PWM_FAN4, COM_PWM_FAN4);
            break;
        case 5:
            ENABLE_PWM(TCCR_PWM_FAN5, COM_PWM_FAN5);
            break;
        }
    }
#else
    for (i = 0; i < MAX_FANS; i++)
    {
        if (fans[i].duty == PWM_DUTY_0 || fans[i].duty == PWM_DUTY_100)
        {
            // Disable PWM generation on output pin
            switch (i)
            {
            case 0:
                DISABLE_PWM(TCCR_PWM_FAN0, COM_PWM_FAN0);
                break;
            case 1:
                DISABLE_PWM(TCCR_PWM_FAN1, COM_PWM_FAN1);
                break;
            case 2:
                DISABLE_PWM(TCCR_PWM_FAN2, COM_PWM_FAN2);
                break;
            case 3:
                DISABLE_PWM(TCCR_PWM_FAN3, COM_PWM_FAN3);
                break;
            case 4:
                DISABLE_PWM(TCCR_PWM_FAN4, COM_PWM_FAN4);
                break;
            case 5:
                DISABLE_PWM(TCCR_PWM_FAN5, COM_PWM_FAN5);
                break;
            }
            if (fans[i].duty == PWM_DUTY_0)
            {
                // Set output pin to constant low level (0% duty cycle)
                switch (i)
                {
                case 0:
                    CLR_OUTPUT(PORT_PWM_FAN0, PIN_PWM_FAN0);
                    break;
                case 1:
                    CLR_OUTPUT(PORT_PWM_FAN1, PIN_PWM_FAN1);
                    break;
                case 2:
                    CLR_OUTPUT(PORT_PWM_FAN2, PIN_PWM_FAN2);
                    break;
                case 3:
                    CLR_OUTPUT(PORT_PWM_FAN3, PIN_PWM_FAN3);
                    break;
                case 4:
                    CLR_OUTPUT(PORT_PWM_FAN4, PIN_PWM_FAN4);
                    break;
                case 5:
                    CLR_OUTPUT(PORT_PWM_FAN5, PIN_PWM_FAN5);
                    break;
                }
            }
            else
            {
                // Set output pin to constant high level (100% duty cycle)
                switch (i)
                {
                case 0:
                    SET_OUTPUT(PORT_PWM_FAN0, PIN_PWM_FAN0);
                    break;
                case 1:
                    SET_OUTPUT(PORT_PWM_FAN1, PIN_PWM_FAN1);
                    break;
                case 2:
                    SET_OUTPUT(PORT_PWM_FAN2, PIN_PWM_FAN2);
                    break;
                case 3:
                    SET_OUTPUT(PORT_PWM_FAN3, PIN_PWM_FAN3);
                    break;
                case 4:
                    SET_OUTPUT(PORT_PWM_FAN4, PIN_PWM_FAN4);
                    break;
                case 5:
                    SET_OUTPUT(PORT_PWM_FAN5, PIN_PWM_FAN5);
                    break;
                }
            }
        }
        else
        {
            // Enable PWM generation on output pin
            switch (i)
            {
            case 0:
                ENABLE_PWM(TCCR_PWM_FAN0, COM_PWM_FAN0);
                break;
            case 1:
                ENABLE_PWM(TCCR_PWM_FAN1, COM_PWM_FAN1);
                break;
            case 2:
                ENABLE_PWM(TCCR_PWM_FAN2, COM_PWM_FAN2);
                break;
            case 3:
                ENABLE_PWM(TCCR_PWM_FAN3, COM_PWM_FAN3);
                break;
            case 4:
                ENABLE_PWM(TCCR_PWM_FAN4, COM_PWM_FAN4);
                break;
            case 5:
                ENABLE_PWM(TCCR_PWM_FAN5, COM_PWM_FAN5);
                break;
            }
        }
    }
#endif
}

void updatePwm()
{
    for (i = 0; i < MAX_FANS; i++)
    {
        // Only 3- or 4-pin fans have a tacho signal
        if (fans[i].config.fanType >= MIN_FANTYPE_WITH_TACHO)
        {
            // Convert nr of pinrises to nr of revolutions/sec.
            // The lower 8 bits of fans[i].rps represent the fraction.
            fans[i].rps = (uint16_t)(fans[i].pinrise_count) * (uint16_t)(fans[i].inc_per_pinrise);
            // A fan will be flagged as stalled when:
            // * it is a 3- or 4-wire fan
            // * min_rps > 0
            // * current rps < min_rps
            fans[i].status.stalled = (fans[i].min_rps > 0) && ((fans[i].rps >> 8) < fans[i].min_rps);
        }
        else
        {
            fans[i].rps = 0;
            fans[i].status.stalled = 0;
        }
        fans[i].pinrise_count = 0;
    }
}

#endif
