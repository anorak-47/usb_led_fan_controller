#ifndef FIRMWARE_LEDSTRIPE_ANIMATION_ROTATING_PALETTE_H_
#define FIRMWARE_LEDSTRIPE_ANIMATION_ROTATING_PALETTE_H_

#include <ledstripe/led_stripe_animation.h>

namespace fastled
{

class AnimationRotatingPalette : public LedStripeAnimation
{
public:
    AnimationRotatingPalette(CRGB *leds, uint8_t led_count, animation *animation_info);
    virtual ~AnimationRotatingPalette();

    virtual void loop() override;
    virtual void setOption(uint8_t option) override;

protected:
    void fillLEDsFromPaletteColors(uint8_t colorIndex);
    void SetupPurpleAndGreenPalette();
    void SetupTotallyRandomPalette();

    CRGBPalette16 currentPalette;
    TBlendType currentBlending = NOBLEND;
	uint8_t _startIndex = 0;
};

} /* namespace fastled */

#endif /* FIRMWARE_LEDSTRIPE_ANIMATION_ROTATING_PALETTE_H_ */
