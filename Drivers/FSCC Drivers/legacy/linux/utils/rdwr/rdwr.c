// rdwr.c a simple loopback example
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <termio.h>
#include <sys/ioctl.h>
#include <malloc.h>
#include "../fsccdrv.h"

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

int main(int argc, char *argv[])
{
	int i,size,j,fd,n;
	char buf;
	int fd1,fd2;
	int hdlc;
	long rsz,wsz,rrsz;
	unsigned long fct;
	unsigned long long bct;
	char fname[40];
	long ltime;
	int stime;
	unsigned long command;
	unsigned long errors=0;
	unsigned long status;
	int use_random=0;
	int max=32768;
	if(argc<4)
	{
		printf("%s port1 port2 size [max]\n",argv[0]);
		printf("PORT1 0-3\tTransmit\nPORT2 0-3\tRecieve\n");
		exit(1);
	}
	// " main port1 port2 "
	sprintf(fname,"/dev/fscc%d",atol(argv[1]));	
	fd1 = open(fname,O_RDWR);
	if(strcmp(argv[1],argv[2])!=0)
	{
		sprintf(fname,"/dev/fscc%d",atol(argv[2]));	
		fd2 = open(fname,O_RDWR);
	}
	else fd2 = fd1;
	if(fd1==-1 || fd2==-1)
	{
		printf("Cannot open Devices!\n");
		exit(1);
	}
	
	fd=0;
	SetRaw(fd);
	printf("Enter q to quit\n\n");
	
	ltime = time(NULL);
	stime = (unsigned int) ltime/2;
	srand(stime);
		

	size = atol(argv[3]);
	if(size==0)
	{
	printf("using random sizes\n");
	use_random=1;
	}
	if(argc>4)
	{
	max = atol(argv[4]);
	if(max==0)max=1;
	}

	if(size==0) rrsz = max+2;
	else rrsz = size+2;
	/*
	RxBuf = (unsigned char *) malloc(rrsz*sizeof(unsigned char));
	if(RxBuf==NULL)
	{
	printf("memory allocation error (RxBuf)%d\n",rrsz);
	exit(1);
	}
	TxBuf = (unsigned char *) malloc(size*sizeof(unsigned char));
	if(TxBuf==NULL)
	{
	printf("memory allocation error (TxBuf)%d\n",size);
	free(RxBuf);
	exit(1);
	}
	printf("%p\n",RxBuf);
	printf("%p\n",TxBuf);
*/
	ioctl(fd2,FSCC_FLUSH_RX,0);
	ioctl(fd1,FSCC_FLUSH_TX,0);
	fct=0;
	bct=0;
	while(1)
	{
	n=0;
//uncomment for pause between frames
//	while(n!=1)
//	{
		n=read(fd,&buf,1);
		if(n==1)
		{
			if((buf=='q')||(buf=='Q')) 
			{
				goto fini;
			}
		}
//	}
if(use_random==1)
{
	size = rand()%max;
	if(size==0) size=1;
}
		for(j=0;j<size;j++)
		{
			TxBuf[j] = rand();
		}
		printf("WRITE[%10ld]: %6d   Press Q to Quit\n",fct,wsz=write(fd1,TxBuf,size));
/*		printf("paused");
		while(n!=1)
	{
		n=read(fd,&buf,1);
		if(n==1)
		{
			if((buf=='q')||(buf=='Q')) 
			{
				goto fini;
			}
		}
	}
*/
		printf("READ [%10ld]: %6d   Press Q to Quit\n",fct,rsz=read(fd2,RxBuf,rrsz));
		//process read/write data here...
			if(rsz<0) perror(NULL);
			if(rsz!=wsz+2) 
			{
				errors++;
				printf("Error, count mismatch\n");//hdlc version
			}
			for(j=0;j<rsz-2;j++)
			{
				if(RxBuf[j]!=TxBuf[j])
				{
					errors++;
					printf("data mismatch: %x != %x @ %d\n",RxBuf[j]&0xff,TxBuf[j]&0xff,j);
				}
			}
		fct++;
		bct+=size;
		if(errors) 
		{
			if(ioctl(fd1,FSCC_IMMEDIATE_STATUS,&status)==-1) perror(NULL);
			else printf("%s Immediate Status: %8.8lx\n",fname,status);
			goto fini;
		}
	
	}
fini:
	TermRestore(fd);
	close(fd1);
	close(fd2);
	printf("FrameCount : %ld\n",fct);
	printf("ByteCount  : %lld\n",bct);
	printf("Error count: %ld\n",errors);
//	printf("%p\n",RxBuf);
//	printf("%p\n",TxBuf);
//	free(RxBuf);
//	free(TxBuf);

	return 0;
}
