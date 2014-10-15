/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
setclock.c -- user mode function to set the programmable clock

usage:
 setclock port frequency

 The port can be any valid escc port (0,1) 
 The frequency can be any in the range 100000 - 200000000


*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <math.h>
#include <sys/poll.h>
#include "../fsccdrv.h"
int find_ics30703_solution(unsigned long freq,unsigned long ppm, unsigned char *progbytes);

int main(int argc, char * argv[])
{
	struct clockstruct clock;
	char nbuf[80];
	int port;
	int fscc = -1;
	unsigned long desfreq;
	unsigned long desppm;
	unsigned char progwords[20];
	int i;
	if(argc<3)
	{
		printf("usage:\n%s port frequency [ppm]\n",argv[0]);
		exit(1);
	}
	port = atoi(argv[1]);
	desfreq = atol(argv[2]);
	if(argc>3)	desppm  = atol(argv[3]);
	else desppm = 2;
	for(i=19;i>=0;i--) progwords[i]=0;
	sprintf(nbuf,"/dev/fscc%u",port);
	fscc = open(nbuf,O_RDWR);
	if(fscc == -1)
	{
		printf("cannot open %s\n",nbuf);
		perror(NULL);
		exit(1);
	}
	printf("opened %s\r\n",nbuf);
	printf("setting clock to 	:%lu\n",desfreq);
	if(find_ics30703_solution(desfreq,desppm,&progwords[0])==0)
	 {
	 clock.freq = desfreq;
	 for(i=0;i<19;i++) clock.progword[i] = progwords[i];
	 ioctl(fscc,FSCC_SET_FREQ,&clock);
	 printf("successful!\n");
	 }
	else
	 {
	 printf("failed! no solution found, increase ppm?\n");
	 }
	close(fscc);
	return 0;
}
