/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
setfeatures.c -- user mode function to set the onboard feature setting for a FSCC port


usage:
 setfeatures port [0|1] [0|1] [0|1] [0|1] [0|1]

 The port can be any valid fscc port (0,1)

parameter1 rx echo cancel control, 1==echo cancel ON, 0== OFF
parameter2 SD 485 control, 1== SD is RS-485, 0== SD is RS-422
parameter3 TT 485 control, 1== TT is RS-485, 0== TT is RS-422
parameter4 DTR/FST control, 1=DTR, 0=FST
parameter5 ASYNC mode control, 1==ASYNC mode enabled, 0 is SYNC mode enabled

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





int main(int argc, char * argv[])
{
	char nbuf[80];
	int port;
	int fscc; 
	unsigned long  desreg;
	unsigned long temp;
	unsigned long rxecho;
	unsigned long sd485;
	unsigned long tt485;
	unsigned long asyncmode;
	unsigned long dtrfst;

	if(argc<7) 
		{
		printf("usage:\r\n");
		printf("%s port [0|1] [0|1] [0|1] [0|1] [0|1]\r\n",argv[0]);
		printf("\r\n");
		printf("The port can be any valid fscc port (0,1)\r\n");
		printf("\r\n");
		printf("parameter1 rx echo cancel control, 1==echo cancel ON, 0== OFF\r\n");
		printf("parameter2 SD 485 control, 1== SD is RS-485, 0== SD is RS-422\r\n");
		printf("parameter3 TT 485 control, 1== TT is RS-485, 0== TT is RS-422\r\n");
		printf("parameter4 DTR/FST control, 1=DTR, 0=FST\r\n");
		printf("parameter5 ASYNC mode control, 1==ASYNC mode, 0== SYNC mode\r\n");
		printf("\r\n");
		exit(1);
		}

	port = atoi(argv[1]);
	rxecho = atol(argv[2]);
	sd485 = atol(argv[3]);
	tt485 = atol(argv[4]);
	dtrfst = atol(argv[5]);
	asyncmode = atol(argv[6]);

        sprintf(nbuf,"/dev/fscc%u",port);


fscc = open(nbuf,O_RDWR);
if(fscc == -1)
{
printf("cannot open %s\n",nbuf);
perror(NULL);
exit(1);
}
printf("opened %s\r\n",nbuf);

desreg = 0;
if(rxecho     == 1)		desreg|=0x01;
if(sd485      == 1)		desreg|=0x02;
if(tt485      == 1)		desreg|=0x04;
if(dtrfst     == 1)             desreg|=0x08;
if(asyncmode  == 1)	        desreg|=0x80000000;
printf("features:%8.8lx\r\n",desreg);
if(ioctl(fscc,FSCC_SET_FEATURES,&desreg)==-1) perror(NULL);
else
{
	if(rxecho     == 0)		printf("RX allways on\r\n");
	else					printf("RX echo	cancel ENABLED\r\n");
	if(sd485      == 0)		printf("SD is RS-422\r\n");
	else					printf("SD is RS-485\r\n");
	if(tt485      == 0)		printf("TT is RS-422\r\n");
	else					printf("TT is RS-485\r\n");
	if(dtrfst     == 0)             printf("DTR/FST is FST\r\n");
	else                                    printf("DTR/FST is DTR\r\n");
	if(asyncmode == 1)	    printf("ASYNC mode!\r\n");
	else					printf("SYNC mode!\r\n");
}

close(fscc);

	return 0;
}



/* $Id$ */
