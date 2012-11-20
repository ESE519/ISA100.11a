#include <stdlib.h>    
#include <stdio.h>
#include <string.h>

char*  bintochar(int c);
static char binarr[8];

int main (int argc, char *argv[])
{
  FILE *rawFile;  
  int ch;
  FILE *dataFile;
  int cnt=0;
  int i=0;
  char *t;
  char fileName[10];
  char outputName[10];

  if (argc != 3) {
    printf ("Please input file name\r\n");
    exit (1);
  }

  strcpy(fileName,argv[1]);
  strcpy(outputName,argv[2]);
  strcat(outputName,"out.txt");

  rawFile = fopen(fileName,"r");
  dataFile = fopen(outputName,"w");
  while((ch=fgetc(rawFile))!=EOF){ 
	 //convert and save to a new file
	 //printf("ch :%d\r\n",ch); 
	 t=bintochar(ch);
	 //for(i=0;i<8;i++){
		//printf("%d",t[i]);
	 fprintf(dataFile,"%s\n",t);

	 //printf("\r\n");
	 //fprintf(dataFile,"%c",ch);
  }
  fclose(rawFile);
  fclose(dataFile);
}

char*  bintochar(int c)
{
    int i;
    for(i=7;i>=0;i--)
    {
        binarr[i]=c%2+'0';
		//printf("%d",binarr[i]);     
        c=c/2;     
    }
	 
    return binarr;
}
