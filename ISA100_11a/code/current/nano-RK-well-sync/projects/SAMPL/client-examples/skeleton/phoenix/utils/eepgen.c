#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGESIZE 256
#define MAX_LOAD_SECTION_SIZE ((40*1024) -1)

unsigned char load_section[MAX_LOAD_SECTION_SIZE];

int main(int argc, char **argv)
{
  char truncfile[20];
	
  unsigned char byte;
  unsigned int read_b = 0;
  unsigned int load_section_size = 0;
  unsigned int img_size = 0;
  unsigned int img_page_size = 0;
  unsigned int version = 0;
  unsigned char node_id = 0;
  unsigned char my_channel = 0;
  FILE* img_bin; // create a new file pointer
  FILE* trunc_h;
  FILE* trunc_bin;
  FILE* trunc_eep;
	
  if(argc < 4)
  {

    printf("USAGE: eepgen <filename.bin> <MAC> <MY_RADIO_CHANNEL>\r\n");
    return 1;
  }


  if((img_bin = fopen(argv[1],"rb"))==NULL)
  { // open a file
    printf("Could not open <%s>\n", argv[1]); // print an error
    exit(1);
  }
  
  if((trunc_eep = fopen("main.eep","wb"))==NULL)
  { // open a file
    printf("Could not open <trunc.eep>\n"); // print an error
    exit(1);
  }

  read_b = fread( load_section, sizeof(unsigned char), MAX_LOAD_SECTION_SIZE, img_bin);
  if(read_b < MAX_LOAD_SECTION_SIZE)
  {
    printf("FILE READ ERROR %lu\r\n",read_b);
    exit(1);
  }

  for(read_b = MAX_LOAD_SECTION_SIZE; read_b >= 0; read_b--)
  {
    if(load_section[read_b] != 0x00) break;
  }
  if(read_b == 0 )
  {
    printf("EMPTY LOAD SECTION\r\n");
    exit(2);
  }

  load_section_size = read_b + 256 - (read_b % PAGESIZE);
  img_page_size = load_section_size / PAGESIZE;
  if(load_section_size % PAGESIZE > 0) img_page_size ++;

  node_id = atoi(argv[2]);
  my_channel = atoi(argv[3]);
  fwrite(&node_id, sizeof(unsigned char), 1, trunc_eep);
  fwrite(&my_channel, sizeof(unsigned char), 1, trunc_eep);
  fwrite(&img_page_size, sizeof(unsigned char), 1, trunc_eep);
  fcloseall();
  printf("MAC: 0x%X\r\n", node_id);
  printf("CHANNEL: 0x%X\r\n", my_channel);
  printf("LOAD PAGES: 0x%X\r\n", img_page_size);
  return 0;
}
