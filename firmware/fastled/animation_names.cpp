#include "animation_names.h"
#include "animation_rotating_palette.h"
#include "animation_rotating_palette_cloud.h"
#include "animation_fire2012.h"
#include "debug.h"

#if FASTLED_SUPPORTED

using namespace fastled;

extern "C" {

void animation_set_for_stripe(uint8_t stripe, animation_names animation)
{
    if (stripe >= MAX_LED_STRIPES)
        return;

    uint8_t animation_index = static_cast<uint8_t>(animation);
    LV_("asfs %u %u", stripe, animation_index);

    delete led_stripe[stripe].animation;

    led_stripe_status[stripe].current_animation = animation;
    led_stripe[stripe].delay_msecs = FPS_TO_DELAY(led_stripe_status[stripe].animations[animation_index].fps);

    switch (animation)
    {
    case animation_names::AnimationRotatingPalette:
        led_stripe[stripe].animation = new fastled::AnimationRotatingPalette(led_stripe[stripe].leds, led_stripe[stripe].led_count, &led_stripe_status[stripe].animations[animation_index]);
        break;
    case animation_names::AnimationRotatingPaletteCloud:
        led_stripe[stripe].animation = new fastled::AnimationRotatingPaletteCloud(led_stripe[stripe].leds, led_stripe[stripe].led_count, &led_stripe_status[stripe].animations[animation_index]);
        break;
    case animation_names::AnimationFire2012:
    	led_stripe[stripe].animation = new fastled::AnimationFire2012(led_stripe[stripe].leds, led_stripe[stripe].led_count, &led_stripe_status[stripe].animations[animation_index]);
		break;
    }
}

}
#endif
