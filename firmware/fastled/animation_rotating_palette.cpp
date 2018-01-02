
#include "animation_rotating_palette.h"

namespace fastled
{

AnimationRotatingPalette::AnimationRotatingPalette(CRGB *leds, uint8_t led_count, animation *animation_info)
    : LedStripeAnimation(leds, led_count, animation_info)
{
}

AnimationRotatingPalette::~AnimationRotatingPalette()
{
}

void AnimationRotatingPalette::fillLEDsFromPaletteColors(uint8_t colorIndex)
{
    uint8_t brightness = 255;

    for (int i = 0; i < _led_count; i++)
    {
        _leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void AnimationRotatingPalette::initialize()
{
    /*
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
    */

    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
}

void AnimationRotatingPalette::loop()
{
    _startIndex = _startIndex + 1; /* motion speed */
    fillLEDsFromPaletteColors(_startIndex);
}

} /* namespace fastled */
