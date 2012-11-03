/*
 * nrk_pin_define.h for IMEC node
 *
 * Contributing authors (this file):
 * Mark Hamilton
 */

#ifndef NRK_PIN_DEFINE_H
#define NRK_PIN_DEFINE_H

/*
 * GPIO handling functions these macros perform raw hw access ports and pins
 * are acctual hw ports and pins.
 */

// use pin_port, and pin; ie: nkr_gpio_raw_set( PORTB, DEBUG_0 )
#define nrk_gpio_raw_set( _port, _pin ) {do { _port |= BM(_pin); } while(0);}
// use pin_port, and pin; ie: nkr_gpio_raw_clr( PORTB, DEBUG_0 )
#define nrk_gpio_raw_clr( _port, _pin ) {do { _port &= ~BM(_pin); } while(0);}
// use pin_port, and pin; ie: nkr_gpio_raw_get( PINB, DEBUG_0 )
#define nrk_gpio_raw_get( _pin_port, _pin ) (_pin_port & BM(_pin))
// use pin_port, port and pin; ie: nkr_gpio_raw_toggle( PINB, PORTB, DEBUG_0 )
#define nrk_gpio_raw_toggle( _pin_port, _port, _pin ) { \
        if ((_pin_port & BM(_pin))) do{ _port &= ~BM(_pin); } while(0); \
        else do { _port |= BM(_pin); }while(0);  \
}

// use direction; ie: nkr_gpio_raw_direction( DDRB, DEBUG_0 )
#define nrk_gpio_raw_direction( _direction_port_name, _pin, _pin_direction ) {\
        if (_pin_direction == NRK_PIN_INPUT) { \
                _direction_port_name &= ~BM( _pin ); \
        } else { \
                _direction_port_name |= BM( _pin ); \
        } \
}

/*
 * Macros to define a pin as used by higher level programs.  Higher level
 * programs refer to pin as NRK_<pin name> these functions declare these
 * NRK_<pin name> pins and provide the mappings to the hardware
 */
#define DECLARE_NRK_PIN( _pin_name )\
 extern const uint8_t NRK_ ## _pin_name;
#define NRK_PIN( _pin_name, _pin , _port )\
 const uint8_t NRK_ ## _pin_name = (_pin << 3) + (_port & 0x07);
#define NRK_INVALID_PIN( _pin_name )\
 const uint8_t NRK_ ## _pin_name = NRK_INVALID_PIN_VAL;

/*
 * when a platform does not support one of the NRK_<pin name> declared below,
 * it must define it has an invalid pin in the platform ulib.c (e.g. a platform
 * that does not support NRK_DEBUG_0 should have the following in ulib.c
 * NRK_INVALID_PIN( NRK_DEBUG_0 )).
 */
#define NRK_INVALID_PIN_VAL 0xFF

/*
 * Define pin directions.
 */
#define NRK_PIN_INPUT 0
#define NRK_PIN_OUTPUT 1

/*
 * NRK ports NRK_<hw port> used for the mapping to the real hw.
 * (3 bits reserved for ports)
 */
#define NRK_PORTA 1
#define NRK_PORTB 2
#define NRK_PORTC 3
#define NRK_PORTD 4
#define NRK_PORTE 5
#define NRK_PORTF 6

/*
 * Pin names for IMEC node follow those given on the schematic.
 */
#define IMP_CS 0 //P1.0
#define IMP_WR 1 //P1.1
#define UP_A0_ECG 4 //P1.4
#define UP_A1_ECG 5 //P1.5
#define UP_A2_ECG 6 //P1.6
#define ASIC_CLK 7 //P1.7 NOTE: pin renamed to avoid confilcts

#define ENABLE1 0 //P2.0
#define START_UP 1 //P2.1
#define OLD_IMP_STIM_NO_LONGER_USED 2 //P2.2
#define UP_RESET 3 //P2.3
#define UP_A0_EEG 4 //P2.4
#define UP_A1_EEG 5 //P2.5
#define UP_A2_EEG 6 //P2.6
#define DR1_RF 7 //P2.7

#define LED 0 //P3.0
#define DATA_RF 1 //P3.1 TODO: this also connects to P3.2
#define CLK1_RF 3 //P3.3
#define TXD 6 //P3.6
#define RXD 7 //P3.7

#define CS_RF 1 //P4.1
#define DOUT2_RF 2 //P4.2
#define CLK2_RF 3 //P4.3
#define DR2_RF 4 //P4.4
#define CE_RF 5 //P4.5
#define PWR_UP_RF 6 //P4.6

#define MUXA0 0 //P5.0
#define MUXA1 1 //P5.1
#define MUXA2 2 //P5.2
#define MUXA3 3 //P5.3
#define MUXA4 4 //P5.4
#define IMP_STIM 5 //P5.5

#define ASIC_OUT 0 //P6.0 NOTE: pin renamed to avoid confilct with timera.h
#define IMP_OUT 2 //P6.2
#define MISC1 3 //P6.3
#define MISC2 4 //P6.4
#define MISC3 5 //P6.5
#define MISC4 6 //P6.6

/*
 * Declare pins as used by higher level programs Mapping to the hardware is
 * done by ulib.c
 */
DECLARE_NRK_PIN(IMP_CS)
DECLARE_NRK_PIN(IMP_WR)
DECLARE_NRK_PIN(UP_A0_ECG)
DECLARE_NRK_PIN(UP_A1_ECG)
DECLARE_NRK_PIN(UP_A2_ECG)
DECLARE_NRK_PIN(ASIC_CLK)
DECLARE_NRK_PIN(ENABLE1)
DECLARE_NRK_PIN(START_UP)
DECLARE_NRK_PIN(OLD_IMP_STIM_NO_LONGER_USED)
DECLARE_NRK_PIN(UP_RESET)
DECLARE_NRK_PIN(UP_A0_EEG)
DECLARE_NRK_PIN(UP_A1_EEG)
DECLARE_NRK_PIN(UP_A2_EEG)
DECLARE_NRK_PIN(DR1_RF)
DECLARE_NRK_PIN(LED)
DECLARE_NRK_PIN(DATA_RF)
DECLARE_NRK_PIN(CLK1_RF)
DECLARE_NRK_PIN(TXD)
DECLARE_NRK_PIN(RXD)
DECLARE_NRK_PIN(CS_RF)
DECLARE_NRK_PIN(DOUT2_RF)
DECLARE_NRK_PIN(CLK2_RF)
DECLARE_NRK_PIN(DR2_RF)
DECLARE_NRK_PIN(CE_RF)
DECLARE_NRK_PIN(PWR_UP_RF)
DECLARE_NRK_PIN(MUXA0)
DECLARE_NRK_PIN(MUXA0)
DECLARE_NRK_PIN(MUXA0)
DECLARE_NRK_PIN(MUXA0)
DECLARE_NRK_PIN(MUXA0)
DECLARE_NRK_PIN(IMP_STIM)
DECLARE_NRK_PIN(ASIC_OUT)
DECLARE_NRK_PIN(IMP_OUT)
DECLARE_NRK_PIN(MISC1)
DECLARE_NRK_PIN(MISC2)
DECLARE_NRK_PIN(MISC3)
DECLARE_NRK_PIN(MISC4)

#endif
