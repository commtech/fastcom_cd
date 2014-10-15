/*
write_random.c -- a user mode program to write a random bitstream (with counter) to a port

 usage:
  write_random port size


*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <malloc.h>
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
if(argc<4)
{
printf("usage:\n");
printf("write_random port blocksize totalbytes [savefile]\n");
exit(1);
}
if(argc>4)
{
fout = fopen(argv[4],"wb");
if(fout==NULL)
	{
	printf("cannot open save file\n");
	exit(1);
	}
}
askedfor = atol(argv[3]);
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
	printf("total requested:%ld\n",askedfor);
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
ioctl(wDevice,FSCC_FLUSH_TX,0);//flushtx	
	while(totalsent<askedfor)
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
		loop++;
		totalsent+=t;
		if(fout!=NULL) fwrite(tdata,1,tosend,fout);
	}
	printf("Wrote %lu bytes\n",totalsent);
	printf("count %lu\n",loop);

close:
free(tdata);
close(wDevice);
if(fout!=NULL) fclose(fout);
return 0;
}
