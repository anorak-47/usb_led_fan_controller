
#include <ledstripe/led_stripe_animation.h>

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
	// do nothing
}

void LedStripeAnimation::deinitialize()
{
    for (int i = 0; i < _led_count; i++)
        _leds[i] = CRGB::Black;
}

void LedStripeAnimation::setOption(uint8_t)
{
	// do nothing
}

} /* namespace fastled */
