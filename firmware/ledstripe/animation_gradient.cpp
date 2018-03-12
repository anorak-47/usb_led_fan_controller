
#include "animation_gradient.h"

namespace fastled
{

AnimationGradient::AnimationGradient(CRGB *leds, uint8_t led_count, animation *animation_info)
    : LedStripeAnimation(leds, led_count, animation_info)
{
}

AnimationGradient::~AnimationGradient()
{
}

void AnimationGradient::setOption(uint8_t option)
{
    if (option == 0)
    {
    }
}

void AnimationGradient::initialize()
{
    fill_gradient(_leds, 0, _animation_info->hsv[0], _led_count-1, _animation_info->hsv[1], SHORTEST_HUES);
}

void AnimationGradient::loop()
{
    //_startIndex = _startIndex + 1; /* motion speed */
    //fill_gradient(_leds, 0, _animation_info->hsv[0], _led_count-1, _animation_info->hsv[1], SHORTEST_HUES);
}

} /* namespace fastled */
