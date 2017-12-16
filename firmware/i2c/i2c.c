#include "i2c.h"
#include "i2cmaster.h"

/*************************************************************************
  Issues a start condition and sends address and transfer direction.
  return true = device accessible, false = failed to access device
*************************************************************************/
bool i2c_scan_address(uint8_t addr)
{
    return (i2c_start(addr) == 0);
}
