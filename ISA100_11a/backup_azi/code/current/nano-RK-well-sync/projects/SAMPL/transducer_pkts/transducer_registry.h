// This file contains a list of the pkt types for any registered
// transducers.  If each packet type is not unique, it will be 
// difficult for the gateway to determine the contents of a transducer
// packet.

#define TRAN_EMPTY		0
#define TRAN_ACK		1
#define TRAN_NCK		2
#define TRAN_FF_BASIC_SHORT	3
#define TRAN_FF_BASIC_LONG	4
#define TRAN_RAW_ADC		5
#define TRAN_POWER_CTRL_SOCK_0	6
#define TRAN_POWER_CTRL_SOCK_1	7
#define TRAN_POWER_SENSE	8



// Actuator Values
#define SOCKET_OFF	0
#define SOCKET_ON	1
