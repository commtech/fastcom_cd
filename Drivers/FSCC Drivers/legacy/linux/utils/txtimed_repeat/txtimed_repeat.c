/* $Id */
/*
Copyright(c) 2007, Commtech, Inc.
timed_transmit.c -- user mode function to write data out a port one frame per timer interrupt

usage:
 timed_transmit port blocksize type

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
#include <stdint.h>
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
int blocksz;
int n;
uint32_t passval[2];
long ltime;
int stime;

ltime = time(NULL);
stime = (unsigned int) ltime/2;
srand(stime);
	
if(argc<2)
{
	printf("usage:\n%s port blocksize\n",argv[0]);
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


buf = (unsigned char *) malloc(blocksz+16);
if(buf==NULL)
{
	printf("cannot allcoate memory for buffer!\n");
	exit(1);
}
fscc = open(nbuf,O_RDWR);
if(fscc == -1)
{
	printf("Cannot open %s\n",nbuf);
	perror(NULL);
	exit(1);
}
printf("opened %s\r\n",nbuf);

if(ioctl(fscc,FSCC_FLUSH_TX,NULL)==-1) perror(NULL);
else printf("%s ==> TX Flushed\n",nbuf);

passval[0] = 1;
if(ioctl(fscc,FSCC_TIMED_TRANSMIT,&passval[0])==-1) perror(NULL);
else printf("%s ==> Timed transmit enabled\n",nbuf);

passval[0] = 1;
if(ioctl(fscc,FSCC_TRANSMIT_REPEAT,&passval[0])==-1) perror(NULL);
else printf("%s ==> Repeat transmit enabled\n",nbuf);

passval[0] = 0x48;
passval[1] = 0x270f8;
if(ioctl(fscc,FSCC_WRITE_REGISTER,&passval[0])==-1) perror(NULL);
else printf("%s ==> Timer set\n",nbuf);

passval[0] = 0x14;
passval[1] = 0x00000001;
if(ioctl(fscc,FSCC_WRITE_REGISTER,&passval[0])==-1) perror(NULL);
else printf("%s ==> Timer started\n",nbuf);



for(i=0;i<blocksz;i++) buf[i]=rand();
if((write(fscc,buf,blocksz))==-1)
{
	printf("write failed:errno = %i\n",errno);
	perror(NULL);
}
else
	printf("Write successful!\n");

SetRaw(fd);
printf("Enter q to exit\n\n");
	
while(1)
{
	n=0;
	n=read(fd,&cbuf,1);
	if(n==1)
	{
		if((cbuf=='q')||(cbuf=='Q')) 
		{
			goto fini;
		}
	}
}

fini:
if(ioctl(fscc,FSCC_FLUSH_TX,NULL)==-1) perror(NULL);
else printf("tx flushed\n");
passval[0] = 0;
if(ioctl(fscc,FSCC_TRANSMIT_REPEAT,&passval[0])==-1) perror(NULL);
else printf("%s ==> Repeat transmit disabled\n",nbuf);
passval[0] = 0;
if(ioctl(fscc,FSCC_TIMED_TRANSMIT,&passval[0])==-1) perror(NULL);
else printf("Timed transmit disabled\n");
passval[0]=0x14;
passval[1]=0x00000002;
if(ioctl(fscc,FSCC_WRITE_REGISTER,&passval[0])==-1) perror(NULL);
else printf("%s ==> Timer stopped\n",nbuf);

TermRestore(fd);
close(fscc);
free(buf);
return 0;
}






