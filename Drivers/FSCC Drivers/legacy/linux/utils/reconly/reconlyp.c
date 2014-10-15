/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
reconlyp.c -- user mode function to receive data

usage:
 reconlyp port 

 The port can be any valid fscc port (0,1) 
 

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

void decode_status(unsigned long stat);

int main(int argc, char * argv[])
{
char buf[4096];
int fscc = -1;
unsigned j;
char nbuf[80];
unsigned rcount;
unsigned port;
long i;

if(argc<2)
{

printf("usage:\n");
printf("%s port\n",argv[0]);
exit(1);
}

port = atoi(argv[1]);
sprintf(nbuf,"/dev/fscc%u",port);

fscc = open(nbuf,O_RDWR);
if(fscc == -1)
	{
	printf("cannot open %s\n",nbuf);
	perror(NULL);
	exit(1);
	}
printf("opened %s\n",nbuf);

if(ioctl(fscc,FSCC_FLUSH_RX,NULL)==-1) perror(NULL);

do
{
i=i++;
if((rcount = read(fscc,buf,4096))==-1)
	{
	printf("read failed:errno = %i\n",errno);
	perror(NULL);
	}
else if(rcount>2)
{
for(j=0;j<40;j++)printf("-");
printf("\n");
printf("%ld Read returned:%i\n",i,rcount);
for(j=0;j<rcount-2;j++) printf("%c",buf[j]);
printf("\n");
if((buf[rcount-2]&0x02)==0x02)printf("CRC FAILED\n");
else printf("CRC Passed\n");
}
else printf("%ld Read returend:%i\n",i,rcount);
}while(1);


close(fscc);
exit(1);
return 0;
}



