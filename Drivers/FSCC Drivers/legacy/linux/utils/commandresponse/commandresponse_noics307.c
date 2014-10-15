/* $Id */
/*
make as:
gcc -O2 -o commandresponse commandresponse.c 

Copyright(c) 2008, Commtech, Inc.
commandresponse.c -- user mode function to write a command and wait for a response

usage:
 commandresponse port [m|s]

 The port can be any valid fscc port (0,1) 
 m is master mode, the port generating the command and waiting for the response
 s is slave mode,  the port listening for the command and generating the response

*/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "../fsccdrv.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <math.h>
#include <sys/poll.h>

#include <time.h>
#include <termio.h>
#include <malloc.h>
#include <string.h>

//function to display FSCC status results
void decode_status(unsigned long stat);

//used to do user/keyboard interaction
static struct termio save_term;


void
SetRaw(int fd)
{
  struct termio s;
  //Get terminal modes.
  (void) ioctl(fd, TCGETA, &s);
  //Save modes and set certain variables dependent on modes.
  save_term = s;
  //Set the modes to the way we want them.
  s.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL);
  s.c_oflag |= (OPOST | ONLCR | TAB3);
  s.c_oflag &= ~(OCRNL | ONOCR | ONLRET);
  s.c_cc[VMIN] = 0;
  s.c_cc[VTIME] = 0;
  (void) ioctl(fd, TCSETAW, &s);
}

static void
TermRestore(int fd)
{
  struct termio s;
  //Restore saved modes.
  s = save_term;
  (void) ioctl(fd, TCSETAW, &s);
}
unsigned char RxBuf[32780];
unsigned char TxBuf[32768];
struct clockstruct clock1843200 = {1843200,
 {0x32,0x00,0x0b,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,
 0x00,0x82,0x6a,0x47,0x01,
 0x84,0x00,0xff,0xff,0xff}};



//main program code

int
main(int argc, char *argv[])
{
  int fd;
  int i;
  unsigned long status;
  unsigned long desreg;
  setup settings;
  char nbuf[80];
  int port;
  FILE *fin;
  int fscc = -1;
  unsigned char *buf;
  char cbuf;
  unsigned rcount;
  unsigned wcount;
  unsigned long wtotal = 0;
  int blocksz;
  unsigned wrsize;
  int type;
  int n;
  time_t tnow;
  unsigned long passval[2];
  unsigned long fcount;
  unsigned long rtimeout;
  long ltime;
  int stime;
  unsigned long desfreq = 1843200;
  unsigned char progwords[20];
  struct clockstruct clock;
  int masterslave;
  int ret;
  ltime = time(NULL);
  stime = (unsigned int) ltime / 2;
  srand(stime);

  if (argc < 3)
    {
      printf("usage:\n%s port [m|s]\n", argv[0]);

      printf
	("\tThe port can be any valid fscc port (0,1,etc)\n\tm=master to generate commands\n\ts=slave to listen for commands\n");
      exit(1);
    }

//get the fscc port to use and open up its devnode
  port = atoi(argv[1]);
  sprintf(nbuf, "/dev/fscc%u", port);

  blocksz = 4096;		//assume 4k reads (1byte to 4k writes)

  buf = (unsigned char *) malloc(blocksz + 16);	//allocate buffer for transfers
  if (buf == NULL)
    {
      printf("cannot allcoate memory for buffer!\n");
      exit(1);
    }

//select between master mode and slave mode
  if (argv[2][0] == 's')
    {
      masterslave = 0;
    }
  else
    masterslave = 1;

//open fscc devnode
  fscc = open(nbuf, O_RDWR);
  if (fscc == -1)
    {
      printf("cannot open %s\n", nbuf);
      perror(NULL);
      exit(1);
    }
  printf("opened %s\r\n", nbuf);

//configure the onboard clock generator for the port
//we are setting it to 1.8432MHz which divides down nicely to 38400bps
//  printf("setting clock to 	:%lu\n", desfreq);
  printf("setting clock to 	:%lu\n", 1843200);

//  if (find_ics30703_solution(desfreq, 2, &progwords[0]) == 0)
//    {
//      clock.freq = desfreq;
//      for (i = 0; i < 19; i++)
//	clock.progword[i] = progwords[i];
      ioctl(fscc, FSCC_SET_FREQ, &clock1843200);
      printf("successful!\n");
//   }
//  else
//    {
//      printf("failed! no solution found, increase ppm?\n");
//    }

//configure the features register to use RS-485 on the data line
//as we have a single data pair (2-wire RS-485) between the master and slave
  desreg = 0;
//if(rxecho     == 1)           desreg|=0x01;
//if(sd485      == 1)           desreg|=0x02;
//if(tt485      == 1)           desreg|=0x04;
//if(dtrfst     == 1)             desreg|=0x08;
//if(asyncmode  == 1)           desreg|=0x80000000;
  desreg = 0x03;		//turn on rx echo cancel and sd=485
  printf("features:%8.8lx\r\n", desreg);
  if (ioctl(fscc, FSCC_SET_FEATURES, &desreg) == -1)
    perror(NULL);


  if (masterslave == 1)
    {
//master mode register settings
      settings.fifot = 0x08001000;
      settings.cmdr = 0x08020000;
      settings.ccr0 = 0x00112010;	//clock mode 4, HDLC, NRZ
      settings.ccr1 = 0x04000018;
      settings.ccr2 = 0x00000000;
      settings.bgr = 0x00000002;
      settings.ssr = 0x0000007e;
      settings.smr = 0x00000000;
      settings.tsr = 0x0000007e;
      settings.tmr = 0x00000000;
      settings.rar = 0x00000000;
      settings.ramr = 0x00000000;
      settings.ppr = 0x00000000;
      settings.tcr = 0x00000000;
      settings.imr = 0x0f000101;
      settings.n_rbufs = 10;
      settings.n_tbufs = 10;
      settings.n_rfsize_max = 4096;
      settings.n_tfsize_max = 4096;
    }
  else
    {
//slave mode register settings
      settings.fifot = 0x08001000;
      settings.cmdr = 0x08020000;
      settings.ccr0 = 0x00112008;	//clock mode 2, HDLC, NRZ
      settings.ccr1 = 0x04000018;
      settings.ccr2 = 0x00000000;
      settings.bgr = 0x00000002;
      settings.ssr = 0x0000007e;
      settings.smr = 0x00000000;
      settings.tsr = 0x0000007e;
      settings.tmr = 0x00000000;
      settings.rar = 0x00000000;
      settings.ramr = 0x00000000;
      settings.ppr = 0x00000000;
      settings.tcr = 0x00000000;
      settings.imr = 0x0f000101;
      settings.n_rbufs = 10;
      settings.n_tbufs = 10;
      settings.n_rfsize_max = 4096;
      settings.n_tfsize_max = 4096;

    }
//configure the FSCC register settings
  if (ioctl(fscc, FSCC_INIT, &settings) == -1)
    {
      perror(NULL);
      printf("failed settings ioctl call\n");
      close(fscc);
      exit(1);
    }
  else
    {
      printf("%s settings set\n", nbuf);
    }


//lets start clean
//flush the transmitter and receiver
  if (ioctl(fscc, FSCC_FLUSH_TX, NULL) == -1)
    perror(NULL);
  else
    printf("%s ==> TX Flushed\n", nbuf);
  if (ioctl(fscc, FSCC_FLUSH_RX, NULL) == -1)
    perror(NULL);
  else
    printf("%s ==> RX Flushed\n", nbuf);

//enter into the main command/response loop
  if (masterslave == 1)
    {
//master mode selected so lets send a command and wait for a response
      fcount = 0;
      fd = 0;
      SetRaw(fd);		//this configures the user input to not require a CR to take input
      while (1)
	{
//give the user some options on commands to send
	  printf("1. send 0x55\n");
	  printf("2. send 0xaa\n");
	  printf("3. send 0x01ff30313233\n");
	  printf("q. quit\n");
	  printf("enter command:\n");

	  n = 0;
	  while (n != 1)	//wait for the user to hit a key
	    {
	      n = read(fd, &cbuf, 1);
	      if (n == 1)
		{
		  if ((cbuf == 'q') || (cbuf == 'Q'))
		    {
		      //user is finished
		      goto fini;
		    }
		  if ((cbuf == '1'))
		    {
		      //selected command frame #1
		      buf[0] = 0x55;
		      wrsize = 1;
		    }
		  if ((cbuf == '2'))
		    {
		      //selected command frame #2
		      buf[0] = 0xaa;
		      wrsize = 1;
		    }
		  if ((cbuf == '3'))
		    {
		      //selected commadn frame #3
		      buf[0] = 0x01;
		      buf[1] = 0xff;
		      buf[2] = 0x30;
		      buf[3] = 0x31;
		      buf[4] = 0x32;
		      buf[5] = 0x33;
		      wrsize = 6;
		    }
		}
	    }
//actually send the command out the FSCC port
	  if ((wcount = write(fscc, buf, wrsize)) == -1)
	    {
	      printf("write failed:errno = %i\n", errno);
	      perror(NULL);
	    }
	  else
	    {
	      //no error on send, so
	      //wait for response.
	      rtimeout = 0;
	    rto:
	      if ((rcount = read(fscc, buf, 4096)) == -1)
		{
		  printf("read failed:%i\n", errno);
		  perror(NULL);
		}
	      else
		{
		  if (rcount == 0)
		    {
		      rtimeout++;
		      if (rtimeout > 1)
			{
			  printf("response timeout!\n");
			}
		      else
			goto rto;
		    }
//a message was received
//this is just display stuff, an actual program would decode/do something
//with the message data here

		  for (i = 0; i < 40; i++)
		    {
		      printf("-");
		    }
		  time(&tnow);
		  printf("\n%s", ctime(&tnow));

		  printf("Port:%u Read returned:%i bytes\n", port, rcount);

		  if (rcount > 2)	//only process if it is a valid frame
		    //all returned frames have the receive status bytes appended to them, so they are at least 3 bytes long
		    {
//display the received data contents
		      for (i = 0; i < rcount - 2; i++)
			printf("%x:", buf[i]);
		      printf("\n");
//these only make sense for HDLC...decode of the received status bytes
		      printf("last bytes:%x:%x\n", buf[rcount - 2] & 0xff, buf[rcount - 1] & 0xff);
		      if ((buf[rcount - 1] & 0x02) == 0x00)
			{
			  printf("CRC-PASSED ");
			}
		      else
			{
			  printf("CRC-FAILED ");
			}
		      if ((buf[rcount - 1] & 0x08) == 0x00)
			{
			  printf("VALID ");
			}
		      else
			{
			  printf("INVALID ");
			}
		      if ((buf[rcount - 1] & 0x20) == 0x20)
			{
			  printf("RDO ");
			}
		      if ((buf[rcount - 1] & 0x01) == 0x01)
			{
			  printf("RECEIVE ABORTED ");
			}
		      if ((buf[rcount - 1] & 0x04) == 0x04)
			{
			  printf("Receive Length Exceeded ");
			}
		      if ((buf[rcount - 1] & 0x10) == 0x10)
			{
			  printf("Receive Frame Lost ");
			}
		      printf("\n");
		      for (i = 0; i < 40; i++)
			{
			  printf("-");
			}
		      printf("\n");
		    }
		}




	    }
	}			//end of while true loop for writing commands
    }
  else
    {
//slave code
//listen for command, send response
      fcount = 0;
      fd = 0;
      SetRaw(fd);		//configure terminal to handle input without waiting for CR
      while (1)
	{
	  //if the user hit a key process it 
	  n = 0;
	  n = read(fd, &cbuf, 1);
	  if (n == 1)
	    {
	      if ((cbuf == 'q') || (cbuf == 'Q'))
		{
		  //user wants to quit
		  goto fini;
		}
	    }
//get a received frame from the FSCC
	  if ((rcount = read(fscc, buf, 4096)) == -1)
	    {
	      printf("read failed:%i\n", errno);
	      perror(NULL);
	    }
	  else
	    {
//frame received, make sure it is valid (and not a timeout which would be a "0" return)
	      if (rcount > 2)
		{
//decode our command and issue the response
		  if (buf[0] == 0x01)
		    {
		      //command #3 detected
		      buf[0] = 0x03;	// #3 response is a 3
		      wrsize = 1;
		    }
		  if (buf[0] == 0x55)
		    {
		      //command #1 detected
		      buf[0] = 0x01;	// #1 response is a 1
		      wrsize = 1;
		    }
		  else if (buf[0] == 0xaa)
		    {
		      //command #2 detected
		      buf[0] = 0x02;	// #2 response is a 2
		      wrsize = 1;
		    }
//physically send the response frame out the FSCC 
		  if ((wcount = write(fscc, buf, wrsize)) == -1)
		    {
		      printf("write failed:errno = %i\n", errno);
		      perror(NULL);
		    }
		  else
		    {
		      printf("response %x sent\n", buf[0]);
		    }
		}
	    }
	}
    }
fini:
//exit program on user hitting 'q'
  if (ioctl(fscc, FSCC_FLUSH_TX, NULL) == -1)
    perror(NULL);
  else
    printf("tx flushed\n");
  TermRestore(fd);		//set terminal processing back to "normal"
  close(fscc);			//finished with the FSCC
  free(buf);			//finished with the transfer buffer
  return 0;			//goodbye
}


