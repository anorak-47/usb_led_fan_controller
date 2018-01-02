#include "animation.h"
#include "Arduino.h"
#include "animation_rotating_palette.h"
#include "animation_rotating_palette_cloud.h"
#include "animation_setup.h"
#include "debug.h"
#include <avr/eeprom.h>

#if FASTLED_SUPPORTED

using namespace fastled;

extern "C" {

#define EEPROM_CHK_MAGIC (0xA5)     // magic number to 'assure' empty/garbage eeprom sections can be detected
uint8_t EEMEM chk_animation_eeprom; // assure this variable follows the eeprom section which must be checksum-checked!

extern void initialize_honode(void);

animation animations[ANIMATION_COUNT];
animation EEMEM animations_eeprom[ANIMATION_COUNT];

led_stripe_animation_status led_stripe_status[MAX_LED_STRIPES];
led_stripe_animation_status EEMEM led_stripe_status_eeprom[MAX_LED_STRIPES];

led_stripe_animation led_stripe[MAX_LED_STRIPES];

void animation_reset(void)
{
    L__(animation_reset);
    for (uint8_t i = 0; i < MAX_LED_STRIPES; i++)
    {
        memset(&led_stripe[i], 0, sizeof(led_stripe_animation));
        led_stripe[i].delay_msecs = FPS_TO_DELAY(10);

        led_stripe_status[i].autoplay = false;
        led_stripe_status[i].current_animation = animation_names::AnimationRotatingPalette;

        for (uint8_t a = 0; a < ANIMATION_COUNT; a++)
        {
            led_stripe_status[i].animations[a].fps = 10;
            led_stripe_status[i].animations[a].sensor_index = 0;
            // led_stripe_status[i].animations[a].hsv =
        }
    }
}

void animation_load(bool clear)
{
    L__(animation_load);
    animation_reset();

    if (clear)
    	return;

    uint8_t magic = eeprom_read_byte(&chk_animation_eeprom);

    if (magic == EEPROM_CHK_MAGIC)
    {
        for (uint8_t i = 0; i < MAX_LED_STRIPES; i++)
        {
            eeprom_read_block((void *)&(led_stripe_status[i]), &(led_stripe_status_eeprom[i]), sizeof(led_stripe_status[i]));

            animation_set_for_stripe(i, led_stripe_status[i].current_animation);

            if (led_stripe_status[i].autoplay)
                animation_start(i);
        }
    }
}

void animation_save(void)
{
    for (uint8_t i = 0; i < MAX_LED_STRIPES; i++)
    {
        eeprom_update_block((void *)&(led_stripe_status[i]), &(led_stripe_status_eeprom[i]), sizeof(led_stripe_status[i]));
    }

    eeprom_update_byte(&chk_animation_eeprom, EEPROM_CHK_MAGIC);
}

void animation_init(void)
{
    initialize_honode();
    animation_load(false);
    led_stripe_setup();
}

void animation_start(uint8_t stripe)
{
    LV_("astart %u", stripe);
    if (led_stripe[stripe].animation)
    {
        led_stripe[stripe].is_running = true;
        led_stripe[stripe].loop_timer = millis();
    }
}

void animation_stop(uint8_t stripe)
{
    LV_("astop %u", stripe);
    led_stripe[stripe].is_running = false;
}

void animation_set_running(uint8_t stripe, bool running)
{
	if (running)
		animation_start(stripe);
	else
		animation_stop(stripe);
}

void animation_set_current_fps(uint8_t stripe, uint8_t fps)
{
	animation_set_fps(stripe, led_stripe_status[stripe].current_animation, fps);
}

animation_names animation_get_current(uint8_t stripe)
{
    return led_stripe_status[stripe].current_animation;
}

void animation_set_current(uint8_t stripe, animation_names animation)
{
	animation_set_for_stripe(stripe, animation);
}

bool animation_get_current_running(uint8_t stripe)
{
    return led_stripe[stripe].is_running;
}

uint8_t animation_get_current_fps(uint8_t stripe)
{
    return DELAY_TO_FPS(led_stripe[stripe].delay_msecs);
}

void animation_set_fps(uint8_t stripe, animation_names animation, uint8_t fps)
{
    led_stripe_status[stripe].animations[static_cast<uint8_t>(animation)].fps = fps;

    if (led_stripe_status[stripe].current_animation == animation)
    {
        led_stripe[stripe].delay_msecs = FPS_TO_DELAY(fps);
    }

    LV_("afps %u %u %u", stripe, fps, led_stripe[stripe].delay_msecs);
}

uint8_t animation_get_fps(uint8_t stripe, animation_names animation)
{
    return led_stripe_status[stripe].animations[static_cast<uint8_t>(animation)].fps;
}

bool animation_get_autoplay(uint8_t stripe)
{
    return led_stripe_status[stripe].autoplay;
}

void animation_set_autoplay(uint8_t stripe, bool autoplay)
{
    led_stripe_status[stripe].autoplay = autoplay;
}

void animation_set_color(uint8_t stripe, animation_names animation, uint8_t cnr, uint8_t hsv[3])
{
    led_stripe_status[stripe].animations[static_cast<uint8_t>(animation)].hsv[cnr] = CHSV(hsv[0], hsv[1], hsv[2]);
}

void animation_get_color(uint8_t stripe, animation_names animation, uint8_t cnr, uint8_t hsv[3])
{
    hsv[0] = led_stripe_status[stripe].animations[static_cast<uint8_t>(animation)].hsv[cnr].hue;
    hsv[1] = led_stripe_status[stripe].animations[static_cast<uint8_t>(animation)].hsv[cnr].saturation;
    hsv[2] = led_stripe_status[stripe].animations[static_cast<uint8_t>(animation)].hsv[cnr].value;
}

void animation_set_current_color(uint8_t stripe, uint8_t cnr, uint8_t hsv[3])
{
    animation_set_color(stripe, led_stripe_status[stripe].current_animation, cnr, hsv);
}

void animation_get_current_color(uint8_t stripe, uint8_t cnr, uint8_t hsv[3])
{
	animation_get_color(stripe, led_stripe_status[stripe].current_animation, cnr, hsv);
}

void animation_set_sensor_index(uint8_t stripe, animation_names animation, uint8_t sensor_index)
{
	led_stripe_status[stripe].animations[static_cast<uint8_t>(animation)].sensor_index = sensor_index;
}

uint8_t animation_get_sensor_index(uint8_t stripe, animation_names animation)
{
	return led_stripe_status[stripe].animations[static_cast<uint8_t>(animation)].sensor_index;
}

void animation_set_current_sensor_index(uint8_t stripe, uint8_t sensor_index)
{
	animation_set_sensor_index(stripe, led_stripe_status[stripe].current_animation, sensor_index);
}

uint8_t animation_get_current_sensor_index(uint8_t stripe)
{
	return animation_get_sensor_index(stripe, led_stripe_status[stripe].current_animation);
}

#define TIMER_DIFF(a, b, max) ((a) >= (b) ? (a) - (b) : (max) - (b) + (a))
#define TIMER_DIFF_8(a, b) TIMER_DIFF(a, b, UINT8_MAX)
#define TIMER_DIFF_16(a, b) TIMER_DIFF(a, b, UINT16_MAX)
#define TIMER_DIFF_32(a, b) TIMER_DIFF(a, b, UINT32_MAX)
#define TIMER_DIFF_RAW(a, b) TIMER_DIFF_8(a, b)

inline uint16_t timer_elapsed(uint16_t last)
{
    uint32_t t;

    // uint8_t sreg = SREG;
    // cli();
    t = millis();
    // SREG = sreg;

    return TIMER_DIFF_16((t & 0xFFFF), last);
}

inline uint32_t timer_elapsed32(uint32_t last)
{
    uint32_t t;

    // uint8_t sreg = SREG;
    // cli();
    t = millis();
    // SREG = sreg;

    return TIMER_DIFF_32(t, last);
}

void animation_loop()
{
    for (uint8_t a = 0; a < MAX_LED_STRIPES; a++)
    {
        if (led_stripe[a].is_running && led_stripe[a].animation)
        {
            if (timer_elapsed(led_stripe[a].loop_timer) >= led_stripe[a].delay_msecs)
            {
                led_stripe[a].loop_timer = millis();
                led_stripe[a].animation->loop();
            }
        }
    }

    FastLED.show();
}
}
#endif
