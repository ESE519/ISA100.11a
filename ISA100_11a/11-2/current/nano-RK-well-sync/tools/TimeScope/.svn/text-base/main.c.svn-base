#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

#define NONE	0
#define ASCII   1
#define DECSIM	2
// There are 28800 ticks per second
#define TICK_DIVISOR	28.8

void print_usage ();

char buf[1024];

main (int Parm_Count, char *Parms[])
{

  long BAUD, DATABITS, STOPBITS, PARITYON, PARITY;
  char devicename[80];
  char logname[80];
  char type[80];
  char debug_flag[80];
  int fd, tty, slipin, slipout, res, i, error,state,pin_mask;
  int ts_low, ts_high; 
  int ts, last_ts,tmp_ts;
  unsigned long timestamp;
  uint8_t c,debug;
  FILE *np,*fp;
  int mode, val;
  //place for old and new port settings for serial port
  struct termios oldtio, newtio;
  int start_flag;

  mode=NONE;

  BAUD = B115200;
  DATABITS = CS8;
  STOPBITS = 0;
  //STOPBITS = CSTOPB;
  PARITYON = 0;
  PARITY = 0;
  //PARITYON = PARENB;
  //PARITY = PARODD;

  if (Parm_Count < 2 || Parm_Count>4 || Parm_Count==3 ) 
    print_usage ();

  strcpy (buf, Parms[1]);
  i = sscanf (buf, "%s", devicename);
  if (i != 1)
    print_usage ();
  printf ("opening: %s\n", devicename);
  //open the device(com port) to be non-blocking (read will return immediately)
  fd = open (devicename, O_RDWR | O_NOCTTY );
  if (fd < 0) {
    perror (devicename);
    exit (-1);
  }
if(Parm_Count>2 )
{
  strcpy (buf, Parms[2]);
  i = sscanf (buf, "%s", type);
  if (i != 1 )
    print_usage ();
  if(type[1]=='D' ) {
	mode=DECSIM;
	printf( "Mode: DECSIM output\n" );
	}
  else if(type[1]=='A' ) {
	mode=ASCII;
	printf( "Mode: ASCII output\n" );
	}
  else {
	printf( "Mode: %s is unkown, select DECSIM or ASCII\n",type );
	exit(-1);
	}
  strcpy (buf, Parms[3]);
  i = sscanf (buf, "%s", logname);
  if (i != 1)
    print_usage ();
  printf ("saving to opening: %s\n", logname);
  //open the device(com port) to be non-blocking (read will return immediately)
  fp = fopen (logname, "w" ); 
  if (fp ==NULL) {
    perror (logname);
    exit (-1);
  }
}

  tcgetattr (fd, &oldtio);      // save current port settings 
  // set new port settings for canonical input processing 
  newtio.c_cflag =
    BAUD | DATABITS | STOPBITS | PARITYON | PARITY | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;
  newtio.c_lflag = 0;           //ICANON;
  newtio.c_cc[VMIN] = 1;
  newtio.c_cc[VTIME] = 0;
  tcflush (fd, TCIFLUSH);
  tcsetattr (fd, TCSANOW, &newtio);

  start_flag = 0;
  res= write( fd,"\n",1 );
  res= write( fd,"\n",1 );
  res= write( fd,"o",1 );
  state=3;
  timestamp=0;
  last_ts=0;

printf( "\n\nData format: Time DEBUG_0 DEBUG_1 DEBUG_2 DEBUG_3\n" ); 
if(mode==DECSIM)
{ 
  fprintf( fp,"!         DDDD\n" );
  fprintf( fp,"!         EEEE\n" );
  fprintf( fp,"!         BBBB\n" );
  fprintf( fp,"!         UUUU\n" );
  fprintf( fp,"!         GGGG\n" );
  fprintf( fp,"!         ____\n" );
  fprintf( fp,"!         0123\n" );
}

  while (1) {
    res = read (fd, &c, 1);
    if(res==1)
    	{
	if(c&0x80) state=0;
	switch(state)
		{
		case 0:
			pin_mask=(c>>3)&0x0F;
			ts_low=0;
			ts_high=0;
			ts_high=(c&0x3)<<6;
			state=1;
			break;
		case 1:
			ts_high|=((c&0x7E)>>1);
			ts_low=((c&1)<<7);
			state=2;
			break;
		case 2:
			ts_low|=(c&0x7F);
			ts=(ts_high<<8)+ts_low;
			tmp_ts=ts;
			if(ts>last_ts)
				ts=ts-last_ts;
			else 
			{
				ts+=65536-last_ts; // ts++;

			}
			last_ts=tmp_ts;
			timestamp+=ts;
			printf( "%ld %d %d %d %d\n",(long) ((float) timestamp/TICK_DIVISOR),!!(pin_mask&0x8), !!(pin_mask&0x4), !!(pin_mask&0x2), !!(pin_mask&0x1)); 
			if(mode==DECSIM)
				fprintf( fp,"%9ld %d%d%d%d\n",(long) ((float)timestamp/TICK_DIVISOR),!!(pin_mask&0x8), !!(pin_mask&0x4), !!(pin_mask&0x2), !!(pin_mask&0x1)); 
			if(mode==ASCII)
				fprintf( fp,"%ld %d %d %d %d\n",(long) ((float)timestamp/TICK_DIVISOR),!!(pin_mask&0x8), !!(pin_mask&0x4), !!(pin_mask&0x2), !!(pin_mask&0x1)); 

			if(mode==ASCII || mode==DECSIM) fflush(fp);
			state=3;
			break;
		default:
			state=3;
			break;	
		}

	}
   }
}                               //end of main



void print_usage ()
{
  printf ("Usage: TimeScope com-port -[DECSIM|ASCII] log-file-name\n");
  printf ("  Ex: TimeScope /dev/ttyUSB1\n");
  printf ("  Ex: TimeScope /dev/ttyUSB1 -DECSIM log.tra\n");
  printf ("  Ex: TimeScope /dev/ttyUSB1 -ASCII  log.txt\n\n");
  exit (-1);

}
