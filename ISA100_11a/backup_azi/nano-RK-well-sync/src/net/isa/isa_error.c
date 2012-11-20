#include <isa_error.h>


// This uint8_t will store the latest error
static uint8_t isaError;


//********************Local function definitions***********************************

void setIsaError(uint8_t);
uint8_t getIsaError ();
void printIsaError();

//***********************************************************************************



void setIsaError(uint8_t value)
{
	isaError = value;
}

uint8_t getIsaError ()
{
	return isaError;
}

void printIsaError()
{
	switch(isaError)
	{
	case LINK_CAPACITY_ERROR :
	printf ("ISA_ERROR : LINK_CAPACITY_ERROR \n\r" );
	break;
	case NEIGHBOR_CAPACITY_ERROR :
		printf ("ISA_ERROR : NEIGHBOR_CAPACITY_ERROR\n\r" );
		break;
	case TRANSMIT_QUEUE_CAPACITY_ERROR :
		printf ("ISA_ERROR : TRANSMIT_QUEUE_CAPACITY_ERROR\n\r" );
		break;
	case MAX_PAYLOAD_ERROR :
		printf ("ISA_ERROR : MAX_PAYLOAD_ERROR\n\r");
		break;
	default: printf ("Unknown ISA_ERROR");
	}


}
