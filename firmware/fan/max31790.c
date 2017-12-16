/*************************************************
 *
 *	MAX31790 Device Class
 *	C interface by Moritz Wenk (moritzwenk@web.de)
 *
 *************************************************/

#include "max31790.h"
#include <stdbool.h>

void I2Cdev_writeBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data);
void I2Cdev_writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);
void I2Cdev_writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data);
int8_t I2Cdev_readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data);
int8_t I2Cdev_readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);

void I2Cdev_writeBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data)
{
}
void I2Cdev_writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data)
{
}
void I2Cdev_writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data)
{
}
int8_t I2Cdev_readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data)
{
	return 0;
}
int8_t I2Cdev_readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data)
{
	return 0;
}

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#define clamp_val(val, lo, hi) MIN(MAX(val, lo), hi)

max31790_device max31790device;

static uint8_t buffer[2];
static const uint8_t tach_period[8] = { 1, 2, 4, 8, 16, 32, 32, 32 };

static uint8_t get_tach_period(uint8_t fan_dynamics)
{
	return tach_period[SR_FROM_REG(fan_dynamics)];
}

static uint8_t bits_for_tach_period(int rpm)
{
	uint8_t bits;

	if (rpm < 500)
		bits = 0x0;
	else if (rpm < 1000)
		bits = 0x1;
	else if (rpm < 2000)
		bits = 0x2;
	else if (rpm < 4000)
		bits = 0x3;
	else if (rpm < 8000)
		bits = 0x4;
	else
		bits = 0x5;

	return bits;
}

void max31790_initialize(max31790_device *max31790device)
{
	I2Cdev_readByte(max31790device->addr, MAX31790_REG_GLOBAL_CONFIG, &(max31790device->config.config.raw));
    I2Cdev_readByte(max31790device->addr, MAX31790_REG_PWM_FREQ, &(max31790device->config.fan_freq.raw));

    for (int fan = 0; fan < NR_CHANNEL; fan++)
    {
    	I2Cdev_readByte(max31790device->addr, MAX31790_REG_FAN_CONFIG(fan), &(max31790device->fan_config[fan].fan_config.raw));
    	I2Cdev_readByte(max31790device->addr, MAX31790_REG_FAN_DYNAMICS(fan), &(max31790device->fan_config[fan].fan_dynamics.raw));
    	uint8_t sr = get_tach_period(max31790device->fan_config[fan].fan_dynamics.raw);
    	max31790device->fan_status[fan].sr = sr;
    }
}

void max31790_configure(max31790_device *max31790device, max31790_config *config)
{
    I2Cdev_writeByte(max31790device->addr, MAX31790_REG_GLOBAL_CONFIG, config->config.raw);
    I2Cdev_writeByte(max31790device->addr, MAX31790_REG_PWM_FREQ, config->fan_freq.raw);
}

void max31790_standby(max31790_device *max31790device, bool standby)
{
	I2Cdev_writeBit(max31790device->addr, MAX31790_REG_GLOBAL_CONFIG, MAX31790_CFG_STANDBY, standby ? 1 : 0);
}

void max31790_reset(max31790_device *max31790device)
{
	I2Cdev_writeBit(max31790device->addr, MAX31790_REG_GLOBAL_CONFIG, MAX31790_CFG_RESET, 1);
}

bool max31790_read_global_config(max31790_device *max31790device, max31790_config *config)
{
	I2Cdev_readByte(max31790device->addr, MAX31790_REG_GLOBAL_CONFIG, &config->config.raw);
	I2Cdev_readByte(max31790device->addr, MAX31790_REG_PWM_FREQ, &config->fan_freq.raw);
	return true;
}

bool max31790_read_fan_status_internal(max31790_device *max31790device, max31790_fan_status *fan_status, uint8_t fan)
{
	I2Cdev_readBytes(max31790device->addr, MAX31790_REG_TACH_COUNT(fan), 2, buffer);
	fan_status->tach[0] = ((((int16_t)buffer[0]) << 8) | buffer[1]);

	if (max31790device->fan_config[fan].fan_config.raw & _BV(MAX31790_FAN_CFG_TACH_INPUT))
	{
		I2Cdev_readBytes(max31790device->addr, MAX31790_REG_TACH_COUNT(NR_CHANNEL + fan), 2, buffer);
		fan_status->tach[1] = ((((int16_t)buffer[0]) << 8) | buffer[1]);
	}
	else
	{
		I2Cdev_readBytes(max31790device->addr, MAX31790_REG_PWM_DUTY_CYCLE(fan), 2, buffer);
		fan_status->duty = ((((int16_t)buffer[0]) << 8) | buffer[1]);

		I2Cdev_readBytes(max31790device->addr, MAX31790_REG_PWMOUT(fan), 2, buffer);
		fan_status->pwm = ((((int16_t)buffer[0]) << 8) | buffer[1]);

		I2Cdev_readBytes(max31790device->addr, MAX31790_REG_TARGET_COUNT(fan), 2, buffer);
		fan_status->target_count = ((((int16_t)buffer[0]) << 8) | buffer[1]);
	}

	return true;
}

bool max31790_read_fan_status(max31790_device *max31790device, uint8_t fan)
{
	return max31790_read_fan_status_internal(max31790device, &max31790device->fan_status[fan], fan);
}

bool max31790_read_fan_states(max31790_device *max31790device)
{
	for (uint8_t i = 0; i < NR_CHANNEL; i++)
		max31790_read_fan_status(max31790device, i);
	return true;
}

bool max31790_is_fan_fault(max31790_device *max31790device, uint8_t fan)
{
	return !!(max31790device->fault_status & (1 << fan));
}

uint16_t max31790_get_fan_rpm(max31790_device *max31790device, uint8_t fan)
{
	uint16_t rpm = RPM_FROM_REG(max31790device->fan_status[fan].tach[0], max31790device->fan_status[fan].sr);
	return rpm;
}

uint16_t max31790_get_fan_rpm_target(max31790_device *max31790device, uint8_t fan)
{
	uint16_t rpm = RPM_FROM_REG(max31790device->fan_status[fan].target_count, max31790device->fan_status[fan].sr);
	return rpm;
}

bool max31790_set_fan_rpm(max31790_device *max31790device, uint16_t rpm, uint8_t fan)
{
    rpm = clamp_val(rpm, FAN_RPM_MIN, FAN_RPM_MAX);
    uint8_t bits = bits_for_tach_period(rpm);
    max31790device->fan_config[fan].fan_dynamics.raw =
        ((max31790device->fan_config[fan].fan_dynamics.raw & ~MAX31790_FAN_DYN_SR_MASK) | (bits << MAX31790_FAN_DYN_SR_SHIFT));

    I2Cdev_writeByte(max31790device->addr, MAX31790_REG_FAN_DYNAMICS(fan), max31790device->fan_config[fan].fan_dynamics.raw);

    uint16_t sr = get_tach_period(max31790device->fan_config[fan].fan_dynamics.raw);
    max31790device->fan_status[fan].sr = sr;
    uint16_t target_count = RPM_TO_REG(rpm, sr);
    target_count = clamp_val(target_count, 0x1, 0x7FF);

    max31790device->fan_status[fan].target_count = target_count << 5;

    I2Cdev_writeByte(max31790device->addr, MAX31790_REG_TARGET_COUNT(fan), max31790device->fan_status[fan].target_count);

    return true;
}

uint16_t max31790_get_fan_pwm(max31790_device *max31790device, uint8_t fan)
{
	uint16_t pwm = max31790device->fan_status[fan].pwm;
	return (pwm >> 8);
}

uint16_t max31790_get_fan_duty(max31790_device *max31790device, uint8_t fan)
{
	uint16_t pwm = max31790device->fan_status[fan].duty;
	return (pwm >> 8);
}

bool max31790_set_fan_pwm(max31790_device *max31790device, uint16_t pwm, uint8_t fan)
{
	max31790device->fan_status[fan].pwm = (pwm >> 8) | (pwm << 15 & 0x8000);
	// MAX31790_REG_PWM_DUTY_CYCLE ???
    I2Cdev_writeBytes(max31790device->addr, MAX31790_REG_PWMOUT(fan), 2, (uint8_t*)&(max31790device->fan_status[fan].pwm));
    return true;
}

enum fan_mode max31790_get_fan_mode(max31790_device *max31790device, uint8_t fan)
{
	if (max31790device->fan_config[fan].fan_config.raw & _BV(MAX31790_FAN_CFG_RPM_MODE))
		return fan_mode_rpm;
	if (max31790device->fan_config[fan].fan_config.raw & _BV(MAX31790_FAN_CFG_TACH_INPUT_EN))
		return fan_mode_input;

	return fan_mode_pwm;
}

bool max31790_set_fan_mode(max31790_device *max31790device, enum fan_mode mode, uint8_t fan)
{
    uint8_t fan_config = max31790device->fan_config[fan].fan_config.raw;

    switch (mode)
    {
    case fan_mode_pwm:
        fan_config &= ~(MAX31790_FAN_CFG_TACH_INPUT_EN | MAX31790_FAN_CFG_RPM_MODE);
        break;
    case fan_mode_input:
        fan_config = (fan_config | MAX31790_FAN_CFG_TACH_INPUT_EN) & ~MAX31790_FAN_CFG_RPM_MODE;
        break;
    case fan_mode_rpm:
        //fan_config |= MAX31790_FAN_CFG_TACH_INPUT_EN | MAX31790_FAN_CFG_RPM_MODE;
        fan_config = (fan_config | MAX31790_FAN_CFG_RPM_MODE) & ~MAX31790_FAN_CFG_TACH_INPUT_EN;
        break;
    }

    I2Cdev_writeByte(max31790device->addr, MAX31790_REG_GLOBAL_CONFIG, fan_config);

    max31790device->fan_config[fan].fan_config.raw = fan_config;

    return true;
}



#if 0

uint16_t max31790_getRPM(max31790_device *max31790device, uint8_t fan_num)
{
    if (!I2Cdev_readBytes(max31790device->addr, MAX31790_REG_TACH_COUNT(fan_num), 2, buffer))
    	return 0;

    uint16_t tach_out = ((((int16_t)buffer[0]) << 8) | buffer[1]) >> 5;

    // convert to RPM
    return 60 * _SR * 8192 / (NP * tach_out);
}

uint16_t max31790_getRPMTarget(max31790_device *max31790device, uint8_t fan_num)
{
    I2Cdev_readBytes(max31790device->addr, MAX31790_REG_TARGET_COUNT(fan_num), 2, buffer);
    uint16_t tach_out = ((((int16_t)buffer[0]) << 8) | buffer[1]) >> 5;

    // convert to RPM
    return 60 * _SR * 8192 / (NP * tach_out);
}

void max31790_setRPM(max31790_device *max31790device, uint8_t fan_num, uint16_t rpm)
{
    uint16_t tach_count = 60 * _SR * 8192 / (NP * rpm) << 5;
    if (rpm == 0)
    {
        tach_count = 0b1111111111111111;
    }
    buffer[0] = tach_count >> 8;
    buffer[1] = tach_count;

    I2Cdev_writeBytes(max31790device->addr, MAX31790_REG_TARGET_COUNT(fan_num), 2, buffer);
}

void max31790_setPWM(max31790_device *max31790device, uint8_t fan_num, uint16_t pwm)
{
    uint16_t pwm_bit = pwm << 7;
    buffer[0] = pwm >> 8;
    buffer[1] = pwm_bit;

    I2Cdev_writeBytes(max31790device->addr, MAX31790_REG_PWMOUT(fan_num), 2, buffer);
}

void max31790_setRPMMode(max31790_device *max31790device, uint8_t fan_num)
{
    I2Cdev_writeBit(max31790device->addr, MAX31790_REG_FAN_CONFIG(fan_num), REG_FAN_CONFIG_MODE, 1);
}

void max31790_setPWMMode(max31790_device *max31790device, uint8_t fan_num)
{
    I2Cdev_writeBit(max31790device->addr, MAX31790_REG_FAN_CONFIG(fan_num), REG_FAN_CONFIG_MODE, 0);
}

#endif

