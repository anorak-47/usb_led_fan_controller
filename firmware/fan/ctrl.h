#ifndef __CTRL_H_INCLUDED__
#define __CTRL_H_INCLUDED__

#include <inttypes.h>
#include "config.h"

#if FAN_MODE_PI_SUPPORTED
void restartCtrl(const uint8_t channel);
#else
#define restartCtrl(channel)
#endif

void updateCtrl(const uint8_t i /* channel */);

#endif
