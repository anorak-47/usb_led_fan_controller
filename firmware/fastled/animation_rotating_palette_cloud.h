#ifndef FIRMWARE_FASTLED_ANIMATION_ROTATING_PALETTE_CLOUD_H_
#define FIRMWARE_FASTLED_ANIMATION_ROTATING_PALETTE_CLOUD_H_

#include "animation_rotating_palette.h"

namespace fastled
{

class AnimationRotatingPaletteCloud : public AnimationRotatingPalette
{
public:
    AnimationRotatingPaletteCloud(CRGB *leds, uint8_t led_count, animation *animation_info);
    virtual ~AnimationRotatingPaletteCloud();

    virtual void initialize() override;
};

} /* namespace fastled */

#endif /* FIRMWARE_FASTLED_ANIMATION_ROTATING_PALETTE_CLOUD_H_ */
