#ifndef __LED_H_INCLUDED__
#define __LED_H_INCLUDED__

#include <inttypes.h>

#define LED_FAN_0 0x01
#define LED_FAN_1 0x02
#define LED_FAN_2 0x04
#define LED_FAN_3 0x08
#define LED_FAN_4 0x10
#define LED_FAN_5 0x20
#define LED_ALARM 0x40
#define LED_INFO  0x80

void set_led(uint8_t led);
void unset_led(uint8_t led);

void set_leds_by_mask(uint8_t mask);
uint8_t get_led_mask(void);

#endif
