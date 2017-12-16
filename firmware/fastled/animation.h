#pragma once

#include "animation_type.h"
#include "config.h"
#include <avr/eeprom.h>

#define MAX_FLED_ANIM 3

extern animation animations[MAX_FLED_ANIM];
extern animation EEMEM animations_eeprom[MAX_FANS];
