
#include <spi_matrix.h>

#include <include.h>
#include <ulib.h>
#include <stdio.h>
//#include <error-def.h>
//#include <command-interpreter.h>

MATRIX_TABLE matrixTable[] = {
		/*
		{1, {{126,0,191}, {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},					//1	  	0
		{2, {{116,0,191}, {92,0,127},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//2		1
		{2, {{0,84,191}, {0,124,127},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//3		2
		{2, {{0,28,191}, {0,124,127},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//4		3
		{2, {{0,92,191}, {0,116,127},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//5		4
		{2, {{0,126,191}, {0,112,127},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//6		5
		{2, {{0,4,191}, {0,124,127},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//7		6
		{2, {{0,124,191}, {0,124,127},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//8		7
		{2, {{0,28,191}, {0,252,129},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//9		8
		{5, {{126,0,254}, {66,0,253},{66,0,251},{82,0,247},{114,0,239},{0,0,0},{0,0,0},{0,0,0}}, 0},	//G		9
		{5, {{2,0,254}, {2,0,253},{126,0,251},{2,0,247},{2,0,239},{0,0,0},{0,0,0},{0,0,0}}, 0},	//T				10
		{5, {{126,0,254}, {64,0,253},{64,0,251},{64,0,247},{64,0,239},{0,0,0},{0,0,0},{0,0,0}}, 0},	//L			11
		{5, {{126,0,254}, {4,0,253},{8,0,251},{16,0,247},{126,0,239},{0,0,0},{0,0,0},{0,0,0}}, 0},	//N			12
		{5, {{126,0,254}, {66,0,253},{66,0,251},{66,0,247},{66,0,239},{0,0,0},{0,0,0},{0,0,0}}, 0},	//C			13
		*/
		{1, {{126,0,253}, {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},					//1	  	0
			{2, {{116,0,253}, {92,0,254},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//2		1
			{2, {{0,84,253}, {0,124,254},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//3		2
			{2, {{0,28,253}, {0,124,254},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//4		3
			{2, {{0,92,253}, {0,116,254},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//5		4
			{2, {{0,126,253}, {0,112,254},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//6		5
			{2, {{0,4,253}, {0,124,254},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//7		6
			{2, {{0,124,253}, {0,124,254},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//8		7
			{2, {{0,28,253}, {0,252,254},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, 0},				//9		8
			{5, {{126,0,127}, {66,0,191},{66,0,223},{82,0,239},{114,0,247},{0,0,0},{0,0,0},{0,0,0}}, 0},	//G		9
			{5, {{2,0,127}, {2,0,191},{126,0,223},{2,0,239},{2,0,247},{0,0,0},{0,0,0},{0,0,0}}, 0},	//T				10
			{5, {{126,0,127}, {64,0,191},{64,0,223},{64,0,239},{64,0,247},{0,0,0},{0,0,0},{0,0,0}}, 0},	//L			11
			{5, {{126,0,127}, {4,0,191},{8,0,223},{16,0,239},{126,0,247},{0,0,0},{0,0,0},{0,0,0}}, 0},	//N			12
			{5, {{126,0,127}, {66,0,191},{66,0,223},{66,0,239},{66,0,247},{0,0,0},{0,0,0},{0,0,0}}, 0},	//C			13
};


uint8_t charIndex;	//used tp store character Index
uint8_t numIndex;	// used to store the number Index
nrk_time_t displayStartTime;
bool displayNeeded = false;

uint8_t toggle = 0;
/*
 void spiSend(void)
{
	MATRIX_CLEAR();
	//Send data to the matrix
	//FASTSPI_TX_WORD(emberUnsignedCommandArgument(0));	//writes 2 bytes to spi
	//FASTSPI_WAIT()
	FASTSPI_TX(emberUnsignedCommandArgument(0));
	FASTSPI_TX(emberUnsignedCommandArgument(1));
	FASTSPI_TX(emberUnsignedCommandArgument(2));

	//Make the shift register output the data that we send
	MATRIX_DISPLAY();
}
*/
void spiPatternSend(uint8_t p1, uint8_t p2,uint8_t p3){

	MATRIX_CLEAR();
	//Send data to the matrix
	//FASTSPI_TX_WORD(emberUnsignedCommandArgument(0));	//writes 2 bytes to spi
	//FASTSPI_WAIT()
	FASTSPI_TX(p1);
	FASTSPI_TX(p2);
	FASTSPI_TX(p3);

	//Make the shift register output the data that we send
	MATRIX_DISPLAY();
}



void setNewDisplay(uint8_t cIndex, uint8_t nIndex){

	//the indexes should be within bounds to avoid accessing weird parts of memory

	if(nIndex<9) numIndex = nIndex;
	if (cIndex > 8 && cIndex < 14) charIndex = cIndex;
	 nrk_time_get(&displayStartTime);
	 displayNeeded = true;

}

void setMatrix(){

	nrk_time_t currentTime;
	  //Do the display thing
if (displayNeeded == true){
	putchar('p');
	   		if (toggle <= 2){
			spiPatternSend(matrixTable[charIndex].pattern[matrixTable[charIndex].currentIndex][0],matrixTable[charIndex].pattern[matrixTable[charIndex].currentIndex][1],matrixTable[charIndex].pattern[matrixTable[charIndex].currentIndex][2]);
			toggle++ ;//= !toggle;
			matrixTable[charIndex].currentIndex++;
			if (matrixTable[charIndex].currentIndex >= matrixTable[charIndex].size) matrixTable[charIndex].currentIndex = 0;

	}
	else {
			spiPatternSend(matrixTable[numIndex].pattern[matrixTable[numIndex].currentIndex][0],matrixTable[numIndex].pattern[matrixTable[numIndex].currentIndex][1],matrixTable[numIndex].pattern[matrixTable[numIndex].currentIndex][2]);
			toggle = 0;//!toggle;
			matrixTable[numIndex].currentIndex++;
			if (matrixTable[numIndex].currentIndex >= matrixTable[numIndex].size) matrixTable[numIndex].currentIndex = 0;
	}
	   			 nrk_time_get(&currentTime);
	   			 if (currentTime.secs - displayStartTime.secs > DISPLAY_INTERVAL_SECS){
	   				MATRIX_CLEAR();
	   				 displayNeeded = false;
	   			 }
	   			putchar('q');
}

}
