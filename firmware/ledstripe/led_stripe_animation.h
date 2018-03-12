#ifndef FIRMWARE_LEDSTRIPE_LED_STRIPE_ANIMATION_H_
#define FIRMWARE_LEDSTRIPE_LED_STRIPE_ANIMATION_H_

#include "animation_type.h"
#include "config.h"

namespace fastled
{

class LedStripeAnimation
{
public:
    LedStripeAnimation(CRGB *leds, uint8_t led_count, animation *animation_info);
    virtual ~LedStripeAnimation();

    virtual void initialize();
    virtual void loop() = 0;
    virtual void deinitialize();

    virtual void setOption(uint8_t option);

protected:
    CRGB *_leds = 0;
    uint8_t _led_count = 0;
    animation *_animation_info = 0;
};

} /* namespace fastled */

#endif /* FIRMWARE_LEDSTRIPE_LED_STRIPE_ANIMATION_H_ */
