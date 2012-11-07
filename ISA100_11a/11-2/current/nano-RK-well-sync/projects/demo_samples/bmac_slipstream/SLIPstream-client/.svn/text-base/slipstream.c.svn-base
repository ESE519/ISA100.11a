#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <slipstream.h>

  int sock, length, n, i, cnt;
  struct sockaddr_in server, from;
  struct hostent *hp;


int slipstream_open(char *addr, int port, int blocking_read)
{
  sock = socket (AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
	  {
    		perror ("socket");
		return 0;
	  }

  if(blocking_read==0)
	{
  	// make socket non-blocking...
  	fcntl (sock, F_SETFL, O_NONBLOCK);
	}
  
  server.sin_family = AF_INET;
  hp = gethostbyname (addr);
  if (hp == 0)
	  {
    		perror ("Unknown host");
		return 0;
	  }

  bcopy ((char *) hp->h_addr, (char *) &server.sin_addr, hp->h_length);
  server.sin_port = htons (port);
  length = sizeof (struct sockaddr_in);

return 1;
}

int slipstream_send(char *buf, int size)
{
int n;
    n = sendto (sock, buf, size, 0, (struct sockaddr *) &server, length);
    if (n < 0)
      {
      perror ("Sendto");
      return 0;
      }

return 1;
}

int slipstream_receive(char *buf)
{
int n;
    n = recvfrom (sock, buf, MAX_BUF, 0, (struct sockaddr *) &from, &length);
return n;
}

