#ifndef __MCP980X_H_INCLUDED__
#define __MCP980X_H_INCLUDED__

#include "config.h"
#include <inttypes.h>

#if SNS_I2C_SUPPORTED
int8_t readI2CMcp980x(uint8_t addr);
#endif

#endif
