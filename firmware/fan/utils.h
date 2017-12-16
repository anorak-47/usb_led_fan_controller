#ifndef __UTILS_H_INCLUDED__
#define __UTILS_H_INCLUDED__

// Some macro's to make life easier....
#define ENABLE_PWM(tccr,com)        (tccr |= _BV(com))
#define DISABLE_PWM(tccr,com)       (tccr &= ~_BV(com))
#define MAKE_OUTPUT(ddr,pin)        (ddr |= _BV(pin))
#define MAKE_INPUT(ddr,pin)         (ddr &= ~_BV(pin))
#define SET_OUTPUT(port,pin)        (port |= _BV(pin))
#define CLR_OUTPUT(port,pin)        (port &= ~_BV(pin))
#define ENABLE_PULLUP(port,pin)     (port |= _BV(pin))
#define IS_OUTPUT(ddr,pin)          ((ddr &= _BV(pin)) != 0)

#endif
