# Platform name  cc2420DK, firefly2_1, firefly2_2, micaZ
PLATFORM = firefly2_2


# Target file name (without extension).
TARGET = main

# Set the Port that you programmer is connected to 
PROGRAMMING_PORT = /dev/ttyUSB1 # programmer connected to serial device

# Set this such that the nano-RK directory is the base path
ROOT_DIR = ../../../..

# Set platform specific defines 
# The following will be defined based on the PLATFORM variable:
# PROG_TYPE  (e.g. avrdude, or uisp)
# MCU (e.g. atmega32, atmega128, atmega1281) 
# RADIO (e.g. cc2420)
include $(ROOT_DIR)/include/platform.mk


SRC = $(TARGET).c
SRC += transducer_handler.c
SRC += ../../transducer_pkts/ff_basic_sensor_pkt.c
SRC += ../../client_core/sampl_tasks.c
SRC += ../../client_core/aggregate.c
SRC += ../../client_core/generate.c
SRC += ../../client_core/p2p_handler.c
SRC += ../../client_core/debug.c
SRC += ../../pkt_handlers/transducer_pkt.c
SRC += ../../pkt_handlers/ping_pkt.c
SRC += ../../pkt_handlers/ack_pkt.c
SRC += ../../pkt_handlers/route_pkt.c
SRC += ../../pkt_handlers/stats_pkt.c
SRC += ../../pkt_handlers/trace.c
SRC += ../../pkt_handlers/neighbor_pkt.c
SRC += ../../pkt_handlers/eeprom_data.c
SRC += ../../pkt_handlers/control_pkt.c
SRC += ../../pkt_handlers/xmpp_pkt.c
SRC += ../../include/pkt_packer.c

ifdef PHOENIX
  SRC += ./phoenix/phoenix.c
  SRC += ./phoenix/nanopatch.c
  SRC += ./phoenix/bootloader.c
endif
# Add extra source files.
# For example:
SRC += $(ROOT_DIR)/src/net/route_table/route_table.c
SRC += $(ROOT_DIR)/src/net/neighbor_list/neighbor_list.c
SRC += $(ROOT_DIR)/src/net/bmac/bmac.c
SRC += $(ROOT_DIR)/src/drivers/platform/$(PLATFORM_TYPE)/source/ff_basic_sensor.c

# Add extra includes files. 
# For example:
EXTRAINCDIRS =
EXTRAINCDIRS += ../../client_core
EXTRAINCDIRS += ../../pkt_handlers
EXTRAINCDIRS += ../../include
EXTRAINCDIRS += $(ROOT_DIR)/src/net/bmac
EXTRAINCDIRS += $(ROOT_DIR)/src/net/route_table
EXTRAINCDIRS += $(ROOT_DIR)/src/net/neighbor_list


#  This is where the final compile and download happens
#include $(ROOT_DIR)/include/platform/$(PLATFORM)/common.mk
ifdef PHOENIX
  include phoenix.mk
else
  include $(ROOT_DIR)/include/platform/$(PLATFORM)/common.mk
endif
