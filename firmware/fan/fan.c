
#include "fan.h"
#include "ctrl.h"
#include "fan_out.h"
#include "sensor.h"
#include "powermeter.h"
#include "pwm.h"
#include "timer.h"
#include "settings.h"
#include "debug.h"

Fan fans[MAX_FANS];
Fan EEMEM fans_eeprom[MAX_FANS]; // first entry in eeprom; start of crc calculation.

static uint8_t status = 0;

uint8_t getStatus(void)
{
	return status;
}

void fanControlInit()
{
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
    // wdt_enable(WDTO_1S);

    initTimer();

    // Init i2c and analog sensors
    initSns();

    initPowerMeter();

    // Load settings from eeprom, or revert to defaults when eeprom is invalid.
    loadSettings(0);

    // Configure PWM channels.
    initPwm();

    initFanOut();
}

void fanControlUpdate()
{
    setFanOut();

    if (timer1_ovf_counter >= FREQ_PWM)
    {
        // A second has passed
    	LS_("fup");

        // Reset timer1 overflow counter, to allow accurate timing within each second.
        timer1_ovf_counter -= FREQ_PWM;

        updatePwm();
        updateSns();
        updatePowerMeter();

        // Update fan control loops and calculate new PWM duty cycles.
        for (uint8_t i = 0; i < MAX_FANS; i++)
        {
            updateCtrl(i);
        }

        // Activate new Pwm dutycycles.
        updatePwmOut();

#if FAN_OUT_SUPPORTED
        // Update output fan PWM
        updateFanOut();
        resetFanOutCounter();
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
