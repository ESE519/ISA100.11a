# Platform name  cc2420DK, firefly, micaZ, firefly2, firefly2_2
PLATFORM = firefly2_2


# Target file name (without extension).
TARGET = main

# Set the Port that you programmer is connected to 
PROGRAMMING_PORT = /dev/ttyUSB1 # programmer connected to serial device

# Set this such that the nano-RK directory is the base path
ROOT_DIR = ../../..

# Set platform specific defines 
# The following will be defined based on the PLATFORM variable:
# PROG_TYPE  (e.g. avrdude, or uisp)
# MCU (e.g. atmega32, atmega128, atmega1281) 
# RADIO (e.g. cc2420)
include $(ROOT_DIR)/include/platform.mk


SRC = $(TARGET).c
SRC += $(ROOT_DIR)/src/net/rt_link/rt_link.c 
SRC += $(ROOT_DIR)/src/net/rt_link/rtl_scheduler.c 
SRC += $(ROOT_DIR)/src/net/rt_link/rtl_debug.c 
SRC += $(ROOT_DIR)/src/drivers/platform/$(PLATFORM_TYPE)/source/ff_basic_sensor.c 

# Add extra source files. 
# For example:
# SRC += $(ROOT_DIR)/src/platform/$(PLATFORM_TYPE)/source/my_src1.c 
# SRC += $(ROOT_DIR)/src/platform/$(PLATFORM_TYPE)/source/my_src2.c 

# Add extra includes files. 
# For example:
# EXTRAINCDIRS += $(ROOT_DIR)/src/platform/include
EXTRAINCDIRS += $(ROOT_DIR)/src/net/rt_link/
EXTRAINCDIRS += $(ROOT_DIR)/src/net/rt_link/platform/$(PLATFORM_TYPE)/


#  This is where the final compile and download happens
include $(ROOT_DIR)/include/platform/$(PLATFORM)/common.mk
