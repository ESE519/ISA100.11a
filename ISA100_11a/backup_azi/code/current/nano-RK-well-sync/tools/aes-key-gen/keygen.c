#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(void)
{
uint8_t v;
srand(time(0));
printf( "%08x%08x%08x%08x\n",
	(uint32_t)rand(),
	(uint32_t)rand(),
	(uint32_t)rand(),
	(uint32_t)rand());
return 1;
}
