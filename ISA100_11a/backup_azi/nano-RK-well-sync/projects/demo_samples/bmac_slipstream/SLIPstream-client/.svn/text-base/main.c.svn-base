#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <slipstream.h>

#define NONBLOCKING  0
#define BLOCKING     1

int main (int argc, char *argv[])
{
  char buffer[128];
  int v,cnt,i;

  if (argc != 3) {
    printf ("Usage: server port\n");
    exit (1);
  }

  v=slipstream_open(argv[1],atoi(argv[2]),BLOCKING);
  
    sprintf (buffer, "Ignore this Setup Message\n");
    v=slipstream_send(buffer,strlen(buffer));
    if (v == 0) printf( "Error sending\n" );


  cnt = 0;
  while (1) {

    v=slipstream_receive( buffer );
    if (v > 0) {
      printf ("Got: ");
      for (i = 0; i < v; i++)
        printf ("%c", buffer[i]);
      printf ("\n");
    } else printf( "Error reading packet\n" );
  }



}

