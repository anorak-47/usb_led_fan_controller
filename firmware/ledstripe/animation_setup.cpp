#include "animation.h"
#include "animation_setup.h"
#include "animation_type.h"
#include "config.h"
#include "Arduino.h"
#include "debug.h"

/*
// ATMEL ATMEGA32U4 / ARDUINO LEONARDO
//
// D0				PD2					RXD1/INT2
// D1				PD3					TXD1/INT3
// D2				PD1		SDA			SDA/INT1
// D3#				PD0		PWM8/SCL	OC0B/SCL/INT0
// D4		A6		PD4					ADC8
// D5#				PC6		???			OC3A/#OC4A
// D6#		A7		PD7		FastPWM		#OC4D/ADC10
// D7				PE6					INT6/AIN0
//
// D8		A8		PB4					ADC11/PCINT4
// D9#		A9		PB5		PWM16		OC1A/#OC4B/ADC12/PCINT5
// D10#		A10		PB6		PWM16		OC1B/0c4B/ADC13/PCINT6
// D11#				PB7		PWM8/16		0C0A/OC1C/#RTS/PCINT7
// D12		A11		PD6					T1/#OC4D/ADC9
// D13#				PC7		PWM10		CLK0/OC4A
//
// A0		D18		PF7					ADC7
// A1		D19		PF6					ADC6
// A2		D20 	PF5					ADC5
// A3		D21 	PF4					ADC4
// A4		D22		PF1					ADC1
// A5		D23 	PF0					ADC0
//
*/

// Data pin that led data will be written out over
#define DATA_PIN_0 8  // D8  --> ATMEGA32U4: PB4 --> Pololu links Pin 8
#define DATA_PIN_1 9  // D9  --> ATMEGA32U4: PB5 --> Pololu links Pin 9
#define DATA_PIN_2 10 // D10 --> ATMEGA32U4: PB6 --> Pololu links Pin 10
#define DATA_PIN_3 11 // D11 --> ATMEGA32U4: PB7 --> Pololu links Pin 11

// Default brightness
#define BRIGHTNESS 64

/*
// How many leds are in the strip?
#define NUM_LEDS_1 64
#define NUM_LEDS_PER_STRIP_1 (NUM_LEDS_1 / 2)

#define NUM_LEDS_2 64
#define NUM_LEDS_PER_STRIP_2 (NUM_LEDS_2 / 2)

// This is an array of leds.  One item for each led in your strip.
CRGB leds_1[NUM_LEDS_1];
CRGB leds_2[NUM_LEDS_2];
*/

#define NUM_LEDS_STRIPE_0 10
CRGB leds_stripe_0[NUM_LEDS_STRIPE_0];

#define NUM_LEDS_STRIPE_1 1
CRGB leds_stripe_1[NUM_LEDS_STRIPE_1];

#define NUM_LEDS_STRIPE_2 128
CRGB leds_stripe_2[NUM_LEDS_STRIPE_2];

void led_stripe_setup()
{
    // sanity check delay - allows reprogramming if accidently blowing power w/leds
    // delay(2000);

    // FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
    FastLED.addLeds<WS2812, DATA_PIN_0, RGB>(leds_stripe_0, NUM_LEDS_STRIPE_0).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<WS2812, DATA_PIN_1, RGB>(leds_stripe_1, NUM_LEDS_STRIPE_1).setCorrection(TypicalLEDStrip);
    // FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
    // FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

    // FastLED.addLeds<APA102, RGB>(leds, NUM_LEDS);
    // FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);

    //FastLED.addLeds<APA102, RGB>(leds_stripe_2, NUM_LEDS_STRIPE_2);

    /*
    // https://github.com/FastLED/FastLED/wiki/Multiple-Controller-Examples

    FastLED.addLeds<WS2812, DATA_PIN_0, RGB>(leds_1, 0, NUM_LEDS_PER_STRIP_1).setCorrection( TypicalLEDStrip );
    FastLED.addLeds<WS2812, DATA_PIN_1, RGB>(leds_1, NUM_LEDS_PER_STRIP_1, NUM_LEDS_PER_STRIP_1).setCorrection( TypicalLEDStrip );

    FastLED.addLeds<WS2812, DATA_PIN_2, RGB>(leds_2, 0, NUM_LEDS_PER_STRIP_2).setCorrection( TypicalLEDStrip );
    FastLED.addLeds<WS2812, DATA_PIN_3, RGB>(leds_2, NUM_LEDS_PER_STRIP_2, NUM_LEDS_PER_STRIP_2).setCorrection( TypicalLEDStrip );
    */

    FastLED.setBrightness(BRIGHTNESS);


    led_stripe[0].leds = leds_stripe_0;
    led_stripe[0].led_count = NUM_LEDS_STRIPE_0;

    led_stripe[1].leds = leds_stripe_1;
    led_stripe[1].led_count = NUM_LEDS_STRIPE_1;

    led_stripe[2].leds = leds_stripe_2;
    led_stripe[2].led_count = NUM_LEDS_STRIPE_2;
}
