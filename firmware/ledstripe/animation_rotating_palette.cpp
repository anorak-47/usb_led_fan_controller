
#include <ledstripe/animation_rotating_palette.h>

// Additionl notes on FastLED compact palettes:
//
// Normally, in computer graphics, the palette (or "color lookup table")
// has 256 entries, each containing a specific 24-bit RGB color.  You can then
// index into the color palette using a simple 8-bit (one byte) value.
// A 256-entry color palette takes up 768 bytes of RAM, which on Arduino
// is quite possibly "too many" bytes.
//
// FastLED does offer traditional 256-element palettes, for setups that
// can afford the 768-byte cost in RAM.
//
// However, FastLED also offers a compact alternative.  FastLED offers
// palettes that store 16 distinct entries, but can be accessed AS IF
// they actually have 256 entries; this is accomplished by interpolating
// between the 16 explicit entries to create fifteen intermediate palette
// entries between each pair.
//
// So for example, if you set the first two explicit entries of a compact
// palette to Green (0,255,0) and Blue (0,0,255), and then retrieved
// the first sixteen entries from the virtual palette (of 256), you'd get
// Green, followed by a smooth gradient from green-to-blue, and then Blue.


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

void AnimationRotatingPalette::SetupPurpleAndGreenPalette()
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

void AnimationRotatingPalette::SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

void AnimationRotatingPalette::setOption(uint8_t option)
{
    if (option == 0)
    {
        currentPalette = RainbowColors_p;
        currentBlending = LINEARBLEND;
    }
    if (option == 1)
    {
        currentPalette = RainbowStripeColors_p;
        currentBlending = NOBLEND;
    }
    if (option == 2)
    {
        currentPalette = RainbowStripeColors_p;
        currentBlending = LINEARBLEND;
    }
    if (option == 3)
    {
        currentPalette = CloudColors_p;
        currentBlending = LINEARBLEND;
    }
    if (option == 4)
    {
        currentPalette = PartyColors_p;
        currentBlending = LINEARBLEND;
    }
    if (option == 5)
    {
        SetupPurpleAndGreenPalette();
        currentBlending = LINEARBLEND;
    }
    if (option == 6)
    {
        SetupTotallyRandomPalette();
        currentBlending = LINEARBLEND;
    }
}

void AnimationRotatingPalette::initialize()
{
    setOption(0);
}

void AnimationRotatingPalette::loop()
{
    _startIndex = _startIndex + 1; /* motion speed */
    fillLEDsFromPaletteColors(_startIndex);
}

} /* namespace fastled */
