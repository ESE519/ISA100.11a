#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGESIZE 256

int main(int argc, char **argv)
{
  unsigned char byte;
  unsigned int updateImgSize = 0;
  unsigned int updatePages = 0;
  unsigned int updateVersion = 0;
  unsigned int updateLessBytes = 0;
  unsigned int updateMode = 0;
  unsigned int updateChecksum = 0;
  
  FILE* update_bin; // create a new file pointer
  FILE* update_c;
  FILE* update_h;
	
  if(argc < 3)
  {
    printf("USAGE: binder <filename.bin> <update_mode, 1:PATCH, 2:FULL_BIN_FLASH> \r\n");
    return 1;
  }
  else
  {
    
  }

  if((update_bin = fopen(argv[1],"rb"))==NULL)
  { // open a file
    printf("Could not open <%s>\n", argv[1]); // print an error
    exit(1);
  }
	
  if((update_c = fopen("update.c","w"))==NULL)
  { // open a file
    printf("Could not open <update.c>\n"); // print an error
    exit(1);
  }
	
  if((update_h = fopen("update.h","w"))==NULL)
  { // open a file
    printf("Could not open <update.h>\n"); // print an error
    exit(1);
  }

  fprintf(update_c,"const unsigned char update[] __attribute__ ((section (\".UpdateSection\"))) =\n{\n\n");

  // Read old image checksum from patch file
  fread(&byte,sizeof(char),1,update_bin);
  updateChecksum = byte;
  
  while(!feof(update_bin)){
	
    if(fread(&byte,sizeof(char),1,update_bin) == 1){
      updateImgSize++;
      fprintf(update_c,"0x%x,\t", byte);
    }
    if(!(updateImgSize % 10)) fprintf(update_c, "\n");
  }
  fprintf(update_c,"0x00\n\n};\n");

  fprintf(update_h,"#define UpdateImgSize 0x%x\n", updateImgSize);
  
  updatePages = updateImgSize / PAGESIZE;
  if(updateImgSize % PAGESIZE > 0){
    updatePages ++;
    updateLessBytes = PAGESIZE - (updateImgSize % PAGESIZE);
  }
  fprintf(update_h,"#define UpdatePages 0x%x\n", updatePages);
  fprintf(update_h,"#define UpdateLessBytes 0x%x\n", updateLessBytes);
  fprintf(update_h,"#define UpdateVersion 0x%x\n", updateVersion);
  fprintf(update_h,"#define UpdateChecksum 0x%x\n", updateChecksum);
  fprintf(update_h,"#define UpdateMode %s\r\n", argv[2]);
  
  fclose(update_bin);
  fclose(update_c);
  fclose(update_h);

  return 0;
}

