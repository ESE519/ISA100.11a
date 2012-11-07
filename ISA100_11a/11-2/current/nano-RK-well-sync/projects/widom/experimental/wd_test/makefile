# Platform name  cc2420DK, firefly, micaZ, firefly2, firefly2_2
PLATFORM = firefly2 


# Target file name (without extension).
TARGET = main

# Set the Port that you programmer is connected to 
#PROGRAMMING_PORT = /dev/ttyUSB1 # programmer connected to serial device
PROGRAMMING_PORT = COM2 # programmer connected to serial device

# Set this such that the nano-RK directory is the base path
ROOT_DIR = ../../../..

# Set platform specific defines 
# The following will be defined based on the PLATFORM variable:
# PROG_TYPE  (e.g. avrdude, or uisp)
# MCU (e.g. atmega32, atmega128, atmega1281) 
# RADIO (e.g. cc2420)
include $(ROOT_DIR)/include/platform.mk


SRC = $(TARGET).c

# Add extra source files. 
# For example:
# SRC += $(ROOT_DIR)/src/platform/$(PLATFORM_TYPE)/source/my_src1.c 
SRC += $(ROOT_DIR)/src/net/widom/experimental-versions/widom.c
SRC += $(ROOT_DIR)/src/net/widom/experimental-versions/platform/$(PLATFORM_TYPE)/widom_timer.c
SRC += $(ROOT_DIR)/src/net/widom/experimental-versions/radio/$(RADIO)/widom_rf.c
SRC += $(ROOT_DIR)/src/net/widom/experimental-versions/radio/$(RADIO)/widom_linx_rf.c

# Add extra includes files. 
# For example:
# EXTRAINCDIRS += $(ROOT_DIR)/src/platform/include
EXTRAINCDIRS = $(ROOT_DIR)/src/net/widom/experimental-versions/
EXTRAINCDIRS += $(ROOT_DIR)/src/net/widom/experimental-versions/platform/$(PLATFORM_TYPE)
EXTRAINCDIRS += $(ROOT_DIR)/src/net/widom/experimental-versions/radio/$(RADIO)

#  This is where the final compile and download happens
include $(ROOT_DIR)/include/platform/$(PLATFORM)/common.mk
