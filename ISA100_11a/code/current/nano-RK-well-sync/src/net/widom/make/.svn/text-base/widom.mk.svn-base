# Widom source files. 
ifdef SRC 
SRC += $(ROOT_DIR)/src/net/widom/widom.c
else
SRC = $(ROOT_DIR)/src/net/widom/widom.c
endif
SRC += $(ROOT_DIR)/src/net/widom/hal/$(MCU)/wd_timer.c
SRC += $(ROOT_DIR)/src/net/widom/radio/$(RADIO)/wd_rf.c

# Widom include files.
ifdef EXTRAINCDIRS 
EXTRAINCDIRS += $(ROOT_DIR)/src/net/widom
else
EXTRAINCDIRS = $(ROOT_DIR)/src/net/widom
endif
EXTRAINCDIRS += $(ROOT_DIR)/src/net/widom/hal/$(MCU)
EXTRAINCDIRS += $(ROOT_DIR)/src/net/widom/radio/$(RADIO)
EXTRAINCDIRS += $(ROOT_DIR)/src/net/widom/platform/$(PLATFORM_TYPE)
