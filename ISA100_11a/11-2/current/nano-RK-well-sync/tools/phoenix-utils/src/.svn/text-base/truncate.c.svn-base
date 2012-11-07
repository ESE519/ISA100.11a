#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGESIZE 256
#define MAX_LOAD_SECTION_SIZE ((48*1024) -1)

unsigned char load_section[MAX_LOAD_SECTION_SIZE];

int main(int argc, char **argv)
{
    char truncfile[128];

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
    //FILE* trunc_eep;

    if(argc < 2)
    {

      printf("USAGE: truncate <filename.bin>\r\n");
      return 1;
    }


    if((img_bin = fopen(argv[1],"rb"))==NULL)
    { // open a file
            printf("Could not open <%s>\n", argv[1]); // print an error
            exit(1);
    }

    sprintf(truncfile, "%s.trunc",argv[1]);

    if((trunc_bin = fopen(truncfile,"wb"))==NULL)
    { // open a file
            printf("Could not open <trunc.bin>\n"); // print an error
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
    fwrite(load_section, sizeof(unsigned char), load_section_size, trunc_bin);

    fcloseall();
    return 0;
}
