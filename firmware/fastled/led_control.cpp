// Use if you want to force the software SPI subsystem to be used for some reason (generally, you don't)
// #define FASTLED_FORCE_SOFTWARE_SPI
// Use if you want to force non-accelerated pin access (hint: you really don't, it breaks lots of things)
// #define FASTLED_FORCE_SOFTWARE_SPI
// #define FASTLED_FORCE_SOFTWARE_PINS

extern "C" {
#include "led_control.h"
#include "config.h"
}

#if FASTLED_SUPPORTED
#define FASTLED_INTERNAL
#include <FastLED.h>

extern "C" {

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
#define DATA_PIN_0  8 // D8  --> ATMEGA32U4: PB4 --> Pololu links Pin 8
#define DATA_PIN_1  9 // D9  --> ATMEGA32U4: PB5 --> Pololu links Pin 9
#define DATA_PIN_2 10 // D10 --> ATMEGA32U4: PB6 --> Pololu links Pin 10
#define DATA_PIN_3 11 // D11 --> ATMEGA32U4: PB7 --> Pololu links Pin 11

// Default brightness
#define BRIGHTNESS  64

// How many leds are in the strip?
#define NUM_LEDS_1 64
#define NUM_LEDS_PER_STRIP_1 (NUM_LEDS_1/2)

#define NUM_LEDS_2 64
#define NUM_LEDS_PER_STRIP_2 (NUM_LEDS_2/2)

// This is an array of leds.  One item for each led in your strip.
CRGB leds_1[NUM_LEDS_1];
CRGB leds_2[NUM_LEDS_2];

#define NUM_LEDS 10
CRGB leds[NUM_LEDS];

void initialize_honode(void);

// This function sets up the ledsand tells the controller about them
void led_control_setup()
{
	initialize_honode();

    // sanity check delay - allows reprogramming if accidently blowing power w/leds
    //delay(2000);

    //FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
    FastLED.addLeds<WS2812, DATA_PIN_0, RGB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    // FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
    // FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

    /*
    // https://github.com/FastLED/FastLED/wiki/Multiple-Controller-Examples

    FastLED.addLeds<WS2812, DATA_PIN_0, RGB>(leds_1, 0, NUM_LEDS_PER_STRIP_1).setCorrection( TypicalLEDStrip );
    FastLED.addLeds<WS2812, DATA_PIN_1, RGB>(leds_1, NUM_LEDS_PER_STRIP_1, NUM_LEDS_PER_STRIP_1).setCorrection( TypicalLEDStrip );

    FastLED.addLeds<WS2812, DATA_PIN_2, RGB>(leds_2, 0, NUM_LEDS_PER_STRIP_2).setCorrection( TypicalLEDStrip );
    FastLED.addLeds<WS2812, DATA_PIN_3, RGB>(leds_2, NUM_LEDS_PER_STRIP_2, NUM_LEDS_PER_STRIP_2).setCorrection( TypicalLEDStrip );
	*/

    FastLED.setBrightness(  BRIGHTNESS );
}

// This function runs over and over, and is where you do the magic to light
// your leds.
void led_control_task__example1()
{
    // Move a single white led
    for (int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1)
    {
        // Turn our current led on to white, then show the leds
        leds[whiteLed] = CRGB::White;

        // Show the leds (only one of which is set to white, from above)
        FastLED.show();

        // Wait a little bit
        delay(100);

        // Turn our current led back to black for the next loop around
        leds[whiteLed] = CRGB::Black;
    }
}

#define UPDATES_PER_SECOND 50

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;

    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;

}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;

    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;

    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }
}

static unsigned long millies_old = 0;

void led_control_task()
{
	unsigned long time_delta = millis() - millies_old;
	if (time_delta > (1000 / UPDATES_PER_SECOND))
	{
		millies_old = millis();
		ChangePalettePeriodically();

		static uint8_t startIndex = 0;
		startIndex = startIndex + 1; /* motion speed */

		FillLEDsFromPaletteColors( startIndex);

		FastLED.show();
	}
}

} // extern "C"
#endif
