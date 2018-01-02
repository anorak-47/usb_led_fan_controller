#ifndef FIRMWARE_FASTLED_LED_STRIPE_ANIMATION_H_
#define FIRMWARE_FASTLED_LED_STRIPE_ANIMATION_H_

#include "config.h"
#include "animation_type.h"

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
protected:
    CRGB *_leds;
    uint8_t _led_count;
    animation *_animation_info;
};

} /* namespace fastled */

#endif /* FIRMWARE_FASTLED_LED_STRIPE_ANIMATION_H_ */
