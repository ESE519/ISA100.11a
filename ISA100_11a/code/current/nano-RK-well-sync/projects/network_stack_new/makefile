# Platform name  cc2420DK, firefly, micaZ
PLATFORM = firefly2_2
#SHELL = /bin/bash 

# Target file name (without extension).
#TARGET = App1-client
#TARGET = App1-server
TARGET = $(TAR)

#do:
#	echo $(TAR)
#	echo $(TARGET)
#	sleep 5
#done:
#do:
#	echo $(TARGET) && 
#	sleep 5
#done:

# Set the Port that you programmer is connected to 
PROGRAMMING_PORT = /dev/ttyUSB1 # programmer connected to serial device

# Set this such that the nano-RK directory is the base path
ROOT_DIR = ../..

# Set platform specific defines 
# The following will be defined based on the PLATFORM variable:
# PROG_TYPE  (e.g. avrdude, or uisp)
# MCU (e.g. atmega32, atmega128, atmega1281) 
# RADIO (e.g. cc2420)
include $(ROOT_DIR)/include/platform.mk


SRC = $(TARGET).c

# Add extra source files. 
SRC += $(ROOT_DIR)/src/net/bmac/bmac.c
SRC += $(ROOT_DIR)/projects/network_stack_new/BufferManager.c
SRC += $(ROOT_DIR)/projects/network_stack_new/NetworkLayerReliable.c
#SRC += $(ROOT_DIR)/projects/network_stack_new/NetworkLayer.c
SRC += $(ROOT_DIR)/projects/network_stack_new/TransportLayerUDP.c
SRC += $(ROOT_DIR)/projects/network_stack_new/LinkLayer.c
SRC += $(ROOT_DIR)/projects/network_stack_new/Debug.c
SRC += $(ROOT_DIR)/projects/network_stack_new/Pack.c
SRC += $(ROOT_DIR)/projects/network_stack_new/NWStackConfig.c
SRC += $(ROOT_DIR)/projects/network_stack_new/Serial.c
SRC += $(ROOT_DIR)/projects/network_stack_new/CriticalRegion.c
SRC += $(ROOT_DIR)/src/net/slip/slip.c
SRC += $(ROOT_DIR)/src/drivers/platform/$(PLATFORM_TYPE)/source/ff_basic_sensor.c

# Add extra includes files. 
# For example:
EXTRAINCDIRS =
EXTRAINCDIRS += $(ROOT_DIR)/src/net/bmac
EXTRAINCDIRS += $(ROOT_DIR)/src/net/dsr
EXTRAINCDIRS += $(ROOT_DIR)/projects/network_stack_new
EXTRAINCDIRS += $(ROOT_DIR)/src/net/slip

# set of custom -D options
# By default the CTG is FALSE(0)
ifndef CTG
CTG = 0
endif
CFLAGS = -D CONNECTED_TO_GATEWAY=$(CTG)
#  This is where the final compile and download happens
include $(ROOT_DIR)/include/platform/$(PLATFORM)/common.mk
