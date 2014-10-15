/* $Id */
/*
make as:
gcc -O2 -o commandresponse commandresponse.c -lm

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

//stuff for programming the ics30703
#define result_array_size 512

struct ResultStruct
{
  double target;
  double freq;
  double errorPPM;
  int VCO_Div;
  int refDiv;
  int outDiv;
  int failed;
};

struct IcpRsStruct
{
  double pdf;
  double nbw;
  double ratio;
  double df;
  unsigned long Rs;
  double icp;
  unsigned long icpnum;		//I have to use this in the switch statement because 8.75e-6 becomes 874
};

int GetICS30703Data(unsigned long desired, unsigned long ppm, struct ResultStruct *theOne, struct IcpRsStruct *theOther,
		    unsigned char *progdata);

int find_ics30703_solution(unsigned long freq, unsigned long ppm, unsigned char *progbytes);

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





int
find_ics30703_solution(unsigned long freq, unsigned long ppm, unsigned char *progbytes)
{
  int t;
  int i;
  unsigned long desiredppm;
  unsigned char progwords[20];
  struct ResultStruct solutiona;	//final results for ResultStruct data calculations
  struct IcpRsStruct solutionb;	//final results for IcpRsStruct data calculations

  printf("desired freq:%ld ppm:%ld\n", freq, ppm);
  desiredppm = ppm;

  memset(&solutiona, 0, sizeof(struct ResultStruct));
  memset(&solutionb, 0, sizeof(struct IcpRsStruct));
  t = GetICS30703Data(freq, desiredppm, &solutiona, &solutionb, &progwords[0]);
  switch (t)
    {
    case 0:
      printf("ICS30703: GetICS30703Data returned successfully.\n");
      printf("programming word:\n");
      for (i = 19; i >= 0; i--)
	printf("%x:", progwords[i]);
      printf("\n");
      break;
    case 1:
      printf("ICS30703: Rs case error\n");
      goto drop;
      break;
    case 2:
      printf("ICS30703: no solutions found, try increasing ppm\n");
      goto drop;
      break;
    case 3:
      printf("ICS30703: Table 1: Input Divider is out of rante.\n");
      goto drop;
      break;
    case 4:
      printf("ICS30703: Table 2: VCODivider is out of range.\n");
      goto drop;
      break;
    case 5:
      printf("ICS30703: Table 4: LoopFilterResistor is incorrect.\n");
      goto drop;
      break;
    case 6:
      printf("ICS30703: Table 3: Charge Pump Current is incorrect.\n");
      goto drop;
      break;
    case 7:
      printf("ICS30703: Table 5: OutputDividerOut1 is out of range.\n");
      goto drop;
      break;
    default:
      printf("ICS30703: Unknown error number.\n");
      goto drop;
    }

  /*
     printk("The One:\n");
     printk(" RD = %4i, ",solutiona.refDiv);
     printk("VD = %4i, ",solutiona.VCO_Div);
     printk("OD = %4i, ",solutiona.outDiv);
     printk("freq_MHz = %12.3f, ",solutiona.freq);
     printk(" error_PPM= %4.3f\n",solutiona.errorPPM);

     printk(" Rs=%5d, ",solutionb.Rs);
     printk("Icp=%6.2f, ",(solutionb.icp)*10e5);
     printk("pdf=%12.2f, ",solutionb.pdf);
     printk("nbw=%15.3f, ",solutionb.nbw);
     printk("ratio=%5.4f, ",solutionb.ratio);
     printk("df=%6.3f\n\n",solutionb.df);
   */

  /*              printk("ICS30703: Programming word is: \n  0x");
     for(t=20;t>0;t--)
     {
     printk("%2.2X ", progwords[t-1]);
     }
   */
  for (i = 0; i < 20; i++)
    progbytes[i] = progwords[i];
drop:

  if (t == 0)
    return 0;
  else
    return 1;
}


int
GetICS30703Data(unsigned long desired, unsigned long ppm, struct ResultStruct *theOne, struct IcpRsStruct *theOther,
		unsigned char *progdata)
{
  //      double inputfreq=18432000.0;
  double inputfreq = 24000000.0;

  unsigned long od = 0;		//Output Divider
  unsigned r = 0;
  unsigned v = 0;
  unsigned V_Divstart = 0;
  double freq = 0;
  unsigned maxR;
  unsigned minR;
  unsigned max_V = 2055;
  unsigned min_V = 12;
  double allowable_error;
  double freq_err;
  struct ResultStruct Results;
  unsigned long i, j;
  struct IcpRsStruct IRStruct;
  unsigned count;
  unsigned long Rs;
  double rule1, rule2;
  int tempint;

  int InputDivider = 0;
  int VCODivider = 0;
  unsigned long ChargePumpCurrent = 0;
  unsigned long LoopFilterResistor = 0;
  unsigned long OutputDividerOut1 = 0;
  unsigned long temp = 0;
  unsigned long requestedppm;

  requestedppm = ppm;

  if (inputfreq == 18432000.0)
    {
      maxR = 921;
      minR = 1;
    }
  else if (inputfreq == 24000000.0)
    {
      maxR = 1200;
      minR = 1;
    }

  ppm = 0;
increaseppm:
  printf("ICS30703: ppm = %ld\n", ppm);
  allowable_error = ppm * desired / 1e6;	// * 1e6

  for (r = minR; r <= maxR; r++)
    {
      rule2 = inputfreq / (double) r;
      if ((rule2 < 20000.0) || (rule2 > 100000000.0))
	{
	  //                      printk("Rule2(r=%d): 20,000<%f<100000000\n",r,rule2);
	  continue;		//next r
	}

      od = 8232;
      while (od > 1)
	{
	  //set starting VCO setting with output freq just below target
	  V_Divstart = (int) (((desired - (allowable_error)) * r * od) / (inputfreq));

	  //check if starting VCO setting too low
	  if (V_Divstart < min_V)
	    V_Divstart = min_V;

	  //check if starting VCO setting too high
	  else if (V_Divstart > max_V)
	    V_Divstart = max_V;

			/** Loop thru VCO divide settings**/
	  //Loop through all VCO divider ratios
	  for (v = V_Divstart; v <= max_V; v++)	//Check all Vco divider settings
	    {
	      rule1 = (inputfreq * ((double) v / (double) r));

	      if (od == 2)
		{
		  if ((rule1 < 90000000.0) || (rule1 > 540000000.0))
		    {
		      continue;	//next VCO_Div
		    }
		}
	      else if (od == 3)
		{
		  if ((rule1 < 90000000.0) || (rule1 > 720000000.0))
		    {
		      continue;	//next VCO_Div
		    }
		}
	      else if ((od >= 38) && (od <= 1029))
		{
		  if ((rule1 < 90000000.0) || (rule1 > 570000000.0))
		    {
		      continue;	//next VCO_Div
		    }
		}
	      else
		{
		  if ((rule1 < 90000000.0) || (rule1 > 730000000.0))
		    {
		      continue;	//next VCO_Div
		    }
		}

	      freq = (inputfreq * ((double) v / ((double) r * (double) od)));
	      freq_err = fabs(freq - desired);	//Hz

	      if ((freq_err) > allowable_error)
		{
		  continue;	//next VCO_Div
		}
	      else if ((freq_err) <= allowable_error)
		{
		  count = 0;
		  for (i = 0; i < 4; i++)
		    {
		      switch (i)
			{
			case 0:
			  Rs = 64000;
			  break;
			case 1:
			  Rs = 52000;
			  break;
			case 2:
			  Rs = 16000;
			  break;
			case 3:
			  Rs = 4000;
			  break;
			default:
			  return 1;
			}

		      for (j = 0; j < 20; j++)
			{
			  IRStruct.Rs = Rs;
			  switch (j)
			    {
			    case 0:
			      IRStruct.icp = 1.25e-6;
			      IRStruct.icpnum = 125;
			      break;
			    case 1:
			      IRStruct.icp = 2.5e-6;
			      IRStruct.icpnum = 250;
			      break;
			    case 2:
			      IRStruct.icp = 3.75e-6;
			      IRStruct.icpnum = 375;
			      break;
			    case 3:
			      IRStruct.icp = 5.0e-6;
			      IRStruct.icpnum = 500;
			      break;
			    case 4:
			      IRStruct.icp = 6.25e-6;
			      IRStruct.icpnum = 625;
			      break;
			    case 5:
			      IRStruct.icp = 7.5e-6;
			      IRStruct.icpnum = 750;
			      break;
			    case 6:
			      IRStruct.icp = 8.75e-6;
			      IRStruct.icpnum = 875;
			      break;
			    case 7:
			      IRStruct.icp = 10.0e-6;
			      IRStruct.icpnum = 1000;
			      break;
			    case 8:
			      IRStruct.icp = 11.25e-6;
			      IRStruct.icpnum = 1125;
			      break;
			    case 9:
			      IRStruct.icp = 12.5e-6;
			      IRStruct.icpnum = 1250;
			      break;
			    case 10:
			      IRStruct.icp = 15.0e-6;
			      IRStruct.icpnum = 1500;
			      break;
			    case 11:
			      IRStruct.icp = 17.5e-6;
			      IRStruct.icpnum = 1750;
			      break;
			    case 12:
			      IRStruct.icp = 18.75e-6;
			      IRStruct.icpnum = 1875;
			      break;
			    case 13:
			      IRStruct.icp = 20.0e-6;
			      IRStruct.icpnum = 2000;
			      break;
			    case 14:
			      IRStruct.icp = 22.5e-6;
			      IRStruct.icpnum = 2250;
			      break;
			    case 15:
			      IRStruct.icp = 25.0e-6;
			      IRStruct.icpnum = 2500;
			      break;
			    case 16:
			      IRStruct.icp = 26.25e-6;
			      IRStruct.icpnum = 2625;
			      break;
			    case 17:
			      IRStruct.icp = 30.0e-6;
			      IRStruct.icpnum = 3000;
			      break;
			    case 18:
			      IRStruct.icp = 35.0e-6;
			      IRStruct.icpnum = 3500;
			      break;
			    case 19:
			      IRStruct.icp = 40.0e-6;
			      IRStruct.icpnum = 4000;
			      break;
			    default:
			      printf("ICS30703: switch(j:icp) - You shouldn't get here! %ld\n", j);
			    }	//end switch(j)
			  //                                                      printf("Rs=%5d ",IRStruct.Rs);
			  //                                                      printf("Icp=%2.2f ",IRStruct.icp*10e5);

			  IRStruct.pdf = (inputfreq / (double) r);
			  //                                                      printf("pdf=%12.2f ",IRStruct.pdf);

			  IRStruct.nbw =
			    (((double) IRStruct.Rs * IRStruct.icp * 310.0e6) / (2.0 * 3.14159 * (double) v));
			  //                                                      printf("nbw=%15.3f ",IRStruct.nbw);

			  IRStruct.ratio = (IRStruct.pdf / IRStruct.nbw);

			  tempint = (int) (IRStruct.ratio * 10.0);
			  if ((IRStruct.ratio * 10.0) - tempint >= 0.0)
			    tempint++;
			  IRStruct.ratio = (double) tempint / 10.0;

			  //                                                      IRStruct.ratio = ceil(IRStruct.ratio*10.0);     //these two statements make the
			  //                                                      IRStruct.ratio = IRStruct.ratio/10.0;           //ratio a little nicer to compare

			  //                                                      printf("ratio=%2.4f ",IRStruct.ratio);

			  IRStruct.df = (((double) IRStruct.Rs / 2) * (sqrt(((IRStruct.icp * 0.093) / (double) v))));
			  //                                                      printf("ndf=%12.3f\n",IRStruct.df);

			  count++;
			  if ((IRStruct.ratio > 30) || (IRStruct.ratio < 7) || (IRStruct.df > 2.0)
			      || (IRStruct.df < 0.2))
			    {
			      continue;
			    }
			  else
			    {
			      Results.target = desired;
			      Results.freq = freq;
			      Results.errorPPM = freq_err / desired * 1.0e6;
			      Results.VCO_Div = v;
			      Results.refDiv = r;
			      Results.outDiv = od;
			      Results.failed = 1;
			      goto finished;
			    }
			}	//end for(j=0;j<20;j++)
		    }		//end for(i=0;i<4;i++)
		}
	    }			//end of for( v = V_Divstart; v < max_V; v++ )

	  if (od <= 1030)
	    od--;
	  else if (od <= 2060)
	    od = od - 2;
	  else if (od <= 4120)
	    od = od - 4;
	  else
	    od = od - 8;

	}			//end of while(od <= 8232)
    }				//end of for( r = maxR, *saved_result_num = 0; r >= minR; r-- )

  ppm++;
  if (ppm > requestedppm)
    {
      return 2;
    }
  else
    {
      //              printk("ICS30703: increasing ppm to %d\n",ppm);
      goto increaseppm;
    }

finished:

  memcpy(theOne, &Results, sizeof(struct ResultStruct));

  memcpy(theOther, &IRStruct, sizeof(struct IcpRsStruct));

  printf("ICS30703: Best result is \n");
  printf("\tRD = %4i,", Results.refDiv);
  printf(" VD = %4i,", Results.VCO_Div);
  printf(" OD = %4i,", Results.outDiv);
  printf(" freq_Hz = %ld,\n", (unsigned long) Results.freq);

  printf("\tRs = %5ld, ", IRStruct.Rs);
  printf("Icp = %ld, ", (unsigned long) (IRStruct.icp * 1e6));
  //      printk("pdf = %d, ",(ULONG)IRStruct.pdf);
  //      printk("nbw = %d, ",(ULONG)IRStruct.nbw);
  printf("ratio = %ld, ", (unsigned long) IRStruct.ratio);
  printf("df = %ld\n", (unsigned long) IRStruct.df * 1000);

  /*
     first, choose the best dividers (V, R, and OD) with

     1st key best accuracy
     2nd key lowest reference divide
     3rd key highest VCO frequency (OD)

     then choose the best loop filter with

     1st key best PDF/NBW ratio (between 7 and 30, 15 is optimal)
     2nd key best damping factor (between 0.2 and 2, 0.7 is optimal)
   */
  /* this is 1MHz
     progdata[19]=0xff;
     progdata[18]=0xff;
     progdata[17]=0xff;
     progdata[16]=0xf0;
     progdata[15]=0x00;
     progdata[14]=0x01;
     progdata[13]=0x43;
     progdata[12]=0x1a;
     progdata[11]=0x9c;
     progdata[10]=0x00;
     progdata[9]=0x00;
     progdata[8]=0x00;
     progdata[7]=0x00;
     progdata[6]=0x00;
     progdata[5]=0x00;
     progdata[4]=0x00;
     progdata[3]=0x00;
     progdata[2]=0x0c;
     progdata[1]=0xdf;
     progdata[0]=0xee;
     goto doitnow;
   */
  /* 10 MHz
     progdata[19]=0xff;
     progdata[18]=0xff;
     progdata[17]=0xff;
     progdata[16]=0x00;
     progdata[15]=0x80;
     progdata[14]=0x01;
     progdata[13]=0x00;
     progdata[12]=0x66;
     progdata[11]=0x38;
     progdata[10]=0x00;
     progdata[9]=0x00;
     progdata[8]=0x00;
     progdata[7]=0x00;
     progdata[6]=0x00;
     progdata[5]=0x00;
     progdata[4]=0x00;
     progdata[3]=0x00;
     progdata[2]=0x07;
     progdata[1]=0x20;
     progdata[0]=0x12;
     goto doitnow;
   */

  progdata[19] = 0xff;
  progdata[18] = 0xff;
  progdata[17] = 0xff;
  progdata[16] = 0x00;
  progdata[15] = 0x04;		//change to xtal, as per.(now overridden in the driver)
  progdata[14] = 0x00;
  progdata[13] = 0x00;
  progdata[12] = 0x00;
  progdata[11] = 0x00;
  progdata[10] = 0x00;
  progdata[9] = 0x00;
  progdata[8] = 0x00;
  progdata[7] = 0x00;
  progdata[6] = 0x00;
  progdata[5] = 0x00;
  progdata[4] = 0x00;
  progdata[3] = 0x00;
  progdata[2] = 0x00;
  progdata[1] = 0x00;
  progdata[0] = 0x00;

  //      progdata[16]|=0x02;     //enable CLK3
  //      progdata[15]&=0xef;     //CLK3 source select: 1=CLK1, 0=CLK1 before OD 
  //      progdata[15]|=0x08;     //CLK2 source select: 1=CLK1, 0=CLK1 before OD
  //      progdata[15]|=0x40;     //reference source is: 1=crystal, 0=clock
  progdata[14] |= 0x01;		//1=Power up, 0=power down feedback counter, charge pump and VCO
  //      progdata[13]|=0x80;     //enable CLK2
  progdata[13] |= 0x40;		//enable CLK1

  InputDivider = theOne->refDiv;
  VCODivider = theOne->VCO_Div;
  ChargePumpCurrent = theOther->icpnum;
  LoopFilterResistor = theOther->Rs;
  OutputDividerOut1 = theOne->outDiv;

  //InputDivider=2;
  //VCODivider=60;
  //OutputDividerOut1 = 45;
  //LoopFilterResistor=16000;
  //ChargePumpCurrent=3500;

  /* Table 1: Input Divider */

  if ((InputDivider == 1) || (InputDivider == 2))
    {
      switch (InputDivider)
	{
	case 1:
	  progdata[0] &= 0xFC;
	  progdata[1] &= 0xF0;
	  break;
	case 2:
	  progdata[0] &= 0xFC;
	  progdata[0] |= 0x01;
	  progdata[1] &= 0xF0;
	  break;
	}
      //      printf("1 0x%2.2X,0x%2.2X\n",progdata[1],progdata[0]);
    }
  else if ((InputDivider >= 3) && (InputDivider <= 17))
    {
      temp = ~(InputDivider - 2);
      temp = (temp << 2);
      progdata[0] = (unsigned char) temp & 0xff;

      progdata[0] &= 0x3e;	//set bit 0 to a 0
      progdata[0] |= 0x02;	//set bit 1 to a 1

      //              printf("2 0x%2.2X,0x%2.2X\n",progdata[1],progdata[0]);
    }
  else if ((InputDivider >= 18) && (InputDivider <= 2055))
    {
      temp = InputDivider - 8;
      temp = (temp << 2);
      progdata[0] = (unsigned char) temp & 0xff;
      progdata[1] = (unsigned char) ((temp >> 8) & 0xff);

      progdata[0] |= 0x03;	//set bit 0 and 1 to a 1

      //              printf("3 0x%2.2X,0x%2.2X\n",progdata[1],progdata[0]);

    }
  else
    return 3;

  /* Table 2 VCO Divider */

  if ((VCODivider >= 12) && (VCODivider <= 2055))
    {
      temp = VCODivider - 8;
      temp = (temp << 5);
      progdata[1] |= temp & 0xff;
      progdata[2] |= ((temp >> 8) & 0xff);
      //              printf("4 0x%2.2X,0x%2.2X\n",progdata[2],progdata[1]);
    }
  else
    return 4;

  /* Table 4 Loop Filter Resistor */

  switch (LoopFilterResistor)
    {
    case 64000:
      progdata[11] &= 0xf9;	//bit 89 and 90 = 0
      break;
    case 52000:
      progdata[11] &= 0xf9;	//bit 89 = 0
      progdata[11] |= 0x04;	//bit 90 = 1
      break;
    case 16000:
      progdata[11] &= 0xf9;	//bit 90 = 0
      progdata[11] |= 0x02;	//bit 89 = 1
      break;
    case 4000:
      progdata[11] |= 0x06;	//bit 89 and 90 = 1
      break;
    default:
      return 5;
    }
  //      printf("5 0x%2.2X\n",progdata[11]);

  /* Table 3 Charge Pump Current */

  switch (ChargePumpCurrent)
    {
    case 125:
      progdata[11] |= 0x38;
      progdata[15] &= 0x7f;
      progdata[16] &= 0xfe;
      //              printf("125\n");
      break;

    case 250:
      progdata[11] |= 0x38;
      progdata[15] |= 0x80;
      progdata[16] &= 0xfe;
      break;

    case 375:
      progdata[11] |= 0x38;
      progdata[15] &= 0x7f;
      progdata[16] |= 0x01;
      break;

    case 500:
      progdata[11] |= 0x38;
      progdata[15] |= 0x80;
      progdata[16] |= 0x01;
      break;

    case 625:
      progdata[11] |= 0x18;
      progdata[11] &= 0xdf;
      progdata[15] &= 0x7f;
      progdata[16] &= 0xfe;
      break;

    case 750:
      progdata[11] |= 0x10;
      progdata[11] &= 0xd7;
      progdata[15] &= 0x7f;
      progdata[16] &= 0xfe;
      break;

    case 875:
      progdata[11] |= 0x08;
      progdata[11] &= 0xcf;
      progdata[15] &= 0x7f;
      progdata[16] &= 0xfe;
      break;

    case 1000:
      progdata[11] &= 0xc7;
      progdata[15] &= 0x7f;
      progdata[16] &= 0xfe;
      break;

    case 1125:
      progdata[11] |= 0x28;
      progdata[11] &= 0xef;
      progdata[15] &= 0x7f;
      progdata[16] |= 0x01;
      break;

    case 1250:
      progdata[11] |= 0x18;
      progdata[11] &= 0xdf;
      progdata[15] |= 0x80;
      progdata[16] &= 0xfe;
      break;

    case 1500:
      progdata[11] |= 0x28;
      progdata[11] &= 0xef;
      progdata[15] |= 0x80;
      progdata[16] |= 0x01;
      break;

    case 1750:
      progdata[11] |= 0x08;
      progdata[11] &= 0xcf;
      progdata[15] |= 0x80;
      progdata[16] &= 0xfe;
      break;

    case 1875:
      progdata[11] |= 0x18;
      progdata[11] &= 0xdf;
      progdata[15] &= 0x7f;
      progdata[16] |= 0x01;
      break;

    case 2000:
      progdata[11] &= 0xc7;
      progdata[15] |= 0x80;
      progdata[16] &= 0xfe;
      break;

    case 2250:
      progdata[11] |= 0x10;
      progdata[15] &= 0x7f;
      progdata[16] |= 0x01;
      break;

    case 2500:
      progdata[11] |= 0x18;
      progdata[11] &= 0xdf;
      progdata[15] |= 0x80;
      progdata[16] |= 0x01;
      break;

    case 2625:
      progdata[11] |= 0x08;
      progdata[11] &= 0xcf;
      progdata[15] &= 0x7f;
      progdata[16] |= 0x01;
      break;

    case 3000:
      progdata[11] &= 0xc7;
      progdata[15] &= 0x7f;
      progdata[16] |= 0x01;
      break;

    case 3500:
      progdata[11] |= 0x08;
      progdata[11] &= 0xcf;
      progdata[15] |= 0x80;
      progdata[16] |= 0x01;
      break;

    case 4000:
      progdata[11] &= 0xc7;
      progdata[15] |= 0x80;
      progdata[16] |= 0x01;
      break;

    default:
      return 6;
    }				//end switch(j)
  //      printf("6 0x%2.2X, 0x%2.2X, 0x%2.2X\n",progdata[16],progdata[15],progdata[11]);

  /* Table 5 Output Divider for Output 1 */
  //OutputDividerOut1=38;
  if ((OutputDividerOut1 >= 2) && (OutputDividerOut1 <= 8232))
    {
      switch (OutputDividerOut1)
	{
	case 2:
	  progdata[11] &= 0x7f;
	  progdata[12] &= 0x00;
	  progdata[13] &= 0xc0;
	  break;

	case 3:
	  progdata[11] |= 0x80;
	  progdata[12] &= 0x00;
	  progdata[13] &= 0xc0;
	  break;

	case 4:
	  progdata[11] &= 0x7f;
	  progdata[12] |= 0x04;
	  progdata[13] &= 0xc0;
	  break;

	case 5:
	  progdata[11] &= 0x7f;
	  progdata[12] |= 0x01;
	  progdata[13] &= 0xc0;
	  break;

	case 6:
	  progdata[11] |= 0x80;
	  progdata[12] |= 0x04;
	  progdata[13] &= 0xc0;
	  break;

	case 7:
	  progdata[11] |= 0x80;
	  progdata[12] |= 0x01;
	  progdata[13] &= 0xc0;
	  break;

	case 11:
	  progdata[11] |= 0x80;
	  progdata[12] |= 0x09;
	  progdata[13] &= 0xc0;
	  break;

	case 9:
	  progdata[11] |= 0x80;
	  progdata[12] |= 0x05;
	  progdata[13] &= 0xc0;
	  break;

	case 13:
	  progdata[11] |= 0x80;
	  progdata[12] |= 0x0d;
	  progdata[13] &= 0xc0;
	  break;

	case 8:
	case 10:
	case 12:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
	  temp = ~(OutputDividerOut1 - 6);
	  temp = (temp << 2);
	  progdata[12] = (unsigned char) temp & 0x7f;

	  progdata[11] &= 0x7f;
	  progdata[12] &= 0xfe;
	  progdata[12] |= 0x02;
	  progdata[13] &= 0xc0;
	  break;

	default:

	  for (i = 0; i < 512; i++)
	    {
	      if (OutputDividerOut1 == ((((i + 3) * 2) + 0) * (1)))
		{
		  //                                      printf("1 x=%d, y=0, z=0\n",i);
		  printf("outputdivider1\n");
		  temp = (i << 5);
		  progdata[12] |= (temp & 0xff);
		  progdata[13] |= (temp >> 8) & 0xff;

		  progdata[12] &= 0xe7;
		  progdata[12] |= 0x04;
		  break;
		}

	      else if (OutputDividerOut1 == ((((i + 3) * 2) + 0) * (2)))
		{
		  //                                      printf("2 x=%d, y=0, z=1\n",i);
		  temp = (i << 5);
		  progdata[12] |= (temp & 0xff);
		  progdata[13] |= (temp >> 8) & 0xff;

		  progdata[12] &= 0xef;
		  progdata[12] |= 0x0c;
		  break;
		}

	      else if (OutputDividerOut1 == ((((i + 3) * 2) + 0) * (4)))
		{
		  //                                      printf("3 x=%d, y=0, z=2\n",i);
		  temp = (i << 5);
		  progdata[12] |= (temp & 0xff);
		  progdata[13] |= (temp >> 8) & 0xff;

		  progdata[12] &= 0xf7;
		  progdata[12] |= 0x14;
		  break;
		}

	      else if (OutputDividerOut1 == ((((i + 3) * 2) + 0) * (8)))
		{
		  //                                      printf("4 x=%d, y=0, z=3\n",i);
		  temp = (i << 5);
		  progdata[12] |= (temp & 0xff);
		  progdata[13] |= (temp >> 8) & 0xff;

		  progdata[12] |= 0x1c;
		  break;
		}

	      else if (OutputDividerOut1 == ((((i + 3) * 2) + 1) * (1)))
		{
		  //                                      printf("5 x=%d, y=1, z=0\n",i);
		  printf("outputdivider5\n");
		  temp = (i << 5);
		  progdata[12] |= (temp & 0xff);
		  progdata[13] |= (temp >> 8) & 0xff;

		  progdata[12] &= 0xe3;
		  break;
		}

	      else if (OutputDividerOut1 == ((((i + 3) * 2) + 1) * (2)))
		{
		  //                                      printf("6 x=%d, y=1, z=1\n",i);
		  temp = (i << 5);
		  progdata[12] |= (temp & 0xff);
		  progdata[13] |= (temp >> 8) & 0xff;

		  progdata[12] &= 0xeb;	//power of 1
		  progdata[12] |= 0x08;
		  break;
		}

	      else if (OutputDividerOut1 == ((((i + 3) * 2) + 1) * (4)))
		{
		  //                                      printf("7 x=%d, y=1, z=2\n",i);
		  temp = (i << 5);
		  progdata[12] |= (temp & 0xff);
		  progdata[13] |= (temp >> 8) & 0xff;

		  progdata[12] &= 0xf7;
		  progdata[12] |= 0x10;
		  break;
		}

	      else if (OutputDividerOut1 == ((((i + 3) * 2) + 1) * (8)))
		{
		  //                                      printf("8 x=%d, y=1, z=3\n",i);
		  temp = (i << 5);
		  progdata[12] |= (temp & 0xff);
		  progdata[13] |= (temp >> 8) & 0xff;

		  progdata[12] &= 0xfb;
		  progdata[12] |= 0x18;
		  break;
		}
	    }
	  progdata[11] |= 0x80;	//1
	  progdata[12] &= 0xfe;	//0
	  progdata[12] |= 0x02;	//1
	}
      //              printf("0x%2.2x, 0x%2.2x, 0x%2.2x\n\n",progdata[13]&0x3f, progdata[12], progdata[11]&0x80);
    }
  else
    return 7;
  //doitnow:

  /*      progdata[15]|=0x03;     //this will set
     progdata[14]|=0xc0;  //the OD of clock 3
     progdata[11]&=0xbf;  //to 2
   */
  return 0;

}				//end of GetICS30703Bits
