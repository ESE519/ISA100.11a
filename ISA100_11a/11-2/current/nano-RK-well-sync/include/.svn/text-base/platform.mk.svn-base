
# We use the stripped version to avoid typos 
PLATFORM_TYPE = $(strip $(PLATFORM))

ifeq ($(PLATFORM_TYPE),firefly)
# FireFly Architecture specific includes should go here...
# Notice that this sets architecture path, the mcu type and the radio
PROG_TYPE = avrdude
MCU = atmega32
RADIO = cc2420
PLATFORM_FOUND= true 
endif

ifeq ($(PLATFORM_TYPE),firefly2_1)
# micaZ Architecture specific includes should go here...
# Notice that this sets architecture path, the mcu type and the radio
PROG_TYPE = avrdude
MCU = atmega128
RADIO = cc2420
PLATFORM_FOUND = true 
endif

ifeq ($(PLATFORM_TYPE),firefly2_2)
# micaZ Architecture specific includes should go here...
# Notice that this sets architecture path, the mcu type and the radio
PROG_TYPE = avrdude
MCU = atmega1281
RADIO = cc2420
PLATFORM_FOUND = true 
endif


ifeq ($(PLATFORM_TYPE),micaZ)
# micaZ Architecture specific includes should go here...
# Notice that this sets architecture path, the mcu type and the radio
PROG_TYPE = uisp
MCU = atmega128
RADIO = cc2420
PLATFORM_FOUND = true 
endif

ifeq ($(PLATFORM_TYPE),cc2420DK)
# micaZ Architecture specific includes should go here...
# Notice that this sets architecture path, the mcu type and the radio
MCU = atmega128
RADIO = cc2420
PLATFORM_FOUND = true 
endif

ifeq ($(PLATFORM_TYPE),tmote)
# tmote Architecture specific includes should go here...
# Notice that this sets architecture path, the mcu type and the radio
PROG_TYPE = msp430-bsl
MCU = msp430x1611
RADIO = cc2420
PLATFORM_FOUND = true 
endif

ifeq ($(PLATFORM_TYPE),expboard)
# tmote Architecture specific includes should go here...
# Notice that this sets architecture path, the mcu type and the radio
PROG_TYPE = msp430-bsl
MCU = msp430xG4618
RADIO = cc2420
PLATFORM_FOUND = true 
endif

ifeq ($(PLATFORM_TYPE),imec)
# tmote Architecture specific includes should go here...
# Notice that this sets architecture path, the mcu type and the radio
PROG_TYPE = msp430-bsl
MCU = msp430x149
RADIO = cc2420
PLATFORM_FOUND = true 
endif

