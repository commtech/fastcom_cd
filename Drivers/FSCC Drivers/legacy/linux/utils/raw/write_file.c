/*
write_random.c -- a user mode program to write a file to a port

 usage:
  write_file port blocksize file 
(if using hdlc frames, you should keep blocksize under 4090 bytes)

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
	FILE *fin=NULL;
	ULONG askedfor;
        unsigned long bufparam[4];

if(argc<4)
{
printf("usage:\n");
printf("write_file port blocksize file\n");
exit(1);
}
fin = fopen(argv[3],"rb");
if(fin==NULL)
	{
	printf("cannot open file:%s\n",argv[3]);
	exit(1);
	}
size = atol(argv[2]);
if(size==0)
{
printf("block size must be nonzero\n");
exit(1);
}
if(size>4096)
{
printf("block size must be <=4096\n");
exit(1);
}
    sprintf(devname,"/dev/fscc%d",atoi(argv[1]));
	printf("devicename:%s\n",devname);
	printf("blocksize:%lu\n",size);
	printf("file requested:%s\n",argv[3]);
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
	do
	{
		error=0;
		tosend = fread(&tdata[0],1,size,fin);
		//printf("pre-write\n");
if(tosend>0)	t = write(wDevice,&tdata[0],tosend);
		//printf("returned:%d",t);
		loop++;
		totalsent+=tosend;
		totalread+=tosend;
	}while(!feof(fin));

	printf("Wrote %lu bytes\n",totalsent);
	printf("Read  %lu bytes\n",totalread);
	printf("count %lu\n",loop);

close:
free(tdata);
close(wDevice);
if(fin!=NULL) fclose(fin);
return 0;
}
