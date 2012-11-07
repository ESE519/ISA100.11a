#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGESIZE 256
#define MAX_LOAD_SECTION_SIZE ((48*1024) -1)

unsigned char load_section[MAX_LOAD_SECTION_SIZE];

char program_str[1024];
char binfile[128];
char port_path[32];
char mac_addr[32];
char aes_key[33];
int channel;
int load_pages;
unsigned char image_checksum;

void print_usage ();
int computeLoadPages();
    
int main (int Parm_Count, char *Parms[]) 
{
int i,checksum,val;
int aes_flag, phoenix_flag;
char tmp[1024];
char buf[1024];
char buf0[32];
char buf1[32];
char buf2[32];
char buf3[32];
int mac[4];


  if(Parm_Count<4) print_usage();

aes_flag=0;
phoenix_flag=0;
load_pages=0;

  strcpy (tmp, Parms[1]);
  i = sscanf (tmp, "%s", port_path);
  if (i != 1)
	print_usage ();
  strcpy (tmp, Parms[2]);
  i = sscanf (tmp, "%s", mac_addr);
  if (i != 1)
	print_usage ();
  strcpy (tmp, Parms[3]);
  i = sscanf (tmp, "%d", &channel);
  if (i != 1)
	print_usage ();

if(Parm_Count>=5)
{
strcpy (tmp, Parms[4]);
  i = sscanf (tmp, "%s", aes_key);
  if (i != 1)
    print_usage ();
    aes_flag=1;
 }


if(Parm_Count>=6)
{
strcpy (tmp, Parms[5]);
  i = sscanf (tmp, "%s", binfile);
  if (i != 1)
    print_usage ();
    phoenix_flag=1;
 }


 printf( "port=%s\n",port_path);
 printf( "mac_addr=%s\n",mac_addr);
 printf( "channel=%d\n",channel);
sprintf(program_str,"avrdude -b115200 -F -p atmega128 -P %s -c avr109 -V -U",port_path );

//val=scanf("%s %s %s %s",buf0,buf1,buf2,buf3);
buf0[0]=mac_addr[0]; buf0[1]=mac_addr[1]; buf0[3]='\0';
buf1[0]=mac_addr[2]; buf1[1]=mac_addr[3]; buf1[3]='\0';
buf2[0]=mac_addr[4]; buf2[1]=mac_addr[5]; buf2[3]='\0';
buf3[0]=mac_addr[6]; buf3[1]=mac_addr[7]; buf3[3]='\0';
val=sscanf( buf0,"%x",&mac[3]);
val+=sscanf( buf1,"%x",&mac[2]);
val+=sscanf( buf2,"%x",&mac[1]);
val+=sscanf( buf3,"%x",&mac[0]);

if(val==4)
	{
  
	if(phoenix_flag==1)
	{
		printf("Computed Load Pages\r\n");
		load_pages = computeLoadPages();
	}
	checksum = mac[3];
	checksum = (checksum+mac[2])&0xff;
	checksum = (checksum+mac[1])&0xff;
	checksum = (checksum+mac[0])&0xff;
	
	if(aes_flag==0) 
	{
		sprintf(buf,"%s eeprom:w:\"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\":m\n",program_str,mac[3],mac[2],mac[1],mac[0],checksum,channel);
		printf( "Writing MAC 0x%x%x%x%x with checksum 0x%x channel=0x%x\n",mac[3],mac[2],mac[1],mac[0],checksum, channel);
	} else
	{
		sprintf(buf,"%s eeprom:w:\"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x ",program_str,mac[3],mac[2],mac[1],mac[0],checksum,channel);
		sprintf(buf,"%s 0x%x 0x%x",buf, load_pages, image_checksum);
		for(i=0; i<32; i+=2)
			sprintf(buf,"%s 0x%c%c",buf, aes_key[i],aes_key[i+1] );
		sprintf(buf,"%s\":m\n",buf );
		printf( "Writing MAC 0x%x%x%x%x with checksum 0x%x channel=0x%x aes-key=0x%s loadpages=0x%x image_checksum=0x%x\n",mac[3],mac[2],mac[1],mac[0],checksum, channel, aes_key,load_pages, image_checksum);
	}

	system(buf); 
	printf( "MAC written\n" );
	}
else printf( "Format Error\n" );


}

int computeLoadPages()
{
  int i;

  char truncfile[20];

  int image_page_size;

  unsigned char data = 0;
  unsigned char byte;
  unsigned int read_b = 0;
  unsigned int load_section_size = 0;
  unsigned int img_size = 0;
  unsigned int img_page_size = 0;
  unsigned int version = 0;

  FILE* img_bin; // create a new file pointer
	
  if((img_bin = fopen(binfile,"rb"))==NULL)
  { // open a file
    printf("Could not open <%s>\n", binfile); // print an error
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

  //Compute Checksum
  for ( i = 0; i < read_b+1; i++ )
  {
    data += load_section[i];
  }
  //Store Checksum
  image_checksum = data;


  load_section_size = read_b + 256 - (read_b % PAGESIZE);
  img_page_size = load_section_size / PAGESIZE;
  if(load_section_size % PAGESIZE > 0) img_page_size ++;

  fcloseall();
  
  return img_page_size;
}

void print_usage ()
{
  printf ("Usage: config-eeprom com-port mac-address channel [encryption-key] [filename]\n");
  printf ("  Ex: config-eeprom /dev/ttyUSB1 1234beef 25 [00112233445566778899AABBCCDDEEFF] [main.bin]\n");
  printf ("  com-port      Serial port device for programmer\n");
  printf ("  mac-address   4 byte hex mac address\n");
  printf ("  channel       802.15.4 channel 10-16\n");
  printf ("  aes-key       16 byte key in hex [optional]\n");
  printf ("  phoenix-file  Compiled bin file used to calcluate page size [optional]\n");
  exit (-1);
}

