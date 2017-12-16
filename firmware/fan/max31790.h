#ifndef _MAX31790_H_INCLUDED__
#define _MAX31790_H_INCLUDED__

#include <inttypes.h>
#include <stdbool.h>

#define MAX_LIBRARY_VERSION 0.1.0

// Register Map
#define MAX31790_REG_GLOBAL_CONFIG 0x00
#define MAX31790_REG_PWM_FREQ 0x01
#define MAX31790_REG_FAN_CONFIG(ch) (0x02 + (ch))
#define MAX31790_REG_FAN_DYNAMICS(ch) (0x08 + (ch))
#define MAX31790_REG_FAN_FAULT_STATUS2 (0x10)
#define MAX31790_REG_FAN_FAULT_STATUS1 (0x11)
#define MAX31790_REG_FAN_FAULT_MASK (0x13)
#define MAX31790_REG_FAN_SEQU_START (0x14)
#define MAX31790_REG_TACH_COUNT(ch) (0x18 + (ch) * 2)
#define MAX31790_REG_PWM_DUTY_CYCLE(ch) (0x30 + (ch) * 2)
#define MAX31790_REG_PWMOUT(ch) (0x40 + (ch) * 2)
#define MAX31790_REG_TARGET_COUNT(ch) (0x50 + (ch) * 2)

#define REG_PWM_RATE_25 0b0000
#define REG_PWM_RATE_30 0b0001
#define REG_PWM_RATE_35 0b0010
#define REG_PWM_RATE_100 0b0011
#define REG_PWM_RATE_125 0b0100
#define REG_PWM_RATE_149 0b0101
#define REG_PWM_RATE_1250 0b0110
#define REG_PWM_RATE_1470 0b0111
#define REG_PWM_RATE_3570 0b1000
#define REG_PWM_RATE_5000 0b1001
#define REG_PWM_RATE_12500 0b1010
#define REG_PWM_RATE_25000 0b1011

#define MAX31790_CFG_STANDBY 7
#define MAX31790_CFG_RESET 6
#define MAX31790_CFG_TIMEOUT 5
#define MAX31790_CFG_OSC 3
#define MAX31790_CFG_WD2 2
#define MAX31790_CFG_WD1 1
#define MAX31790_CFG_WDS 0

#define MAX31790_CFG_WD_OFF 0b00
#define MAX31790_CFG_WD_5S 0b01
#define MAX31790_CFG_WD_10S 0b10
#define MAX31790_CFG_WD30S 0b11

/* Fan Config register bits */
#define MAX31790_FAN_CFG_RPM_MODE 7
#define MAX31790_FAN_CFG_SPINUP 5
#define MAX31790_FAN_CFG_CONTROL 4
#define MAX31790_FAN_CFG_TACH_INPUT_EN 3
#define MAX31790_FAN_CFG_MODE_LOCKED 2
#define MAX31790_FAN_CFG_MODE_POLARITY 1
#define MAX31790_FAN_CFG_TACH_INPUT 0

/* Fan Dynamics register bits */
#define MAX31790_FAN_DYN_SR_SHIFT	5
#define MAX31790_FAN_DYN_SR_MASK	0xE0
#define SR_FROM_REG(reg)		(((reg) & MAX31790_FAN_DYN_SR_MASK) \
					 >> MAX31790_FAN_DYN_SR_SHIFT)

#define FAN_RPM_MIN			120
#define FAN_RPM_MAX			7864320

#define RPM_FROM_REG(reg, sr)		(((reg) >> 4) ? \
					 ((60 * (sr) * 8192) / ((reg) >> 4)) : \
					 FAN_RPM_MAX)
#define RPM_TO_REG(rpm, sr)		((60 * (sr) * 8192) / ((rpm) * 2))

#define NR_CHANNEL	6



enum fan_mode
{
	fan_mode_pwm,
	fan_mode_input,
	fan_mode_rpm
};

struct _s_max31790_fan_config
{
    union {
        struct
        {
            uint8_t mode : 1;
            uint8_t spin_up : 2;
            uint8_t control_monitor : 1;
            uint8_t tach_input_en : 1;
            uint8_t tach_locked_rotor : 1;
            uint8_t locked_rotor_polarity : 1;
            uint8_t pwm_tach : 1;
        };
        uint8_t raw;
    } fan_config;

    union {
        struct
        {
            uint8_t speed_range : 2;
            uint8_t pwm_rate_of_change : 3;
            uint8_t asym_change : 1;
            uint8_t RESERVED : 1;
        };
        uint8_t raw;
    } fan_dynamics;
};

typedef struct _s_max31790_fan_config max31790_fan_config;

struct _s_max31790_fan_status
{
	uint16_t tach[2];
	uint16_t pwm;
	uint16_t duty;
	uint16_t target_count;
	uint8_t sr;
};

typedef struct _s_max31790_fan_status max31790_fan_status;

struct _s_max31790_config
{
    union {
        struct
        {
            uint8_t standby : 1;
            uint8_t reset : 1;
            uint8_t i2c_bus_timeout : 1;
            uint8_t RESERVED : 1;
            uint8_t ext_OSC : 1;
            uint8_t i2c_WD_time : 2;
            uint8_t wd_fault : 1;
        };
        uint8_t raw;
    } config;

    union {
    	struct
		{
    		uint8_t fan_freq_46 : 4;
    	    uint8_t fan_freq_13 : 4;
		};
    	uint8_t raw;
    } fan_freq;
};

typedef struct _s_max31790_config max31790_config;

struct _s_max31790_device
{
    uint8_t addr;
    max31790_config config;
    uint16_t fault_status;
    max31790_fan_config fan_config[NR_CHANNEL];
    max31790_fan_status fan_status[NR_CHANNEL];
};

typedef struct _s_max31790_device max31790_device;

extern max31790_device max31790device;

void max31790_initialize(max31790_device *max31790device);
void max31790_configure(max31790_device *max31790device, max31790_config *config);

void max31790_reset(max31790_device *max31790device);
void max31790_standby(max31790_device *max31790device, bool standby);
bool max31790_read_global_config(max31790_device *max31790device, max31790_config *config);

bool max31790_read_fan_status(max31790_device *max31790device, uint8_t fan);
bool max31790_read_fan_states(max31790_device *max31790device);

bool max31790_is_fan_fault(max31790_device *max31790device, uint8_t fan);
enum fan_mode max31790_get_fan_mode(max31790_device *max31790device, uint8_t fan);
bool max31790_set_fan_mode(max31790_device *max31790device, enum fan_mode mode, uint8_t fan);

uint16_t max31790_get_fan_rpm(max31790_device *max31790device, uint8_t fan);
uint16_t max31790_get_fan_rpm_target(max31790_device *max31790device, uint8_t fan);
uint16_t max31790_get_fan_pwm(max31790_device *max31790device, uint8_t fan);
uint16_t max31790_get_fan_duty(max31790_device *max31790device, uint8_t fan);

bool max31790_set_fan_rpm(max31790_device *max31790device, uint16_t rpm, uint8_t fan);
bool max31790_set_fan_pwm(max31790_device *max31790device, uint16_t pwm, uint8_t fan);


#if 0
// Get functions
uint16_t max31790_getRPM(max31790_device *max31790device, uint8_t fan);
uint16_t max31790_getRPMTarget(max31790_device *max31790device, uint8_t fan);

// Set functions
void max31790_setRPM(max31790_device *max31790device, uint8_t fan, uint16_t rpm);
void max31790_setPWM(max31790_device *max31790device, uint8_t fan, uint16_t pwm);

void max31790_setPWMMode(max31790_device *max31790device, uint8_t fan);
void max31790_setRPMMode(max31790_device *max31790device, uint8_t fan);
#endif
#endif
