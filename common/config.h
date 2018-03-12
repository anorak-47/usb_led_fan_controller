/*
    USB Fan Controller - Flexible PWM Fan Controller firmware

    Copyright (C) 2012  Ivo Pullens (info@emmission.nl)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef __CONFIG_H_INCLUDED__
#define __CONFIG_H_INCLUDED__

#define VERSION_MAJOR (2)
#define VERSION_MINOR (0)

#define USB_PROTOCOL_VERSION (1)


#define DEBUG_OUTPUT_SUPPORTED              1
#define DEBUG_OUTPUT_USE_UART               1
#define DEBUG_OUTPUT_USE_USB                0
#define DEBUG_FUNCTIONS_SUPPORTED           1

#define FAN_CONTROL_SUPPORTED               1

#define SNS_ANALOG_SUPPORTED                0   // Support analog NTC sensors
#define SNS_I2C_SUPPORTED                   0   // Support I2C sensors
#define FAN_OUT_SUPPORTED                   0   // Support simulated tacho signal to motherboard
#define PWM_EXACT_0_AND_100_PERCENT_DUTY    1   // Force generation of exact 0 & 100% duty cycles for all channels
#define ENTER_BOOTLOADER_REMOTELY           0   // Enable to enter bootloader for firmware updates from software
#define EEPROM_UPDOWNLOAD                   0   // Enable up- & download of full eeprom contents.
#define FAN_MODE_TRIP_POINTS_SUPPORTED      1   // Trip Point Controller
#define FAN_MODE_LIN_TRIP_POINTS_SUPPORTED  1   // Trip Point Controller with linear interpolation between trip points
#define FAN_MODE_LINEAR_SUPPORTED           1   // Linear fan control support, using gain/offset.
#define FAN_MODE_PI_SUPPORTED               1   // PI-controller support.
#define FAN_MODE_FUZZY_SUPPORTED            1   // Fuzzy conrtroller support. Only prepared, not implemented yet!
#define FAN_PWM_MAX31790_SUPPORTED          0   // Use a MAX31790 to control and monitor the fans. If disabled, interal fan control is used
#define FASTLED_SUPPORTED                   1   // Enable FASTled library and animations
#define POWER_METER_SUPPORTED               0   // Add support for power meters based on INA219 or INA260
#define LED_PCA8574_SUPPORTED               0   // Enable LED output using PCA8574
#define CTRL_DEBUG                          1   // Enable extra functions to debug controller behavior.

// If the USB comm watchdog timeout has a value > 0
// the given value in seconds determines the maximum
// time between any two USB commands received before the
// communication watchdog gets triggered and a safe situation
// will be entered.
//#define USB_COMM_WATCHDOG_TIMEOUT (10)
#define USB_COMM_WATCHDOG_TIMEOUT (0)

// Always define the macro's which influence the external interface
// when not compiling on AVR (e.g. for the PC interface) so enum values
// etc. will be known, even when not compiled in the firmware.
#ifndef __AVR__
#if SNS_ANALOG_SUPPORTED != 1
#undef SNS_ANALOG_SUPPORTED
#define SNS_ANALOG_SUPPORTED               1
#endif
#if SNS_I2C_SUPPORTED != 1
#undef SNS_I2C_SUPPORTED
#define SNS_I2C_SUPPORTED                  1
#endif
#if FAN_OUT_SUPPORTED != 1
#undef FAN_OUT_SUPPORTED
#define FAN_OUT_SUPPORTED                  1
#endif
#if USB_COMM_WATCHDOG_TIMEOUT <= 0
#undef USB_COMM_WATCHDOG_TIMEOUT
#define USB_COMM_WATCHDOG_TIMEOUT (10)
#endif
#if FASTLED_SUPPORTED != 1
#undef FASTLED_SUPPORTED
#define FASTLED_SUPPORTED 1
#endif
#if POWER_METER_SUPPORTED != 1
#undef POWER_METER_SUPPORTED
#define POWER_METER_SUPPORTED 1
#endif
#if ENTER_BOOTLOADER_REMOTELY != 1
#undef ENTER_BOOTLOADER_REMOTELY
#define ENTER_BOOTLOADER_REMOTELY 1
#endif

#define EEMEM
#define PROGMEM
#define PSTR(arg) (arg)
#define printf_P printf
#define fprintf_P fprintf
#define strcmp_P strcmp
#endif


#define FREQ_PWM        (25000)    // PWM frequency for fans. Equals number of timer1 overflow interrups/second.
#define MAX_SNS         (12)       // Total number of sensors supported.
#define MAX_TRIP_POINTS (5)        // Total number of trip points supported.
#define MAX_FANS        (6)        // Total number of fans supported.
#define MAX_FAN_OUTS    (2)        // Total number of fan outputs supported.
#define MAX_POWERMETER  (3)        // Total number of power meters supported.
#define MAX_LED_STRIPES (3)        // Total number of led stripes supported.

#define PWM_DUTY_0           (0)     // Value representing 0% duty cycle
#define PWM_DUTY_100         (255)   // Value representing 100% duty cycle

#if ENTER_BOOTLOADER_REMOTELY
#define BOOTLOADER_MAGIC         (0x73)   // Magic number in eeprom to trigger bootloader; don't choose 0x00 or 0xff or other trivial values.
#define BOOTLOADER_ENTER_NUM_REQ (3)      // Number of successive calls to CUSTOM_RQ_ENTER_BOOTLOAD before bootloader is actually eneterd.
#endif

// Pin configurations etc.
#ifdef __AVR__

#if defined(__AVR_ATmega88__) || defined (__AVR_ATmega88P__) || defined(__AVR_ATmega168__) || defined (__AVR_ATmega168P__)
#define EEPROM_SIZE (512)
#elif defined(__AVR_ATmega32U4__)
#define EEPROM_SIZE (1024)
#else
#error Define EEPROM_SIZE!
#endif


#define USE_BITFIELDS       0       // Using bitfields for data storage reduces RAM/Eeprom usage, but increases code size.


#if FAN_PWM_MAX31790_SUPPORTED == 0
#define FAN_PWM_INTERNAL_SUPPORTED
#endif

#if !defined(MAX_FAN_OUTS) || (defined(FAN_OUT_SUPPORTED) && MAX_FAN_OUTS < 1)
#error MAX_FAN_OUTS must be >= 1!
#endif


#ifdef FAN_PWM_INTERNAL_SUPPORTED

/*
   Pin configuration:

   PB0     TACHO3
   PB1     PWM3
   PB2     PWM4
   PB3     PWM5 / Bootload
   PB4     TACHO4
   PB5     TACHO5

   PC0     DUTY_AN_IN (input fan duty as analog)
   PC1     SENSE_IN (input fan generated tacho signal)
   PC2     TEMP_AN0
   PC3     TEMP_AN1
   PC4     SDA
   PC5     SCL

   PD0     TACHO0
   PD1     TACHO1
   PD2     USB D+
   PD3     PWM0
   PD4     USB D-
   PD5     PWM1
   PD6     PWM2
   PD7     TACHO2
*/
#define TCCR_PWM_FAN0    (TCCR2A)
#define TCCR_PWM_FAN1    (TCCR0A)
#define TCCR_PWM_FAN2    (TCCR0A)
#define TCCR_PWM_FAN3    (TCCR1A)
#define TCCR_PWM_FAN4    (TCCR1A)
#define TCCR_PWM_FAN5    (TCCR2A)
#define COM_PWM_FAN0     (COM2B1)
#define COM_PWM_FAN1     (COM0B1)
#define COM_PWM_FAN2     (COM0A1)
#define COM_PWM_FAN3     (COM1A1)
#define COM_PWM_FAN4     (COM1B1)
#define COM_PWM_FAN5     (COM2A1)
#define DDR_PWM_FAN0     (DDRD)
#define DDR_PWM_FAN1     (DDRD)
#define DDR_PWM_FAN2     (DDRD)
#define DDR_PWM_FAN3     (DDRB)
#define DDR_PWM_FAN4     (DDRB)
#define DDR_PWM_FAN5     (DDRB)
#define PIN_PWM_FAN0     (PD3)
#define PIN_PWM_FAN1     (PD5)
#define PIN_PWM_FAN2     (PD6)
#define PIN_PWM_FAN3     (PB1)
#define PIN_PWM_FAN4     (PB2)
#define PIN_PWM_FAN5     (PB3)
#define PORT_PWM_FAN0    (PORTD)
#define PORT_PWM_FAN1    (PORTD)
#define PORT_PWM_FAN2    (PORTD)
#define PORT_PWM_FAN3    (PORTB)
#define PORT_PWM_FAN4    (PORTB)
#define PORT_PWM_FAN5    (PORTB)

// Also change PCINT0_vect code when changing TACHO pins!
#define PIN_TACHO_FAN0   (PD0)
#define PIN_TACHO_FAN1   (PD1)
#define PIN_TACHO_FAN2   (PD7)
#define PIN_TACHO_FAN3   (PB0)
#define PIN_TACHO_FAN4   (PB4)
#define PIN_TACHO_FAN5   (PB5)
#define DDR_TACHO_FAN0   (DDRD)
#define DDR_TACHO_FAN1   (DDRD)
#define DDR_TACHO_FAN2   (DDRD)
#define DDR_TACHO_FAN3   (DDRB)
#define DDR_TACHO_FAN4   (DDRB)
#define DDR_TACHO_FAN5   (DDRB)
#define PORT_TACHO_FAN0  (PORTD)
#define PORT_TACHO_FAN1  (PORTD)
#define PORT_TACHO_FAN2  (PORTD)
#define PORT_TACHO_FAN3  (PORTB)
#define PORT_TACHO_FAN4  (PORTB)
#define PORT_TACHO_FAN5  (PORTB)

#endif

#define PIN_TACHO_OUT_0    (PC6)
#define DDR_TACHO_OUT_0    (DDRC)
#define PORT_TACHO_OUT_0   (PORTC)

#define PIN_TACHO_OUT_1    (PC7)
#define DDR_TACHO_OUT_1    (DDRC)
#define PORT_TACHO_OUT_1   (PORTC)

// -- Timer1 (16bit), OC1A/OC1B -- Fan3/Fan4 -------------------------------
// FREQ_PWM = F_CPU / (2*PRESCALE*top)
// top = F_CPU / (2*PRESCALE*FREQ_PWM)
// F_CPU = 16MHz, FREQ_PWM=25kHz, PRESCALE={1, 8, 64, 256, 1024}
// top = 16000000/(2*8*25000) = 40
#define TMR1_PRESCALE      (8)
#define TMR1_PRESCALE_BITS (2)       // bitwise value in TCCR1B register

// -- Timer0 (8bit), OC0A/OC0B -- Fan2/Fan1 --------------------------------
// FREQ_PWM = F_CLK_IO / (PRESCALE*510)   for phase correct PWM
// FREQ_PWM = F_CLK_IO / (PRESCALE*256)   for fast PWM
// F_CLK_IO = F_CPU = 16MHz, PRESCALE={1, 8, 64, 256, 1024}
// when choosing fast PWM, PRESCALE=1, FREQ_PWM = 62.5kHz ==>  too high
// when choosing fast PWM, PRESCALE=8, FREQ_PWM = 7.8kHz  ==>  too low
// The trick: select PRESCALE=8, but have the exact 25kHz interrupt from
// timer 1 reset the timer0 counter every time to also have it run at 25kHz exactly.
// At 7.8kHz a single step in OCR0x increases the high-time by
// (1s/7.8kHz)/256 = 0.5us. At 25kHz a PWM cycle takes 40us, so the maximum
// value for OCR0x (duty 100%) is 40/0.5=80
// MAX_OCR0x = 1/(FREQ_PWM) / ((1/(F_CPU/(TMR0_PRESCALE*256)))/256) = F_CPU / TMR0_PRESCALE / FREQ_PWM
#define TMR0_PRESCALE      (8)
#define TMR0_PRESCALE_BITS (_BV(CS01))       // bitwise value in TCCR0B register
#define MAX_OCR0x          (F_CPU/TMR0_PRESCALE/FREQ_PWM)

#if (FREQ_PWM < (F_CPU/TMR0_PRESCALE/256))
#error Requested PWM frequency cannot be achieved for timer 0 using the selected prescaler value!
#endif

// -- Timer2 (8bit), OC2A/OC2B -- Fan5/Fan0---------------------------------
// FREQ_PWM = F_CLK_IO / (PRESCALE*510)   for phase correct PWM
// FREQ_PWM = F_CLK_IO / (PRESCALE*256)   for fast PWM
// F_CLK_IO = F_CPU = 16MHz, PRESCALE={1, 8, 32, 64, 128, 256, 1024}
// when choosing fast PWM, PRESCALE=1, FREQ_PWM = 62.5kHz ==>  too high
// when choosing fast PWM, PRESCALE=8, FREQ_PWM = 7.8kHz  ==>  too low
// The trick: select PRESCALE=8, but have the exact 25kHz interrupt from
// timer 1 reset the timer0 counter every time to also have it run at 25kHz exactly.
// At 7.8kHz a single step in OCR2x increases the high-time by
// (1s/7.8kHz)/256 = 0.5us. At 25kHz a PWM cycle takes 40us, so the maximum
// value for OCR2x (duty 100%) is 40/0.5=80
#define TMR2_PRESCALE      (8)
#define TMR2_PRESCALE_BITS (_BV(CS21))       // bitwise value in TCCR2B register
#define MAX_OCR2x          (F_CPU/TMR2_PRESCALE/FREQ_PWM)

#if (FREQ_PWM < (F_CPU/TMR2_PRESCALE/256))
#error Requested PWM frequency cannot be achieved for timer 2 using the selected prescaler value!
#endif

// TOP values for PWM generation
#define TOP_PWM_FAN0 ((uint16_t)MAX_OCR2x)
#define TOP_PWM_FAN1 ((uint16_t)MAX_OCR0x)
#define TOP_PWM_FAN2 ((uint16_t)MAX_OCR0x)
#define TOP_PWM_FAN3 (ICR1)
#define TOP_PWM_FAN4 (ICR1)
#define TOP_PWM_FAN5 ((uint16_t)MAX_OCR2x)

// OCR registers storing dutycycle for PWM generation
#define OCR_PWM_FAN0 (OCR2B)
#define OCR_PWM_FAN1 (OCR0B)
#define OCR_PWM_FAN2 (OCR0A)
#define OCR_PWM_FAN3 (OCR1A)
#define OCR_PWM_FAN4 (OCR1B)
#define OCR_PWM_FAN5 (OCR2A)


#if POWER_METER_SUPPORTED
#define POWER_METER_INA219 0
#define POWER_METER_INA260 1
#endif

#if SNS_I2C_SUPPORTED
#define SNS_I2C_MCP980X_BASE_ADDR (0x48)    // Base address for MCP980x temp sensors.
#endif

#if SNS_ANALOG_SUPPORTED || FAN_OUT_SUPPORTED
#define ADC_PRESCALER      (64)
#define ADC_PRESCALER_BITS (_BV(ADPS2) | _BV(ADPS1))
#endif

#endif // __AVR__

#endif // __CONFIG_H_INCLUDED__
