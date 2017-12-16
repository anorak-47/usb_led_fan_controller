#pragma once
#define FASTLED_INTERNAL
#include <FastLED.h>
#include <inttypes.h>

struct _s_animation_type
{
    uint16_t delay_msecs;
    CHSV hsv[2];
    uint8_t parameters;
    uint8_t parameter[2];
};

typedef struct _s_animation_type animation;

struct _s_animation_status
{
	uint8_t current_animation;
	bool is_running;
	uint16_t delay_msecs;

    void (*animationStart)(void);
    void (*animationStop)(void);
    void (*animationLoop)(void);
};

typedef struct _s_animation_status animation_status;
