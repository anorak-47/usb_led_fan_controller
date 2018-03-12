#pragma once

#include "config.h"
#include "animation_names.h"
#include <inttypes.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#if FASTLED_SUPPORTED

void animation_init(void);
void animation_load(bool clear);
void animation_save(void);
void animation_start(uint8_t stripe);
void animation_stop(uint8_t stripe);
void animation_loop(void);

void animation_load_stripe(uint8_t stripe);
void animation_save_stripe(uint8_t stripe);

void animation_set_running(uint8_t stripe, bool running);
bool animation_get_running(uint8_t stripe);

animation_names animation_get_current(uint8_t stripe);
void animation_set_current(uint8_t stripe, animation_names animation);

uint8_t animation_get_current_fps(uint8_t stripe);
void animation_set_current_fps(uint8_t stripe, uint8_t fps);

void animation_set_current_color(uint8_t stripe, uint8_t cnr, uint8_t hsv[3]);
void animation_get_current_color(uint8_t stripe, uint8_t cnr, uint8_t hsv[3]);

void animation_set_current_sensor_index(uint8_t stripe, uint8_t sensor_index);
uint8_t animation_get_current_sensor_index(uint8_t stripe);

void animation_set_autoplay(uint8_t stripe, bool autoplay);
bool animation_get_autoplay(uint8_t stripe);

void animation_set_fps(uint8_t stripe, animation_names animation, uint8_t fps);
uint8_t animation_get_fps(uint8_t stripe, animation_names animation);

void animation_set_sensor_index(uint8_t stripe, animation_names animation, uint8_t sensor_index);
uint8_t animation_get_sensor_index(uint8_t stripe, animation_names animation);

void animation_set_color(uint8_t stripe, animation_names animation, uint8_t cnr, uint8_t hsv[3]);
void animation_get_color(uint8_t stripe, animation_names animation, uint8_t cnr, uint8_t hsv[3]);

void animation_set_current_option(uint8_t stripe, uint8_t option);
uint8_t animation_get_current_option(uint8_t stripe);

void animation_set_option(uint8_t stripe, animation_names animation, uint8_t option);
uint8_t animation_get_option(uint8_t stripe, animation_names animation);

void animation_set_global_brightness(uint8_t brightness);
uint8_t animation_get_global_brightness(void);

#else
#define animation_init() do {} while(0)
#define animation_loop() do {} while(0)
#endif

#ifdef __cplusplus
}
#endif
