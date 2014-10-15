/* $Id */
/*
Copyright(c) 2007, Commtech, Inc.
readcutoff.c -- user mode function to set/clear the allow read cutoff function

usage:
 readcutoff port 0|1

 The port can be any valid escc port (0,1) 
 0 - disable the read cutoff function
 1 - enable the read cutoff function, when enabled the driver will return data as it arrives when the 
     number of received bytes equals the requested read size, which equals the n_rfsize_max setting.
     (ie read will complete when n_rfsize_max bytes have been received)
     This can be used to receive data when the data coming in has no boundaries or frame end markers
     (ie continuous receive, in transparent mode, or x-sync mode).

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

int main(int argc, char * argv[])
{
	char nbuf[80];
	int port;
	int fscc = -1;
	unsigned long readcutoff;
	if(argc<3)
	{
		printf("usage:\n%s port 0|1\n",argv[0]);
		exit(1);
	}
	port = atoi(argv[1]);
	readcutoff = atol(argv[2]);
	sprintf(nbuf,"/dev/fscc%u",port);
	fscc = open(nbuf,O_RDWR);
	if(fscc == -1)
	{
		printf("cannot open %s\n",nbuf);
		perror(NULL);
		exit(1);
	}
	printf("opened %s\r\n",nbuf);
	ioctl(fscc,FSCC_ALLOW_READ_CUTOFF,&readcutoff);
	if(readcutoff==1) printf("read cutoff enabled\n");
        else printf("read cutoff disabled\n");
	close(fscc);
	return 0;
}
