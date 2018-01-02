#ifndef FIRMWARE_LEDSTRIPE_ANIMATION_FIRE2012_H_
#define FIRMWARE_LEDSTRIPE_ANIMATION_FIRE2012_H_

#include <ledstripe/led_stripe_animation.h>

namespace fastled
{

class AnimationFire2012 : public LedStripeAnimation
{
public:
    AnimationFire2012(CRGB *leds, uint8_t led_count, animation *animation_info);
    virtual ~AnimationFire2012();

    virtual void initialize() override;
    virtual void loop() override;

protected:
    void Fire2012WithPalette();

    // Array of temperature readings at each simulation cell
    uint8_t *heat;

    bool gReverseDirection = false;
    CRGBPalette16 gPal;
};

} /* namespace fastled */

#endif /* FIRMWARE_LEDSTRIPE_ANIMATION_FIRE2012_H_ */
