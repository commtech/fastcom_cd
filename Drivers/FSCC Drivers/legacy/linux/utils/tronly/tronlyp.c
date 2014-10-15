/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
tronlyp.c -- user mode function to write data out a port

usage:
 tronlyp port file [delay] [loop]

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
unsigned long passval;
unsigned long status;
char nbuf[80];
int port;
FILE *fin;
int fscc = -1;
char buf[4096];
unsigned rcount;
unsigned wcount;
unsigned long wtotal = 0;
unsigned loop = 0;
unsigned delay = 0;


if(argc<3)
{
printf("usage:\n%s port file [delay] [loop]\n",argv[0]);
exit(1);
}
fin = fopen(argv[2],"rb");
if(fin==NULL)
{
printf("cannot open input file :%s\n",argv[2]);
exit(1);
}
if(argc>3) delay = atoi(argv[3]);
if(delay<0) delay = 0;

if(argc>4) loop = atoi(argv[4]);
if(loop<0) loop = 0;


port = atoi(argv[1]);
sprintf(nbuf,"/dev/fscc%u",port);

fscc = open(nbuf,O_RDWR);
if(fscc == -1)
{
printf("cannot open %s\n",nbuf);
perror(NULL);
exit(1);
}
printf("opened %s\r\n",nbuf);
//lets start clean

if(ioctl(fscc,FSCC_FLUSH_TX,NULL)==-1) perror(NULL);
else printf("tx flushed\n");


do
{
while((rcount=fread(buf,1,1024,fin))!=0)
	{
		if((wcount = write(fscc,buf,rcount))==-1)
		{
		printf("write failed:errno = %i\n",errno);
		perror(NULL);
		}
		else
		{
		wtotal += wcount;
		printf("wtotal:%lu\n",wtotal);
		}
	}
loop--;
rewind(fin);
sleep(delay);
}while(loop > 0);

printf("\n\n");
printf("paused\n");
getchar();

fclose(fin);
close(fscc);
return 0;
}






