



#include <include.h>

/*
 * this debug pin is connected to SRCLR(serial clear)
 * It is used to clear the values held by the shift registers
 * Clears on falling edge
 */
#define MATRIX_CLEAR() \
do{\
PORTF &= ~(0x10);\
PORTF |= 0x10;\
}\
while(0)

//nrk_gpio_clr(NRK_DEBUG_0);\
//nrk_gpio_set(NRK_DEBUG_0);\
}\
while(0)

/*
 * This debug pin is connected to RCLK. It is used to make the shift register store its value to the storage register
 * and output it on its output lines
 */

#define MATRIX_DISPLAY()\
do{\
PORTF |= 0x20;\
PORTF &= ~(0x20);\
}\
while(0)
//nrk_gpio_set(NRK_DEBUG_1);\
//nrk_gpio_clr(NRK_DEBUG_1);\
}\
while(0)

#define DISPLAY_INTERVAL_SECS 4

typedef struct {
	uint8_t size;
	uint8_t pattern[8][3];
	uint8_t currentIndex;
}MATRIX_TABLE;

extern void spiSend(void);
extern void spiPatternSend(uint8_t p1, uint8_t p2,uint8_t p3);
extern void setMatrix();
extern void setNewDisplay(uint8_t cIndex, uint8_t nIndex);

