#pragma once

#include "led_stripe_animation.h"

namespace fastled
{

class AnimationGradient : public LedStripeAnimation
{
public:
    AnimationGradient(CRGB *leds, uint8_t led_count, animation *animation_info);
    virtual ~AnimationGradient();

    virtual void initialize() override;
    virtual void loop() override;
    virtual void setOption(uint8_t option) override;

protected:
	uint8_t _startIndex = 0;
};

} /* namespace fastled */
