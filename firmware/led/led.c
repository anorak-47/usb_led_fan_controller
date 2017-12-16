
#include "led.h"
#include "config.h"
#include "i2cmaster.h"

#if LED_PCA8574_SUPPORTED

// Note: LEDs are LOW active!

static uint8_t _led_mask = 0;
const uint8_t _addr_pcf8574_0 = 0x40; // addr[0-3] connected to GND

void i2c_write_byte(uint8_t addr, uint8_t data)
{
    i2c_start_wait(addr + I2C_WRITE); // set device address and write mode
    i2c_write(data);                  // write data
    i2c_stop();                       // set stop conditon = release bus
}

void set_led(uint8_t led)
{
    _led_mask |= led;
    i2c_write_byte(_addr_pcf8574_0, ~_led_mask);
}

void unset_led(uint8_t led)
{
    _led_mask &= ~led;
    i2c_write_byte(_addr_pcf8574_0, ~_led_mask);
}

void set_leds_by_mask(uint8_t mask)
{
    _led_mask = mask;
    i2c_write_byte(_addr_pcf8574_0, ~mask);
}

uint8_t get_led_mask(void)
{
    return _led_mask;
}

#endif
