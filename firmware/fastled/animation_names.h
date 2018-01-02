#pragma once

#include "config.h"
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#if FASTLED_SUPPORTED

enum _e_animation_names
{
	AnimationRotatingPalette,
	AnimationRotatingPaletteCloud,
	AnimationFire2012
};

typedef enum _e_animation_names animation_names;

#define ANIMATION_COUNT 3

void animation_set_for_stripe(uint8_t stripe, animation_names animation);

#endif

#ifdef __cplusplus
}
#endif
