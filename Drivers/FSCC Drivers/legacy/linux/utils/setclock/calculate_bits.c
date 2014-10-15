/* $Id$ */
/*
calculate_bits
  Usermode program that calculates and prints the programming bits
  for the ics307-03 programmable clock generator

usage:
 calculate_bits frequency [ppm]

 The port can be any valid fscc port (0,1) 
 The frequency can be any in the range 100000 - 200000000

make -f calculate_bits.make

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
	if( (argc!=2) && (argc!=3) )
	{
		printf("usage:\n%s frequency [ppm]\n",argv[0]);
		exit(1);
	}
	desfreq = atol(argv[1]);
	if(argc==3)	desppm  = atol(argv[2]);
	else desppm = 2;
	for(i=19;i>=0;i--) progwords[i]=0;
	printf("setting clock to 	:%lu\n",desfreq);
	if(find_ics30703_solution(desfreq,desppm,&progwords[0])==0)
	 {
	 clock.freq = desfreq;
	 for(i=0;i<19;i++) clock.progword[i] = progwords[i];
	 printf("successful!\n");
	 }
	else
	 {
	 printf("failed! no solution found, increase ppm?\n");
	 }
	close(fscc);
	return 0;
}
