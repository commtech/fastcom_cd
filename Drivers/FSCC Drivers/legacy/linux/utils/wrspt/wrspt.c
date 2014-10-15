/*
wrspt.c -- a user mode program to time writes of a given size/ammount 

 usage:
  wrspt port size count


*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <malloc.h>
#include <stdint.h>
#include <asm/msr.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include "../fsccdrv.h"
#define ULONG unsigned long
#define DWORD unsigned long

int main(int argc, char *argv[])
{
	int wDevice;
	ULONG t;
	DWORD nobyteswritten;
	char *tdata;
	ULONG size;
	int j,x,error,tosend;
	ULONG totalsent;
	ULONG totalread;
	ULONG totalerror;
	ULONG loop;
    char devname[25];
	FILE *fout=NULL;
	ULONG askedfor;
	unsigned long bufparam[4];	
	uint64_t t1,t2,tickspersecond;
	int i;
	unsigned long count;
	unsigned long status;
	unsigned long passval;

tickspersecond=0;
for(i=0;i<2;i++)
{	
rdtscll(t1);
sleep(1);
rdtscll(t2);
tickspersecond+=(t2-t1);
}
tickspersecond=(tickspersecond)/2;
//tickspersecond=2400000000.0;
printf("tps:%f\n",(double)tickspersecond);
if(argc<4)
{
printf("usage:\n");
printf("%s port blocksize count\n",argv[0]);
exit(1);
}

askedfor = atol(argv[3]);
if(askedfor==0)
{
	printf("count must be nonzero\n");
}
srand( (unsigned)time( NULL ) );
size = atol(argv[2]);
if(size==0)
{
printf("block size must be nonzero\n");
exit(1);
}

    sprintf(devname,"/dev/fscc%d",atoi(argv[1]));
	printf("devicename:%s\n",devname);
	printf("blocksize:%lu\n",size);
	printf("total requested:%d\n",askedfor);
	wDevice = open(devname,O_RDWR);
    if (wDevice == -1)
	{
		//for some reason the driver won't load or isn't loaded
		printf ("Can't open %s\n",devname);
		exit(1);
		//abort and leave here!!!
	}


tdata = (char*)malloc(size+1);
if(tdata==NULL)
{
printf("unable to allocate data buffer\n");
exit(1);
}
	totalerror=0;
	totalsent=0;
	totalread=0;
	loop=0;
passval=1;	
if(ioctl(wDevice,FSCC_TRANSMIT_DELAY,&passval)==-1) perror(NULL);

ioctl(wDevice,FSCC_FLUSH_TX,0);//flushtx	
	rdtscll(t1);
	for(loop=0;loop<askedfor;loop++)
	{
		error=0;

		tosend=size;
		// generate a random string of our random length.
		for(x=0;x<tosend;x++)	tdata[x]=(rand());
sprintf(&tdata[10],"%c%c%8.8lu%c%c",0xff,0x00,loop,0x00,0xff);//put our frame counter in ascii in the data
//printf("pre-write\n");
		t = write(wDevice,&tdata[0],tosend);
//printf("returned:%d",t);
//usleep(1000);
		if(t<0) perror(NULL);
		totalsent+=t;
		if(fout!=NULL) fwrite(tdata,1,tosend,fout);
	}
status=0;
while((status&ST_ALLS)!=ST_ALLS)
{
ioctl(wDevice,FSCC_IMMEDIATE_STATUS,&status);
}
	rdtscll(t2);
	printf("Wrote %lu bytes in %f seconds\n",totalsent,(double)(t2-t1)/(double)tickspersecond);
	printf("approx bps :%.0f\n",((double)totalsent*8.0)/((double)(t2-t1)/(double)tickspersecond));
	printf("count %lu\n",loop);
passval=0;	
if(ioctl(wDevice,FSCC_TRANSMIT_DELAY,&passval)==-1) perror(NULL);


close:
free(tdata);
close(wDevice);
if(fout!=NULL) fclose(fout);
return 0;
}
