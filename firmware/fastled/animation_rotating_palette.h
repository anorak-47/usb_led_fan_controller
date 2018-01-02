#ifndef FIRMWARE_FASTLED_ANIMATION_ROTATING_PALETTE_H_
#define FIRMWARE_FASTLED_ANIMATION_ROTATING_PALETTE_H_

#include "led_stripe_animation.h"

namespace fastled
{

class AnimationRotatingPalette : public LedStripeAnimation
{
public:
    AnimationRotatingPalette(CRGB *leds, uint8_t led_count, animation *animation_info);
    virtual ~AnimationRotatingPalette();

    virtual void initialize() override;
    virtual void loop() override;

protected:
    void fillLEDsFromPaletteColors(uint8_t colorIndex);

    CRGBPalette16 currentPalette;
    TBlendType currentBlending = NOBLEND;
	uint8_t _startIndex = 0;
};

} /* namespace fastled */

#endif /* FIRMWARE_FASTLED_ANIMATION_ROTATING_PALETTE_H_ */
