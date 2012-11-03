#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <slipstream.h>

#define NONBLOCKING  0
#define BLOCKING     1
#define MM 17

void meta_file_generator(char *buf);

int main (int argc, char *argv[])
{
  char buffer1[48];
  char buffer2[MM];
  //unsigned char buf[3];
  int v,cnt,i;

  if (argc != 3) {
    printf ("Usage: server port\n");
    exit (1);
  }

  v=slipstream_open(argv[1],atoi(argv[2]),NONBLOCKING);
  
  sprintf (buffer1, "This is a sample slip string: Count %d\n", cnt);

  v=slipstream_send(buffer1,strlen(buffer1)+1);
  if (v == 0) printf( "Error sending\n" );
  
  //cnt = 0;
  //bzero(buffer2,11);
  while (1) {
    //cnt++;
    //sprintf (buffer1, "This is a sample slip string: Count %d\n", cnt);

    //v=slipstream_send(buffer1,strlen(buffer1)+1);
    //if (v == 0) printf( "Error sending\n" );

    v=slipstream_receive( buffer2);
    //printf("V:%d",v);
    //buffer2[10]='\0';
    if (v > 0) {
	    meta_file_generator(buffer2);
    }
    
	// for test
	/*else{
		 buf[0]=0xff;
		 buf[1]=0xff;
		 buf[2]=0xff;
		 meta_file_generator(buf);
	}*/

// Pause for 1 second 
    sleep (1);
  }


}

void meta_file_generator(char *buf)
{
	FILE * metadata;
	int i;
	char fileName[10];
	char zero_killer=0xaa;

	sprintf(fileName, "%d", buf[0]-1);
        strcat(fileName, ".txt");
	//fileName="test";
	metadata = fopen(fileName,"a+");
	for(i=1;i<MM;i++){
		buf[i] ^= zero_killer;			
		fputc(buf[i],metadata);
		//printf("%d",buf[i]);
	}
	fclose(metadata);

}

