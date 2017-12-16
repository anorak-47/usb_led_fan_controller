/*
    USB Fan Controller - Flexible PWM Fan Controller firmware

    Copyright (C) 2012  Ivo Pullens (info@emmission.nl)
    Copyright (C) 2008  Objective Development Software GmbH

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

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  // for sei()
#include <util/delay.h>     // for _delay_ms, _delay_us
#include <avr/pgmspace.h>   // required by usbdrv.h
#include <avr/eeprom.h>
#include <util/twi.h>
#include <string.h>         // memset

#include "config.h"
#include "usbdrv.h"
#include "types.h"
#include "requests.h"


#define PWM_DUTY_0           (0)     // Value representing 0% duty cycle
#define PWM_DUTY_100         (255)   // Value representing 100% duty cycle

// Some macro's to make life easier....
#define ENABLE_PWM(tccr,com)        (tccr |= _BV(com))
#define DISABLE_PWM(tccr,com)       (tccr &= ~_BV(com))
#define MAKE_OUTPUT(ddr,pin)        (ddr |= _BV(pin))
#define MAKE_INPUT(ddr,pin)         (ddr &= ~_BV(pin))
#define SET_OUTPUT(port,pin)        (port |= _BV(pin))
#define CLR_OUTPUT(port,pin)        (port &= ~_BV(pin))
#define ENABLE_PULLUP(port,pin)     (port |= _BV(pin))
#define IS_OUTPUT(ddr,pin)          ((ddr &= _BV(pin)) != 0)

// Count number of timer1 overflow interrupts to allow accurate timing without
// using a timer. Increments by FREQ_PWM each second.
static uint16_t timer1_ovf_counter = 0;

#if (USB_COMM_WATCHDOG_TIMEOUT > 0)
static uint8_t usb_comm_watchdog_count = 0;
#endif

static uint8_t status = 0;

// Definition of struct to hold all data (configuration, status etc.) for a single fan.
typedef struct
{
  struct {
#if USE_BITFIELDS
    uint8_t  snsIdx:  3;       // index of sensor controlling fan speed. The number of bits must be able to hold MAX_SNS values.
    FANTYPE  fanType: 2;       // type of fan connected (if any). The number of bits must be able to hold all FANTYPE values!
    FANMODE  fanMode: 2;       // mode of fan connected. The number of bits must be able to hold all FANMODE values!
#else
    uint8_t  snsIdx;
    FANTYPE  fanType;
    FANMODE  fanMode;
#endif
  } config;
  struct {
#if USE_BITFIELDS
    uint8_t  stalled: 1;       // when set, fan is regarded stalled (rps < min_rps)
#else
    uint8_t  stalled;
#endif
  } status;
  uint8_t   duty;              // duty cycle of fan; calculated in updateCtrl. [0..255] = [0%..100%]
  uint8_t   pinrise_count;     // counts number of pinrises/second.
  uint8_t   inc_per_pinrise;   // frational increments per low/high transition in pinchange interrupt. Most fans generate 2 pulses/rev, resulting in 2 rises. 256/2=128.
  uint16_t  rps;               // current revolutions/sec; updated once per second. High byte = full revolutions, low byte = fraction.
  uint8_t   min_rps;           // minimum number of revolutions, otherwise the fan will be seen as stalled.
  // -- Parameters for fan in mode fixed dutycycle.
  uint8_t   dutyFixed;
#if FAN_MODE_LINEAR_SUPPORTED
  // -- Parameters for fan in mode linear.
  int16_t   dutyOffs;
  int16_t   dutyGain;          // lower 6(!) bits = fraction.
#endif
#if FAN_MODE_PI_SUPPORTED || FAN_MODE_FUZZY_SUPPORTED
  // -- Parameters for fan in mode using a setpoint based on sensor data
  struct {
    int8_t    delta;           // setpoint delta value for sensor (will be added to reference sensor's value) (when under sensor control)
#if USE_BITFIELDS
    uint8_t   snsIdx:    3;    // reference sensor index for setpoint value; the setpoint will be added to this sensor's value to get the actual setpoint
                               // (when under sensor control and reference sensor != SNSTYPE_NONE). The number of bits must be able to hold MAX_SNS values.
#else
    uint8_t   snsIdx;
#endif
  } snsSetp;
#endif
#if FAN_MODE_PI_SUPPORTED
  // -- Parameters for fan in mode PI control.
  int16_t   i;                 // integral portion of PI controller
  int8_t    Kp;
  int8_t    Ki;
  int8_t    Kt;
  int16_t   es;
#endif
  // --
  uint8_t   dutyMin;
  uint8_t   dutyMax;
} Fan;

#if CTRL_DEBUG
typedef struct
{
  int8_t    e;
  int16_t   p;
} Fan_dbg;
static volatile Fan_dbg fans_dbg[MAX_FANS];
#endif


#if ENTER_BOOTLOADER_REMOTELY
unsigned char EEMEM eeprom_bootload_trigger = 0;
uint8_t eeprom_bootload_counter = 0;
#endif

static volatile Fan fans[MAX_FANS];
Fan EEMEM fans_eeprom[MAX_FANS];    // first entry in eeprom; start of crc calculation.

#if FAN_OUT_SUPPORTED
typedef struct
{
  FANOUTMODE mode;           // mode, e.g. clone fan with minimum rps, maximum rps or a specific fan.
  uint8_t    fanStallDetect; // bitmask defining which fans to include in the stall detection.
                             // An rps of 0 is output when any of the selected fans is stalled.
                             // Should contain at least MAX_FANS bits!
  uint8_t    rps;            // current rps, as output on connector.
} Fan_out;

static volatile Fan_out fan_out;
Fan_out EEMEM fan_out_eeprom;
#endif

// Definition of struct to hold all data for a single sensor.
typedef struct
{
  struct {
#if USE_BITFIELDS
    uint8_t valid:      1;      // current temperature reading is valid
#else
    uint8_t valid;
#endif
  } status;
  uint8_t   type;               // temperature sensor type, e.g SnsType_Temp_NTC1.
  int8_t    value;              // current value (e.g. temperature in C) of sensor.
} Sensor;

static volatile Sensor sns[MAX_SNS];
Sensor EEMEM sns_eeprom[MAX_SNS];

#define EEPROM_CHK_MAGIC (0xA5) // magic number to 'assure' empty/garbage eeprom sections can be detected
uint8_t EEMEM chk_eeprom;       // assure this variable follows the eeprom section which must be checksum-checked!

#if SNS_ANALOG_SUPPORTED
// Conversion table for 10K NTC
const int8_t NTC_Conv_Table[] = {100,85,75,67,61,56,51,47,43,40,37,34,31,28,26,23,20,18,15};
#define NTC_CONV_TABLE_FIRST_ADC_VALUE (56)
#define NTC_CONV_TABLE_STEP            (32)
#define NTC_CONV_TABLE_STEP_SHIFT      (5)
#define NTC_CONV_TABLE_SIZE            (sizeof(NTC_Conv_Table)/sizeof(NTC_Conv_Table[0]))
#endif

#if FAN_OUT_SUPPORTED
static uint16_t rps_out_tmr1_ovf_interval;
#endif

#if FAN_MODE_PI_SUPPORTED
static void restartCtrl( const uint8_t channel );
#else
#define restartCtrl(channel)
#endif
static void loadSettings( const uint8_t clear );
static void saveSettings();

inline static void initPwm()
{
  // in updatePwmOut() the Pwm generation on the output pins is enabled.

  // -- Timer1 (16bit), OC1A/OC1B -- Fan3/Fan4 -------------------------------
  TCCR1A =   _BV(WGM11); // mode 10 (phase correct PWM)
  TCCR1B =   _BV(WGM13)
           | TMR1_PRESCALE_BITS;   // prescale: clkIO/8 (From prescaler)

  ICR1 = F_CPU/2/TMR1_PRESCALE/FREQ_PWM; // top, don't write F_CPU/(2*TMR1_PRESCALE*FREQ_PWM) as this fails in macro expansion.

  TIMSK1 |= _BV(TOIE1); // enable timer1 Overflow Interrupt.

  // -- Timer0 (8bit), OC0A/OC0B -- Fan2/Fan1 --------------------------------
  TCCR0A =   _BV(WGM01)
           | _BV(WGM00);  // fast pwm mode 3 (top=0xff)
  TCCR0B = TMR0_PRESCALE_BITS;

  // -- Timer2 (8bit), OC2A/OC2B -- Fan5/Fan0---------------------------------
  TCCR2A =   _BV(WGM01)
           | _BV(WGM00);  // fast pwm mode 3 (top=0xff)
  TCCR2B = TMR2_PRESCALE_BITS;

  // configure all PWM ports as outputs
  MAKE_OUTPUT( DDR_PWM_FAN0, PIN_PWM_FAN0 );
  MAKE_OUTPUT( DDR_PWM_FAN1, PIN_PWM_FAN1 );
  MAKE_OUTPUT( DDR_PWM_FAN2, PIN_PWM_FAN2 );
  MAKE_OUTPUT( DDR_PWM_FAN3, PIN_PWM_FAN3 );
  MAKE_OUTPUT( DDR_PWM_FAN4, PIN_PWM_FAN4 );
  MAKE_OUTPUT( DDR_PWM_FAN5, PIN_PWM_FAN5 );
}

#if SNS_I2C_SUPPORTED
static uint8_t Sns_Temp_I2C_Present = 0;  // bits 0..7 represent which i2c sensor SnsType_Temp_I2C_Addr0..SnsType_Temp_I2C_Addr7 is detected.

// Wait for I2C (TWI) interface to become ready, or timeout.
// Returns 1 on ready, 0 on timeout.
static uint8_t waitI2CReady()
{
  uint16_t timeout_us = 10000;
  while (bit_is_clear(TWCR, TWINT))
  {
    if (timeout_us == 0) return 0;
    timeout_us--;
    _delay_us(1);
  };
  return 1;
}

// Read temperature via I2C. Returns -127 on error.
static int8_t readI2CMcp980x(uint8_t addr)
{
  int8_t temp = -127;
  addr = (SNS_I2C_MCP980X_BASE_ADDR | (addr & 7)) << 1;

  waitI2CReady();
  TWCR  = _BV(TWINT) | _BV(TWEN) | _BV(TWSTA);    // Send START condition
  waitI2CReady();
  TWDR = addr | 0;                                // Send slave address & write operation
  TWCR = _BV(TWINT) | _BV(TWEN);                  // Clear TWINT bit in TWCR to start transmission of address
  if (!waitI2CReady()) return temp;
  if ((TWSR & TW_STATUS_MASK) != TW_MT_SLA_ACK) return temp;   // Return error when slave did not ack.
  TWDR = 0x00;                                    // Write byte to slave - temp. register
  TWCR = _BV(TWINT) | _BV(TWEN);                  // Clear TWINT bit in TWCR to start transmission of data
  waitI2CReady();
  TWCR  = _BV(TWINT) | _BV(TWEN) | _BV(TWSTA);    // Send (rep.) START condition
  waitI2CReady();
  TWDR = addr | 1;                                // Send slave address & read operation
  TWCR = _BV(TWINT) | _BV(TWEN);                  // Clear TWINT bit in TWCR to start transmission of address
  waitI2CReady();
  TWCR = _BV(TWINT) | _BV(TWEN) /* | _BV(TWEA)*/;      // Clear TWINT bit in TWCR to start transmission of data
  waitI2CReady();
  temp = TWDR;
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);     // Transmit STOP condition
  return temp;
}

static inline void initI2C()
{
  uint8_t i;
// F_SCL = F_CPU / (16+2*TWBR*TWI_PRESCALE)
#define  TWI_PRESCALE      (1)
#define  TWI_PRESCALE_BITS (0)

  TWSR = TWI_PRESCALE_BITS;
  TWBR = (F_CPU / FREQ_I2C_SCL - 16) / 2;

  // Determine which slaves are really present on the bus.
  // Try to read each one of them and the ones that respond
  // are marked as present.
  for (i = 0; i <= 7; i++)
  {
    if (readI2CMcp980x(i) > -127)
      Sns_Temp_I2C_Present |= 1<<i;
  }
}
#endif // SNS_I2C_SUPPORTED


#if EEPROM_UPDOWNLOAD
static uint16_t usbFunctionBytesRemain;
#endif

// USB request handler
usbMsgLen_t usbFunctionSetup(uchar data[8])
{
  int i;
  usbRequest_t    *rq = (void *)data;
  static uchar    dataBuffer[8];  // buffer must stay valid when usbFunctionSetup returns
  usbMsgPtr = dataBuffer;         // tell the driver which data to return

  // Most calls use rq->wIndex.bytes[0] as fan/sensor index; store in i.
  i = rq->wIndex.bytes[0];

#if (USB_COMM_WATCHDOG_TIMEOUT > 0)
  // Reset the usb communication watchdog timer.
  usb_comm_watchdog_count = 0;
#endif

#if ENTER_BOOTLOADER_REMOTELY
  if (CUSTOM_RQ_ENTER_BOOTLOAD == rq->bRequest)
  {
    // Bootloader will only be entered when this call is made a successive number of times.
    eeprom_bootload_counter++;
    if (eeprom_bootload_counter > BOOTLOADER_ENTER_NUM_REQ)
    {
      eeprom_write_byte(&eeprom_bootload_trigger, BOOTLOADER_MAGIC);
      // Endless loop; triggering watchdog stops to force reset and enter bootloader.
      for (;;) {};
    }
  }
  else
  {
    // Non CUSTOM_RQ_ENTER_BOOTLOAD call: reset bootload call counter.
    eeprom_bootload_counter = 0;
  }
#endif


  switch (rq->bRequest)
  {
    case CUSTOM_RQ_ECHO:
      // echo -- used for communication reliability tests
      // return ones complement of input
      dataBuffer[0] = ~rq->wValue.bytes[0];
      dataBuffer[1] = ~rq->wValue.bytes[1];
      dataBuffer[2] = ~rq->wIndex.bytes[0];
      dataBuffer[3] = ~rq->wIndex.bytes[1];
      return 4;
    case CUSTOM_RQ_FUNCS_SUPPORTED:
      *((uint16_t*)dataBuffer) = 0
#if SNS_ANALOG_SUPPORTED
                    | SUPPORTED_SNS_ANALOG
#endif
#if SNS_I2C_SUPPORTED
                    | SUPPORTED_SNS_I2C
#endif
#if FAN_OUT_SUPPORTED
                    | SUPPORTED_FAN_OUT
#endif
#if CTRL_DEBUG
                    | SUPPORTED_CTRL_DEBUG
#endif
#if (SUPPORTED_USBCOMM_WATCHDOG > 0)
                    | SUPPORTED_USBCOMM_WATCHDOG
#endif
#if FAN_MODE_LINEAR_SUPPORTED
                    | SUPPORTED_FANMODE_LINEAR
#endif
#if FAN_MODE_PI_SUPPORTED
                    | SUPPORTED_FANMODE_PI
#endif
#if FAN_MODE_FUZZY_SUPPORTED
                    | SUPPORTED_FANMODE_FUZZY
#endif
#if EEPROM_UPDOWNLOAD
                    | SUPPORTED_EEPROM_UPDOWNLD
#endif
      ;
      return 2;
    case CUSTOM_RQ_STATUS_READ:
      *dataBuffer = status;
      return 1;
    case CUSTOM_RQ_FANRPS_READ:
      // rq->wIndex.bytes[0]   Channel
      *((uint16_t*)dataBuffer) = fans[i].rps;
      return 2;
    case CUSTOM_RQ_FANDUTY_READ:
      // rq->wIndex.bytes[0]   Channel
      *dataBuffer = fans[i].duty;
      return 1;
    case CUSTOM_RQ_FANTYPE_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.bytes[0]   Type
      fans[i].config.fanType = rq->wValue.bytes[0];
      break;
    case CUSTOM_RQ_FANTYPE_READ:
      // rq->wIndex.bytes[0]   Channel
      *dataBuffer = fans[i].config.fanType;
      return 1;
    case CUSTOM_RQ_FANMODE_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.bytes[0]   Mode
      fans[i].config.fanMode = rq->wValue.bytes[0];
      break;
    case CUSTOM_RQ_FANMODE_READ:
      // rq->wIndex.bytes[0]   Channel
      *dataBuffer = fans[i].config.fanMode;
      return 1;
#if FAN_MODE_PI_SUPPORTED || FAN_MODE_FUZZY_SUPPORTED
    case CUSTOM_RQ_FANSETP_DELTA_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.bytes[0]   Setpoint delta
      fans[i].snsSetp.delta = rq->wValue.bytes[0];
      // Restart control loop
      restartCtrl(i);
      break;
    case CUSTOM_RQ_FANSETP_DELTA_READ:
      // rq->wIndex.bytes[0]   Channel
      *dataBuffer = fans[i].snsSetp.delta;
      return 1;
    case CUSTOM_RQ_FANSETP_READ:
      // rq->wIndex.bytes[0]   Channel
      *dataBuffer = sns[fans[i].snsSetp.snsIdx].value + fans[i].snsSetp.delta;
      return 1;
    case CUSTOM_RQ_FANSETP_REFSNS_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.bytes[0]   Reference sensor index
      fans[i].snsSetp.snsIdx = rq->wValue.bytes[0];
      // Restart control loop
      restartCtrl(i);
      break;
    case CUSTOM_RQ_FANSETP_REFSNS_READ:
      // rq->wIndex.bytes[0]   Channel
      *dataBuffer = fans[i].snsSetp.snsIdx;
      return 1;
#endif
#if FAN_MODE_PI_SUPPORTED
    case CUSTOM_RQ_FANKP_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.word       Kp
      fans[i].Kp = rq->wValue.word;
      break;
    case CUSTOM_RQ_FANKP_READ:
      *((uint16_t*)dataBuffer) = fans[i].Kp;
      return 2;
    case CUSTOM_RQ_FANKI_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.word       Ki
      fans[i].Ki = rq->wValue.word;
      break;
    case CUSTOM_RQ_FANKI_READ:
      *((uint16_t*)dataBuffer) = fans[i].Ki;
      return 2;
    case CUSTOM_RQ_FANKT_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.word       Kt
      fans[i].Kt = rq->wValue.word;
      break;
    case CUSTOM_RQ_FANKT_READ:
      *((uint16_t*)dataBuffer) = fans[i].Kt;
      return 2;
#endif
    case CUSTOM_RQ_FANDUTYFIXED_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.bytes[0]   Dutycycle. [0%..100%] = [0..255]
      fans[i].dutyFixed = rq->wValue.bytes[0];
      break;
    case CUSTOM_RQ_FANDUTYFIXED_READ:
      // rq->wIndex.bytes[0]   Channel
      *dataBuffer = fans[i].dutyFixed;
      return 1;
#if FAN_MODE_LINEAR_SUPPORTED
    case CUSTOM_RQ_FANGAIN_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.word       Gain, lower 6 bits contain fraction
      fans[i].dutyGain = rq->wValue.word;
      break;
    case CUSTOM_RQ_FANGAIN_READ:
      *((int16_t*)dataBuffer) = fans[i].dutyGain;
      return 2;
    case CUSTOM_RQ_FANOFFS_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.word       Offset
      fans[i].dutyOffs = rq->wValue.word;
      break;
    case CUSTOM_RQ_FANOFFS_READ:
      *((int16_t*)dataBuffer) = fans[i].dutyOffs;
      return 2;
#endif
    case CUSTOM_RQ_FANSNS_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.bytes[0]   Sensor index
      fans[i].config.snsIdx = rq->wValue.bytes[0];  // sensor index to use
      break;
    case CUSTOM_RQ_FANSNS_READ:
      // rq->wIndex.bytes[0]   Channel
      *dataBuffer = fans[i].config.snsIdx;
      return 1;
    case CUSTOM_RQ_FANMINRPS_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.bytes[0]   Minimum rps before fan is seen as stalled.
      fans[i].min_rps = rq->wValue.bytes[0];
      break;
    case CUSTOM_RQ_FANMINRPS_READ:
      // rq->wIndex.bytes[0]   Channel
      *dataBuffer = fans[i].min_rps;
      return 1;
    case CUSTOM_RQ_FANSTALL_READ:
      // rq->wIndex.bytes[0]   Channel
      *dataBuffer = fans[i].status.stalled;
      return 1;
    case CUSTOM_RQ_SNSTYPE_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.bytes[0]   Type
      sns[i].type = rq->wValue.bytes[0];  // see SnsType for values
      break;
    case CUSTOM_RQ_SNSTYPE_READ:
      // rq->wIndex.bytes[0]   Channel
      *dataBuffer = sns[i].type;
      return 1;
    case CUSTOM_RQ_SNS_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.bytes[0]   Value
      sns[i].value = rq->wValue.bytes[0];
      break;
    case CUSTOM_RQ_SNS_READ:
      // rq->wIndex.bytes[0]   Channel
      *dataBuffer = sns[i].value;
      return 1;
#if FAN_OUT_SUPPORTED
    case CUSTOM_RQ_FANSTALLDETECT_WRITE:
      // rq->wIndex.bytes[0]   Channel
      // rq->wValue.bytes[0]   When != 0, the fan rps will be included in the stall detection for the output fan
      if (rq->wValue.bytes[0])
        fan_out.fanStallDetect |= 1<<i;
      else
        fan_out.fanStallDetect &= ~(1<<i);
      break;
    case CUSTOM_RQ_FANSTALLDETECT_READ:
      // rq->wIndex.bytes[0]   Channel
      *dataBuffer = fan_out.fanStallDetect & (1<<i);
      return 1;
    case CUSTOM_RQ_FANOUTMODE_WRITE:
      // rq->wValue.bytes[0]   Mode, see FANOUTMODE
      fan_out.mode = rq->wValue.bytes[0];
      break;
    case CUSTOM_RQ_FANOUTMODE_READ:
      *dataBuffer = fan_out.mode;
      return 1;
    case CUSTOM_RQ_FANOUTRPS_READ:
      *dataBuffer = fan_out.rps;
      return 1;
#endif
    case CUSTOM_RQ_EEPROM_READ:
      // Load settings from eeprom, or revert to defaults when eeprom is invalid.
      loadSettings(0);
      break;
    case CUSTOM_RQ_EEPROM_WRITE:
      // Save settings to eeprom.
      saveSettings();
      break;
    case CUSTOM_RQ_LOAD_DEFAULTS:
      // Load default settings.
      loadSettings(1);
      break;
#if EEPROM_UPDOWNLOAD
    case CUSTOM_RQ_EEPROM_DOWNLOAD:
      usbFunctionBytesRemain = rq->wLength.word;
      return USB_NO_MSG;        // tell driver to use usbFunctionRead()
    case CUSTOM_RQ_EEPROM_UPLOAD:
      usbFunctionBytesRemain = rq->wLength.word;
      return USB_NO_MSG;        // tell driver to use usbFunctionWrite()
#endif
#if CTRL_DEBUG
    case CUSTOM_RQ_FANE_READ:
      *dataBuffer = fans_dbg[i].e;
      return 1;
    case CUSTOM_RQ_FANP_READ:
      *((uint16_t*)dataBuffer) = fans_dbg[i].p;
      return 2;
    case CUSTOM_RQ_FANI_READ:
      *((uint16_t*)dataBuffer) = fans[i].i;
      return 2;
#endif
  }
  return 0;   // default for not implemented requests: return no data back to host
}

#if USB_CFG_IMPLEMENT_FN_READ
// Deferred handling of USB read request.
uchar usbFunctionRead(uchar *data, uchar len)
{
  // Request was: CUSTOM_RQ_EEPROM_DOWNLOAD
  if(len > usbFunctionBytesRemain)                // len is max chunk size
    len = usbFunctionBytesRemain;                 // send an incomplete chunk
  usbFunctionBytesRemain -= len;
  eeprom_read_block((void*)data,(const void*)(EEPROM_SIZE-usbFunctionBytesRemain),len);
  return len;                                     // return real chunk size
}
#endif

#if USB_CFG_IMPLEMENT_FN_WRITE
// Deferred handling of USB write request.
uchar usbFunctionWrite(uchar *data, uchar len)
{
  // Request was: CUSTOM_RQ_EEPROM_UPLOAD
  if(len > usbFunctionBytesRemain)                // if this is the last incomplete chunk
    len = usbFunctionBytesRemain;                 // limit to the amount we can store
  usbFunctionBytesRemain -= len;
  eeprom_write_block((const void*)data,(void*)(EEPROM_SIZE-usbFunctionBytesRemain),len);
  if (usbFunctionBytesRemain == 0)
  {
    loadSettings(0);      // activate new eeprom contents
    return 1;             // return 1 to indicate we got all data
  }
  return 0;               // more to go...
}
#endif

/* ------------------------------------------------------------------------- */


// pin change interrupt for PCINT7..0
// pin change interrupt for PCINT23..16 is aliased use to this interrupt handler
// define as NONBLOCK to allow USB INT0 to interrupt this code.
ISR(PCINT0_vect, ISR_NOBLOCK)
{
  static uint8_t prev_PINB = 0;
  static uint8_t prev_PIND = 0;
  uint8_t changes;

  // Figure out which pin changed and had a low/high transition
  // and increment corresponding TACHO number of pinchanges
  changes = PIND ^ prev_PIND; // xor - bits changed since previous check will become 1
  if ((changes & _BV(PIN_TACHO_FAN0)) && (PIND & _BV(PIN_TACHO_FAN0)))  fans[0].pinrise_count++;  // PD0 TACHO0   PCINT16
  if ((changes & _BV(PIN_TACHO_FAN1)) && (PIND & _BV(PIN_TACHO_FAN1)))  fans[1].pinrise_count++;  // PD1 TACHO1   PCINT17
  if ((changes & _BV(PIN_TACHO_FAN2)) && (PIND & _BV(PIN_TACHO_FAN2)))  fans[2].pinrise_count++;  // PD7 TACHO2   PCINT23

  changes = PINB ^ prev_PINB; // xor - bits changed since previous check will become 1
  if ((changes & _BV(PIN_TACHO_FAN3)) && (PINB & _BV(PIN_TACHO_FAN3)))  fans[3].pinrise_count++;  // PB0 TACHO3   PCINT0
  if ((changes & _BV(PIN_TACHO_FAN4)) && (PINB & _BV(PIN_TACHO_FAN4)))  fans[4].pinrise_count++;  // PB4 TACHO4   PCINT4
  if ((changes & _BV(PIN_TACHO_FAN5)) && (PINB & _BV(PIN_TACHO_FAN5)))  fans[5].pinrise_count++;  // PB5 TACHO5   PCINT5

  prev_PINB = PINB;
  prev_PIND = PIND;
}

// pin change interrupt for PCINT23..16
ISR(PCINT2_vect, ISR_ALIASOF(PCINT0_vect));

// Timer1 overflow interrupt.
// Used to keep the 8bit PWM's in sync with this timer.
ISR(TIMER1_OVF_vect, ISR_NOBLOCK)
{
  TCNT0 = 0xff;
  TCNT2 = 0xff;
  timer1_ovf_counter++;
}


// Convert analog to digital, 10 bits accuracy.
// The result is right-adjusted, meaning the lower 10bits are used.
static uint16_t adConvert10bits(uint8_t channel)
{
  ADMUX =   _BV(REFS0)                  // Voltage reference: AVCC with external capacitor at AREF pin
          | channel;                    // Analog channel: MUX3..MUX0
  ADCSRA |= _BV(ADIF) | _BV( ADSC);     // clear hardware "conversion complete" flag & start conversion
  while( bit_is_set(ADCSRA, ADSC) );    // wait until conversion complete

  // read ADC (full 10 bits) (ADCL must be read first!)
  return ADCL | (ADCH<<8);
}

#if SNS_ANALOG_SUPPORTED
// Convert adc value to temperature, in degrees Celcius.
int8_t ntcAdcToTemp(uint16_t adc)
{
  uint8_t i;
  int8_t hi,lo;
  int16_t frac;

  // Clip to upper/lower table values
  if (adc < NTC_CONV_TABLE_FIRST_ADC_VALUE)
    return NTC_Conv_Table[0];
  if (adc >= NTC_CONV_TABLE_FIRST_ADC_VALUE+(NTC_CONV_TABLE_SIZE-1)*NTC_CONV_TABLE_STEP)
    return NTC_Conv_Table[NTC_CONV_TABLE_SIZE-1];

  adc -= NTC_CONV_TABLE_FIRST_ADC_VALUE;
  i = adc >> NTC_CONV_TABLE_STEP_SHIFT;   // calculate index in table
  frac = adc & (NTC_CONV_TABLE_STEP-1);   // calculate fraction
  lo = NTC_Conv_Table[i];
  hi = NTC_Conv_Table[i+1];
  return lo + ((frac*(hi-lo))>>NTC_CONV_TABLE_STEP_SHIFT);
}
#endif

// Update value for each sensor.
static inline void updateSns()
{
  uint8_t i;
  for (i = 0; i < MAX_SNS; i++)
  {
    sns[i].status.valid = 1;  // Temp value is valid by default.
    if ( sns[i].type == SNSTYPE_NONE )
    {
      sns[i].status.valid = 0;
    }
#if SNS_ANALOG_SUPPORTED
    else if (    sns[i].type >= SNSTYPE_TEMP_NTC0
              && sns[i].type <= SNSTYPE_TEMP_NTC1 )
    {
      uint8_t channel = sns[i].type - SNSTYPE_TEMP_NTC0 + 2;
      sns[i].value = ntcAdcToTemp( adConvert10bits( channel ));
    }
#endif
#if SNS_I2C_SUPPORTED
    else if (    sns[i].type >= SNSTYPE_TEMP_I2C_ADDR0
              && sns[i].type <= SNSTYPE_TEMP_I2C_ADDR7 )
    {
      uint8_t addr = sns[i].type - SNSTYPE_TEMP_I2C_ADDR0;
      // Only a sensor present at the bus can be read!
      if (Sns_Temp_I2C_Present & (1<<addr))
      {
        sns[i].value = readI2CMcp980x( addr );
      }
      else
      {
        sns[i].status.valid = 0;
      }
    }
#endif
    else if (    sns[i].type >= SNSTYPE_RPS_FAN0
              && sns[i].type <= SNSTYPE_RPS_FAN5 )
    {
      uint8_t channel = sns[i].type - SNSTYPE_RPS_FAN0;
      sns[i].value = fans[channel].rps >> 8;  // lower 8 bits represent the fraction, so take the full nr of revolutions.
    }
#if FAN_OUT_SUPPORTED
    else if ( sns[i].type == SNSTYPE_DUTY_IN )
    {
      uint16_t advalue = adConvert10bits( 0 );  // ADC0 receives the analog representation of the input duty cycle
      sns[i].value = (advalue*25)>>8;           // Convert value (1024 represents 100%, 0 represents 0%) to [0..100]% range.
                                                // multiply by 100, then divide by 1024 (won't fit in 16 bits) so first
                                                // multiply by 25, then divide by 256.
    }
#endif
    // Sensors SNSTYPE_EXT0..SNSTYPE_EXT3 can be written over USB.

    // If sensor value is invalid, set the value to 0.
    if (!sns[i].status.valid)
    {
      sns[i].value = 0;
    }
  }
}

#if FAN_OUT_SUPPORTED
static inline void updateFanOut()
{
  uint8_t i;
  uint8_t rps = 0;  // rps for fan which is used as input sensor (e.g. going to the motherboard to act on fan failures)
  uint8_t first = 1;

  // A fan will be flagged as stalled when:
  // * it has tacho feedback
  // * min_rps > 0
  // * current rps < min_rps
  if (    (fan_out.mode >= FANOUTMODE_RPS_CLONEFAN0)
       && (fan_out.mode <= FANOUTMODE_RPS_CLONEFAN5))
  {
    rps = fans[fan_out.mode-FANOUTMODE_RPS_CLONEFAN0].rps >> 8;
  }
  else if (fan_out.mode == FANOUTMODE_RPS_FIXED_50)
  {
    rps = 50;
  }

  // Figure out if any fan has stalled, and determine min/max
  // rps when requested.
  for (i = 0; i < MAX_FANS; i++)
  {
    if (fans[i].config.fanType >= MIN_FANTYPE_WITH_TACHO) // fan has speed feedback
    {
      uint8_t rps_fan = fans[i].rps >> 8;
      if (    (fan_out.fanStallDetect & (1<<i))    // fan i included in stall detection?
           && fans[i].status.stalled)              // fan i is stalled?
      {
        rps = 0;  // Fan stalled: output rps of 0 and break out of loop
        break;
      }
      else if (fan_out.mode == FANOUTMODE_RPS_MINFAN)
      {
        if (first || rps > rps_fan)
        {
          rps = rps_fan;
          first = 0;
        }
      }
      else if (fan_out.mode == FANOUTMODE_RPS_MAXFAN)
      {
        if (first || rps < rps_fan)
        {
          rps = rps_fan;
          first = 0;
        }
      }
    }
  }
  fan_out.rps = rps;

  // To simulate the fan rotation to e.g. the motherboard, the fan tacho signal
  // has to generate 2 pulses per revolution, and rps revolutions a second.
  // The interval, using timer1 overflow counter, between each toggle
  // is FREQ_PWM/(4*rps)
  rps_out_tmr1_ovf_interval = (FREQ_PWM>>2)/rps;
}
#endif

// Calculate the new dutycycle for given channel.
static inline void updateCtrl(const uint8_t i /* channel */)
{
  uint8_t snsIdx = fans[i].config.snsIdx;
  // Safety: If fan is under automatic control (mode != fixed),
  // then an invalid sensor readout will lead to 100% dutycycle
  // for the fan.
  if (    (fans[i].config.fanMode != FANMODE_FIXED_DUTY)
       && (!sns[snsIdx].status.valid))
  {
    fans[i].duty = fans[i].dutyMax;
    return;
  }

  // Determine new dutycycle for fan, depending on the fan's control mode.
  switch (fans[i].config.fanMode)
  {
    case FANMODE_FIXED_DUTY:
      fans[i].duty = fans[i].dutyFixed;
      break;
#if FAN_MODE_LINEAR_SUPPORTED
      case FANMODE_LINEAR:
      {
        int16_t v = ((fans[i].dutyGain * sns[snsIdx].value) >> 6) + fans[i].dutyOffs; // gain*value shift right 6 bits, as lower 6 bits in gain contain the fraction
        // Limit output to allowed range
        if (v < (int16_t)fans[i].dutyMin)
          v = (int16_t)fans[i].dutyMin;         // clip to lower duty cycle
        else if (v > (int16_t)fans[i].dutyMax)
          v = (int16_t)fans[i].dutyMax;         // clip to upper duty cycle
        fans[i].duty = v;
      }
      break;
#endif
#if FAN_MODE_PI_SUPPORTED
    case FANMODE_PI:
      {
        // Calculate new duty cycle.
        int8_t setp;
        int16_t v,u;
        // Calculate the setpoint.
        setp = sns[fans[i].snsSetp.snsIdx].value + fans[i].snsSetp.delta;
        // Calculate error e
        int8_t e = sns[snsIdx].value - setp;
#if CTRL_DEBUG
        fans_dbg[i].e = e;
#endif
        // Calculate P & I, store in v
        v = fans[i].Kp*e;                                    // P
#if CTRL_DEBUG
        fans_dbg[i].p = v;
#endif
        fans[i].i += fans[i].Ki*e + fans[i].Kt*fans[i].es;   // I, kt*es for anti-windup
        v += fans[i].i;                                      // v = P + I

        // Limit output to allowed range
        if (v < (int16_t)fans[i].dutyMin)
          u = (int16_t)fans[i].dutyMin;         // clip to lower duty cycle
        else if (v > (int16_t)fans[i].dutyMax)
          u = (int16_t)fans[i].dutyMax;         // clip to upper duty cycle
        else
          u = v;

        // when error becomes 0 and integrator is still != 0, slowly reduce integrator to 0
        // Otherwise calculate saturation amount of output wrt input.
        if (e == 0 && fans[i].i != 0)
          if (fans[i].i > 0)
            fans[i].es = -1;
          else
            fans[i].es = +1;
        else
          fans[i].es = u - v;

        // Store the calculated duty cycle. The next update (every second)
        // the new duty cycle will be set.
        fans[i].duty = u;
      }
      break;
#endif
#if FAN_MODE_FUZZY_SUPPORTED
    case FANMODE_FUZZY:
      {
        // TODO
      }
      break;
#endif
  }
}

static inline void updatePwmOut()
{
  uint8_t i;

  // Set duty cycle for PWM outputs.
  OCR_PWM_FAN0 = (fans[0].duty * TOP_PWM_FAN0) >> 8;  // PWM0
  OCR_PWM_FAN1 = (fans[1].duty * TOP_PWM_FAN1) >> 8;  // PWM1
  OCR_PWM_FAN2 = (fans[2].duty * TOP_PWM_FAN2) >> 8;  // PWM2
  OCR_PWM_FAN3 = (fans[3].duty * TOP_PWM_FAN3) >> 8;  // PWM3
  OCR_PWM_FAN4 = (fans[4].duty * TOP_PWM_FAN4) >> 8;  // PWM4
  OCR_PWM_FAN5 = (fans[5].duty * TOP_PWM_FAN5) >> 8;  // PWM5

  // * PWM 0, 1, 2, 5 will still produce a pulse-train when set to 0 or 100%
  //   dutycycle if we don't explicitly set them to low/high and disable PMW generation.
  // * PWM 3, 4 will still produce a pulse-train when set to 100%
  //   dutycycle if we don't explicitly set them to high and disable PMW generation.
  // Defining PWM_EXACT_0_AND_100_PERCENT_DUTY to 1 will fix this, at the cost of more code.

#if !PWM_EXACT_0_AND_100_PERCENT_DUTY
  for (i = 0; i < MAX_FANS; i++)
  {
    // Enable PWM generation on output pin
    switch (i)
    {
      case 0: ENABLE_PWM( TCCR_PWM_FAN0, COM_PWM_FAN0 ); break;
      case 1: ENABLE_PWM( TCCR_PWM_FAN1, COM_PWM_FAN1 ); break;
      case 2: ENABLE_PWM( TCCR_PWM_FAN2, COM_PWM_FAN2 ); break;
      case 3: ENABLE_PWM( TCCR_PWM_FAN3, COM_PWM_FAN3 ); break;
      case 4: ENABLE_PWM( TCCR_PWM_FAN4, COM_PWM_FAN4 ); break;
      case 5: ENABLE_PWM( TCCR_PWM_FAN5, COM_PWM_FAN5 ); break;
    }
  }
#else
  for (i = 0; i < MAX_FANS; i++)
  {
    if (    fans[i].duty == PWM_DUTY_0
         || fans[i].duty == PWM_DUTY_100 )
    {
      // Disable PWM generation on output pin
      switch (i)
      {
        case 0: DISABLE_PWM( TCCR_PWM_FAN0, COM_PWM_FAN0 ); break;
        case 1: DISABLE_PWM( TCCR_PWM_FAN1, COM_PWM_FAN1 ); break;
        case 2: DISABLE_PWM( TCCR_PWM_FAN2, COM_PWM_FAN2 ); break;
        case 3: DISABLE_PWM( TCCR_PWM_FAN3, COM_PWM_FAN3 ); break;
        case 4: DISABLE_PWM( TCCR_PWM_FAN4, COM_PWM_FAN4 ); break;
        case 5: DISABLE_PWM( TCCR_PWM_FAN5, COM_PWM_FAN5 ); break;
      }
      if ( fans[i].duty == PWM_DUTY_0 )
      {
        // Set output pin to constant low level (0% duty cycle)
        switch (i)
        {
          case 0: CLR_OUTPUT( PORT_PWM_FAN0, PIN_PWM_FAN0 ); break;
          case 1: CLR_OUTPUT( PORT_PWM_FAN1, PIN_PWM_FAN1 ); break;
          case 2: CLR_OUTPUT( PORT_PWM_FAN2, PIN_PWM_FAN2 ); break;
          case 3: CLR_OUTPUT( PORT_PWM_FAN3, PIN_PWM_FAN3 ); break;
          case 4: CLR_OUTPUT( PORT_PWM_FAN4, PIN_PWM_FAN4 ); break;
          case 5: CLR_OUTPUT( PORT_PWM_FAN5, PIN_PWM_FAN5 ); break;
        }
      }
      else
      {
        // Set output pin to constant high level (100% duty cycle)
        switch (i)
        {
          case 0: SET_OUTPUT( PORT_PWM_FAN0, PIN_PWM_FAN0 ); break;
          case 1: SET_OUTPUT( PORT_PWM_FAN1, PIN_PWM_FAN1 ); break;
          case 2: SET_OUTPUT( PORT_PWM_FAN2, PIN_PWM_FAN2 ); break;
          case 3: SET_OUTPUT( PORT_PWM_FAN3, PIN_PWM_FAN3 ); break;
          case 4: SET_OUTPUT( PORT_PWM_FAN4, PIN_PWM_FAN4 ); break;
          case 5: SET_OUTPUT( PORT_PWM_FAN5, PIN_PWM_FAN5 ); break;
        }
      }
    }
    else
    {
      // Enable PWM generation on output pin
      switch (i)
      {
        case 0: ENABLE_PWM( TCCR_PWM_FAN0, COM_PWM_FAN0 ); break;
        case 1: ENABLE_PWM( TCCR_PWM_FAN1, COM_PWM_FAN1 ); break;
        case 2: ENABLE_PWM( TCCR_PWM_FAN2, COM_PWM_FAN2 ); break;
        case 3: ENABLE_PWM( TCCR_PWM_FAN3, COM_PWM_FAN3 ); break;
        case 4: ENABLE_PWM( TCCR_PWM_FAN4, COM_PWM_FAN4 ); break;
        case 5: ENABLE_PWM( TCCR_PWM_FAN5, COM_PWM_FAN5 ); break;
      }
    }
  }
#endif
}


// Validate or calculate eeprom checksum.
// When write is true, the eeprom checksum will be calculated and stored for the current contents
// and the returned value is undefined.
// When write is false, the eeprom checksum will be validated; false will be returned when the checkum
// is invalid, true otherwise.
static inline uint8_t eepromChecksum(const uint8_t write)
{
  uint8_t* adr = (uint8_t*)&fans_eeprom[0];
  uint8_t chk = 0;
  // Calculate xor of all bytes in checksum-checked eeprom area.
  do {
    chk ^= eeprom_read_byte(adr++);
  } while (adr != &chk_eeprom);
  // Write the checksum to the eeprom area, or calculate it.
  if (write)
  {
    chk ^= EEPROM_CHK_MAGIC;
    eeprom_write_byte(&chk_eeprom,chk);
  }
  else
  {
    chk ^= eeprom_read_byte(&chk_eeprom);
  }
  return chk == EEPROM_CHK_MAGIC;
}

#if FAN_MODE_PI_SUPPORTED
// Restart control loop
static inline void restartCtrl( const uint8_t channel )
{
  fans[channel].i  = 0;
  fans[channel].es = 0;
}
#endif

// Try to load settings from eeprom.
// When eeprom data is invalid or parameter clear is set, new default values will be generated.
static inline void loadSettings( const uint8_t clear )
{
  // Load eeprom contents when checksum is valid, or defaults otherwise.
  uint8_t i;
  uint8_t eeprom_valid = !clear && eepromChecksum(0);
  for (i = 0; i < MAX_FANS; i++)
  {
    // Clear the whole structure. Any records which must be set to 0
    // will not explicitly have to be cleared, to save some bytes codespace.
    memset((void*)&(fans[i]),0,sizeof(fans[i]));
    if (eeprom_valid)
    {
      // Eeprom valid; load fan defaults from eeprom.
      eeprom_read_block((void*)&(fans[i]),&(fans_eeprom[i]),sizeof(fans[i]));
    }
    else
    {
      fans[i].config.fanType = FANTYPE_NONE;
      fans[i].config.fanMode = FANMODE_FIXED_DUTY;
      fans[i].dutyFixed = PWM_DUTY_100;
      fans[i].inc_per_pinrise = 256/2;
#if FAN_MODE_PI_SUPPORTED || FAN_MODE_FUZZY_SUPPORTED
      fans[i].snsSetp.delta = 20;           // get a sane starting value
      fans[i].snsSetp.snsIdx = 0;           // defaults to first reference sensor (which in turn defaults to no sensor). Set to 0 by memset
#endif
#if FAN_MODE_PI_SUPPORTED
      fans[i].Kp = 2;
      fans[i].Ki = 1;
      fans[i].Kt = 1;
#endif
#if (PWM_DUTY_0 != 0)                       // set to 0 by memset
      fans[i].dutyMin = PWM_DUTY_0;
#endif
      fans[i].dutyMax = PWM_DUTY_100;
      fans[i].min_rps = 5;                  // 5 rps -> 300 rpm
    }
//    fans[i].pinrise_count = 0;            // set to 0 by memset
//    fans[i].rps = 0;                      // set to 0 by memset
    restartCtrl(i);
  }

#if FAN_OUT_SUPPORTED
  memset((void*)&(fan_out),0,sizeof(fan_out));
  if (eeprom_valid)
  {
    // Eeprom valid; load fan_out defaults from eeprom.
    eeprom_read_block((void*)&(fan_out),&(fan_out_eeprom),sizeof(fan_out));
  }
  else
  {
#if (FANOUTMODE_RPS_MINFAN != 0)           // set to 0 by memset
    fan_out.mode = FANOUTMODE_RPS_MINFAN;
#endif
    fan_out.fanStallDetect = (1<<MAX_FANS)-1;  // include all fans in stall detection.
  }
#endif

  for (i = 0; i < MAX_SNS; i++)
  {
    memset((void*)&(sns[i]),0,sizeof(sns[i]));
    if (eeprom_valid)
    {
      // Eeprom valid; load sensor defaults from eeprom.
      eeprom_read_block((void*)&(sns[i]),&(sns_eeprom[i]),sizeof(sns[i]));
    }
    else
    {
#if (SNSTYPE_NONE != 0)               // set to 0 by memset
      sns[i].type = SNSTYPE_NONE;
#endif
    }
    sns[i].status.valid = 0;
  }
}

static inline void saveSettings()
{
  uint8_t i;
  // Write complete fan/sensor structs to eeprom, to be restored automatically
  // on AVR start.
  // Each struct contains too much data to be stored in eeprom, actually, but
  // it takes less code to simply store the complete struct.
  // Eeprom space is sufficient...
  for (i = 0; i < MAX_FANS; i++)
  {
    eeprom_write_block((void*)&(fans[i]),&(fans_eeprom[i]),sizeof(fans[i]));
  }
#if FAN_OUT_SUPPORTED
  eeprom_write_block((void*)&(fan_out),&(fan_out_eeprom),sizeof(fan_out));
#endif
  for (i = 0; i < MAX_SNS; i++)
  {
    eeprom_write_block((void*)&(sns[i]),&(sns_eeprom[i]),sizeof(sns[i]));
  }
  // Calculate & store new eeprom checksum.
  eepromChecksum(1);
}

void main() __attribute__ ((noreturn));
void main()
{
  uint8_t i;
  // Remember the cause of reset: watchdog-/brownout-/external-/poweron-reset.
  if (MCUSR & _BV(WDRF)) status |= STATUS_WATCHDOG_RESET;
  if (MCUSR & _BV(BORF)) status |= STATUS_BROWNOUT_RESET;
  MCUSR = 0;

#if ENTER_BOOTLOADER_REMOTELY
  if (eeprom_read_byte(&eeprom_bootload_trigger) == BOOTLOADER_MAGIC)
  {
    // We came here after a bootloader session triggered by writing a magic number to the eeprom.
    // Reset this number to prevent entering the bootloader again after a reset.
    eeprom_write_byte(&eeprom_bootload_trigger, 0);
  }
#endif

  // Enable watchdog. When the watchdog expires, the AVR resets and the WDRF
  // flag gets set in the MCUSR (reset_cause) register.
  wdt_enable(WDTO_1S);

  // RESET status: all port bits are inputs without pull-up.
  // That's the way we need D+ and D-. Therefore we don't need any
  // additional hardware initialization.
  usbInit();

  // Configure AD conversion
#if SNS_ANALOG_SUPPORTED || FAN_OUT_SUPPORTED
  ADCSRA =   _BV(ADEN)            // enable ADC
           | ADC_PRESCALER_BITS;  // prescaler
#endif

  // Load settings from eeprom, or revert to defaults when eeprom is invalid.
  loadSettings(0);

  // Configure PWM channels.
  initPwm();

#if SNS_I2C_SUPPORTED
  initI2C();
#endif

  // set TACHO's to input with internal pullup & enable pin change interrupts
  MAKE_INPUT( DDR_TACHO_FAN0, PIN_TACHO_FAN0 );
  MAKE_INPUT( DDR_TACHO_FAN1, PIN_TACHO_FAN1 );
  MAKE_INPUT( DDR_TACHO_FAN2, PIN_TACHO_FAN2 );
  MAKE_INPUT( DDR_TACHO_FAN3, PIN_TACHO_FAN3 );
  MAKE_INPUT( DDR_TACHO_FAN4, PIN_TACHO_FAN4 );
  MAKE_INPUT( DDR_TACHO_FAN5, PIN_TACHO_FAN5 );
  ENABLE_PULLUP( PORT_TACHO_FAN0, PIN_TACHO_FAN0 );
  ENABLE_PULLUP( PORT_TACHO_FAN1, PIN_TACHO_FAN1 );
  ENABLE_PULLUP( PORT_TACHO_FAN2, PIN_TACHO_FAN2 );
  ENABLE_PULLUP( PORT_TACHO_FAN3, PIN_TACHO_FAN3 );
  ENABLE_PULLUP( PORT_TACHO_FAN4, PIN_TACHO_FAN4 );
  ENABLE_PULLUP( PORT_TACHO_FAN5, PIN_TACHO_FAN5 );

  PCMSK2 = _BV(PCINT16) | _BV(PCINT17) | _BV(PCINT23); // enable pin change interrupt for TACHO's 0,1,2
  PCMSK0 = _BV(PCINT0)  | _BV(PCINT4)  | _BV(PCINT5);  // enable pin change interrupt for TACHO's 3,4,5
  PCICR |= _BV(PCIE0) | _BV(PCIE2);                    // enable Pin change interrupts 0 & 2

#if FAN_OUT_SUPPORTED
  // Configure generated TACHO output
  // According to Intel 4-Wire Pulse Width Modulation Controlled Fans, rev 1.2:
  // Fan shall provide tachometer output signal with the following characteristics:
  // * Two pulses per revolution
  // * Open-collector or open-drain type output
  // * Motherboard will have a pull up to 12V, maximum 13.2V
  MAKE_INPUT( DDR_TACHO_OUT,  PIN_TACHO_OUT );   // Tristate pin
  CLR_OUTPUT( PORT_TACHO_OUT, PIN_TACHO_OUT );   // ..
#endif
#define MAKE_INPUT(ddr,pin)         (ddr &= ~_BV(pin))

  usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */

  /* fake USB disconnect for > 250 ms */
  i = 0;
  while(--i)
  {
    wdt_reset();
    _delay_ms(1);
  }
  usbDeviceConnect();

  sei();

  /* main event loop */
  for(;;)
  {
#if FAN_OUT_SUPPORTED
    static uint16_t rps_out_counter;
#endif
    wdt_reset();
    usbPoll();  // to be called at least once every 50ms

#if FAN_OUT_SUPPORTED
    if (timer1_ovf_counter >= rps_out_counter)
    {
      rps_out_counter += rps_out_tmr1_ovf_interval;
      if (IS_OUTPUT( DDR_TACHO_OUT, PIN_TACHO_OUT ))
        MAKE_INPUT( DDR_TACHO_OUT, PIN_TACHO_OUT );     // Tristate pin; motherboard will pull up to Vcc.
      else
        MAKE_OUTPUT( DDR_TACHO_OUT, PIN_TACHO_OUT );    // Output low
    }
#endif

    if (timer1_ovf_counter >= FREQ_PWM)
    {
      // A second has passed

      // Reset timer1 overflow counter, to allow accurate timing within each second.
      timer1_ovf_counter -= FREQ_PWM;

      for (i = 0; i < MAX_FANS; i++)
      {
        // Only 3- or 4-pin fans have a tacho signal
        if ( fans[i].config.fanType >= MIN_FANTYPE_WITH_TACHO )
        {
          // Convert nr of pinrises to nr of revolutions/sec.
          // The lower 8 bits of fans[i].rps represent the fraction.
          fans[i].rps = (uint16_t)(fans[i].pinrise_count) * (uint16_t)(fans[i].inc_per_pinrise);
          // A fan will be flagged as stalled when:
          // * it is a 3- or 4-wire fan
          // * min_rps > 0
          // * current rps < min_rps
          fans[i].status.stalled = (fans[i].min_rps > 0) && ((fans[i].rps >> 8) < fans[i].min_rps);
        }
        else
        {
          fans[i].rps = 0;
          fans[i].status.stalled = 0;
        }
        fans[i].pinrise_count = 0;
      }

      // Update sensor values
      updateSns();

      // Update fan control loops and calculate new PWM duty cycles.
      for (i = 0; i < MAX_FANS; i++)
      {
        updateCtrl(i);
      }

      // Activate new Pwm dutycycles.
      updatePwmOut();

#if FAN_OUT_SUPPORTED
      // Update output fan PWM
      updateFanOut();
      rps_out_counter = rps_out_tmr1_ovf_interval;
#endif

#if (USB_COMM_WATCHDOG_TIMEOUT > 0)
      // Increment the usb communication watchdog timer.
      // This timer counts the number of seconds elapsed since
      // last USB command was received. If it becomes larger than
      // the specified amount, a safety sequency can be started.
      // When the watchdog is triggered, USB communication will
      // clear the watchdog state again!
      if (usb_comm_watchdog_count < USB_COMM_WATCHDOG_TIMEOUT)
      {
        usb_comm_watchdog_count++;
        status &= ~STATUS_USBCOMM_WATCHDOG_TO;
      }
      else
      {
        // DO something 'safe' ;-)
        status |= STATUS_USBCOMM_WATCHDOG_TO;
      }
#endif
    }
  }
}

/* ------------------------------------------------------------------------- */
