
#include "led_stripe_animation.h"

namespace fastled
{

LedStripeAnimation::LedStripeAnimation(CRGB *leds, uint8_t led_count, animation *animation_info)
    : _leds(leds), _led_count(led_count), _animation_info(animation_info)
{
}

LedStripeAnimation::~LedStripeAnimation()
{
}

void LedStripeAnimation::initialize()
{
	setOption(0);
}

void LedStripeAnimation::deinitialize()
{
	fill_solid(_leds, _led_count, CRGB::Black);
}

void LedStripeAnimation::setOption(uint8_t)
{
	// do nothing
}

} /* namespace fastled */
