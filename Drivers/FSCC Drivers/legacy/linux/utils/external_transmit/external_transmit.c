/* $Id */
/*
Copyright(c) 2007, Commtech, Inc.
external_transmit.c -- user mode function to write data out a port one frame per external signal

usage:
 external_transmit port blocksize type

 The port can be any valid fscc port (0,1) 
 The blocksize is the number of bytes to send per frame
 The type is: 	0 = random
 		1 = 0xaa
		2 = 0x00
		3 = 0x55
		4 = 0xff
		5 = ascii counter

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



void decode_status(unsigned long stat);
static struct termio save_term;


void SetRaw(int fd)
{
    struct termio s;
    //Get terminal modes.
    (void)ioctl(fd, TCGETA, &s);
    //Save modes and set certain variables dependent on modes.
    save_term = s;
    //Set the modes to the way we want them.
    s.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL);
    s.c_oflag |= (OPOST|ONLCR|TAB3);
    s.c_oflag &= ~(OCRNL|ONOCR|ONLRET);
    s.c_cc[VMIN] = 0;
    s.c_cc[VTIME] = 0;
    (void)ioctl(fd, TCSETAW, &s);
}

static void TermRestore(int fd)
{
    struct termio s;
    //Restore saved modes.
    s = save_term;
    (void)ioctl(fd, TCSETAW, &s);
}
	unsigned char RxBuf[32780];
	unsigned char TxBuf[32768];


int main(int argc, char * argv[])
{
int fd;
int i;
unsigned long status;
char nbuf[80];
int port;
FILE *fin;
int fscc = -1;
char *buf;
char cbuf;
unsigned rcount;
unsigned wcount;
unsigned long wtotal = 0;
int blocksz;
int type;
int n;
unsigned long passval[2];
unsigned long fcount;
long ltime;
int stime;

	ltime = time(NULL);
	stime = (unsigned int) ltime/2;
	srand(stime);
	
if(argc!=4)
{
printf("usage:\n%s port blocksize type\n",argv[0]);

printf("\tThe port can be any valid fscc port (0,1,etc)\n\tThe blocksize is the number of bytes to send per frame\n\tThe type is: 	\n\t0 = random\n\t1 = 0xaa\n\t2 = 0x00\n\t3 = 0x55\n\t4 = 0xff\n\t5 = ascii counter\n");
exit(1);
}

port = atoi(argv[1]);
sprintf(nbuf,"/dev/fscc%u",port);
blocksz=atoi(argv[2]);
if(blocksz==0)
{
printf("blocksize must be >0!\n");
exit(1);
}
type = atoi(argv[3]);
if((type==5)&&(blocksz<13)) blocksz=16;

buf = (unsigned char *) malloc(blocksz+16);
if(buf==NULL)
{
printf("cannot allcoate memory for buffer!\n");
exit(1);
}
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
else printf("%s ==> TX Flushed\n",nbuf);
passval[0] = 1;
if(ioctl(fscc,FSCC_EXTERNAL_TRANSMIT,&passval[0])==-1) perror(NULL);
else printf("%s ==> External transmit enabled\n",nbuf);
//depending on how big your blocks are, and your driver buffering setup you may want this:
//passval[0]=1;
//ioctl(fscc,FSCC_TRANSMIT_DELAY,&passval[0]);//delay physical writes until driver buffers are full

fcount=0;
	fd=0;
	SetRaw(fd);
	printf("Enter q to quit\n\n");
	
	while(1)
	{
	n=0;
//uncomment for pause between frames
//	while(n!=1)
//	{
		n=read(fd,&cbuf,1);
		if(n==1)
		{
			if((cbuf=='q')||(cbuf=='Q')) 
			{
				goto fini;
			}
		}
//	}

		if(type==0) for(i=0;i<blocksz;i++) buf[i]=rand();
		else if(type==1) for(i=0;i<blocksz;i++) buf[i] = 0xaa;
		else if(type==2) for(i=0;i<blocksz;i++) buf[i] = 0x00;
		else if(type==3) for(i=0;i<blocksz;i++) buf[i] = 0x55;
		else if(type==4) for(i=0;i<blocksz;i++) buf[i] = 0xff;
		else if(type==5) sprintf(buf,"%10.10u,%c,%c",fcount,0x00,0x00);
		


	if((wcount = write(fscc,buf,blocksz))==-1)
		{
		printf("write failed:errno = %i\n",errno);
		perror(NULL);
		}
		else
		{
		wtotal += wcount;
		fcount++;
		printf("wtotal:%lu\n",wtotal);
		}
	}

fini:

if(ioctl(fscc,FSCC_FLUSH_TX,NULL)==-1) perror(NULL);
else printf("tx flushed\n");
passval[0] = 0;
if(ioctl(fscc,FSCC_EXTERNAL_TRANSMIT,&passval[0])==-1) perror(NULL);
else printf("External transmit disabled\n");

TermRestore(fd);
close(fscc);
free(buf);
return 0;
}






