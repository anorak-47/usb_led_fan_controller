
#include "timer.h"
#include "config.h"
#include <avr/io.h>

// Count number of timer1 overflow interrupts to allow accurate timing without
// using a timer. Increments by FREQ_PWM each second.
uint16_t timer1_ovf_counter = 0;

// Timer1 overflow interrupt.
// Used to keep the 8bit PWM's in sync with this timer.
ISR(TIMER1_OVF_vect, ISR_NOBLOCK)
{
    //TCNT0 = 0xff;
    //TCNT2 = 0xff;
    timer1_ovf_counter++;
}

void initTimer()
{
    // -- Timer1 (16bit), OC1A/OC1B -- Fan3/Fan4 -------------------------------
    TCCR1A = _BV(WGM11);                      // mode 10 (phase correct PWM)
    TCCR1B = _BV(WGM13) | TMR1_PRESCALE_BITS; // prescale: clkIO/8 (From prescaler)

    ICR1 = F_CPU / 2 / TMR1_PRESCALE / FREQ_PWM; // top, don't write F_CPU/(2*TMR1_PRESCALE*FREQ_PWM) as this fails in macro expansion.

    TIMSK1 |= _BV(TOIE1); // enable timer1 Overflow Interrupt.
}
