#
#             LUFA Library
#     Copyright (C) Dean Camera, 2017.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
#
# --------------------------------------
#         LUFA Project Makefile.
# --------------------------------------

# Run "make help" for target help.

MCU          = atmega32u4
ARCH         = AVR8
BOARD        = BOARD_NONE
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = usb_control
PROGRAM_CMD  = /home/wenk/bin/hid_bootloader_cli -v -mmcu=$(MCU) $(TARGET).hex
LUFA_PATH    = ../../lufa/LUFA
FASTLED_PATH = ../../libFastLed
FASTLED_INCL = -I$(FASTLED_PATH) -I$(FASTLED_PATH)/arduino-1.0.1/cores/arduino -I$(FASTLED_PATH)/arduino-1.0.1/variants/leonardo -include Arduino.h
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -I../common/ -IConfig/ -Ii2c/ -Ipower/ -Iled/ -Ifastled/ -Ifan/ -Iusb_serial/ -Iusb_raw/ $(FASTLED_INCL) 
#LD_FLAGS     = -Wl,-u,vfprintf -lprintf_min -L$(FASTLED_PATH)/Release -lFastLed
LD_FLAGS     = -L$(FASTLED_PATH)/Release -lFastLed
LUFA_SRC     = $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)

SRC          = \
	lufa.c \
	lufa_virtual_serial.c \
	descriptors.c \
	i2c/twimaster.c \
	i2c/i2c.c \
	led/led.c \
	power/wire.c \
	power/ina219.c \
	power/ina260.c \
	power/power.c \
	fastled/led_control.cpp \
	fastled/animation.cpp \
	usb_serial/shell.c \
	usb_serial/shell_fan.c \
	usb_serial/shell_fan_out.c \
	usb_serial/shell_sensor.c \
	usb_serial/shell_max31790.c \
	usb_serial/shell_ina219.c \
	usb_serial/shell_powermeter.c \
	usb_raw/usb_generic_hid.c \
	fan/ctrl.c \
	fan/fan.c \
	fan/fan_out.c \
	fan/max31790.c \
	fan/mcp980x.c \
	fan/pwm.c \
	fan/sensor.c \
	fan/settings.c \
	fan/timer.c \
	$(LUFA_SRC)



# Default target
all:

# Include LUFA-specific DMBS extension modules
DMBS_LUFA_PATH ?= $(LUFA_PATH)/Build/LUFA
include $(DMBS_LUFA_PATH)/lufa-sources.mk
include $(DMBS_LUFA_PATH)/lufa-gcc.mk

# Include common DMBS build system modules
DMBS_PATH      ?= $(LUFA_PATH)/Build/DMBS/DMBS
include $(DMBS_PATH)/core.mk
include $(DMBS_PATH)/cppcheck.mk
include $(DMBS_PATH)/doxygen.mk
include $(DMBS_PATH)/dfu.mk
include $(DMBS_PATH)/gcc.mk
include $(DMBS_PATH)/hid.mk
include $(DMBS_PATH)/avrdude.mk
include $(DMBS_PATH)/atprogram.mk

# Program the device.  
program: $(TARGET).hex $(TARGET).eep
	$(PROGRAM_CMD)

boot: 
	@echo teensy_loader_cli: Boot only, do not program
	teensy_loader_cli --mcu $(MCU) -v -b
	
	
	