/*
read.c -- a user mode program to read bytes from a channel and stuff them to a file

 usage:
  read port size outfile


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
#include <errno.h>
#include "../fsccdrv.h"
int main(int argc, char *argv[])
{
	int rDevice;
	FILE *fout;
	int t;
	unsigned long nobytesread;
	char *rdata;
	unsigned long size;
	int j,error;
	unsigned long totalsent;
	unsigned long totalread;
	unsigned long totalerror;
	unsigned long loop;
	unsigned long askedfor;
	unsigned long bufparam[4];
	unsigned long readcutoff;
	unsigned timeout;
    char devname[25];
if(argc<3)
{
printf("usage:\n");
printf("read port size outfile totalbytes\n");
exit(1);
}
size = atol(argv[2]);
if(size==0)
{
printf("block size cannot be 0\n");
exit(1);
}
fout=fopen(argv[3],"wb");
if(fout==NULL)
{
printf("cannot open output file %s\n",argv[3]);
exit(1);
}
askedfor = atol(argv[4]);
sprintf(devname,"/dev/fscc%d",atoi(argv[1]));
printf("devicename:%s\n",devname);
printf("blocksize:%ld\n",size);
printf("totalbytes requested:%ld\n",askedfor);
rDevice = open(devname,O_RDWR);
    if (rDevice == -1)
    	{
		//for some reason the driver won't load or isn't loaded
                printf ("Can't open %s\n",devname);
		
				fclose(fout);
				exit(1);
		//abort and leave here!!!
	}
rdata = (char*)malloc(size+1);
if(rdata==NULL)
{
printf("cannot allocate memory for data area\n");
fclose(fout);
close(rDevice);
exit(1);
}
	readcutoff=1;//enable read cutoff for transparent operation
	ioctl(rDevice,FSCC_ALLOW_READ_CUTOFF,&readcutoff);

	/* Flush the RX Descriptors so not as to have any complete descriptors in their
	 * the first read in hdlc will get those left over frames and this test program
	 * would not be of any use. */
printf("flush rx\n");
       ioctl(rDevice,FSCC_FLUSH_RX,0);	
       totalerror=0;
	totalsent=0;
	totalread=0;
	loop=0;
	timeout=0;
while(totalread<askedfor)
{
		error=0;
		t = read(rDevice,&rdata[0],size);
		if(t<0) 
		{
		perror(NULL);
		printf("read error!:%d\n",t);
		goto close;
		}

//		printf("READ %d\n\n",t);
		if(t==0)
		 {
		 timeout++;
		 printf("timeout:%d\n",timeout);
		 if(timeout>20) goto close;
		 }
		else if(t!=size)
		 {
		 printf("received:%d, expected %lu\n",t,size);
		 }

	if(t>0)
		{
		int i;
		timeout=0;
		totalsent+=fwrite(rdata,1,t,fout);
//		for(i=0;i<t;i++) printf("%x:",rdata[i]);
//		printf("\n");
		loop++;
		totalread+=(t);

		}
		//printf("Found: %d errors\n",error);
}
done:
	printf("Read  %lu bytes\n",totalread);
	printf("Wrote %lu bytes\n",totalsent);
	printf("Count %lu\n",loop);


close:
	readcutoff=0;//restore normal operation	
	ioctl(rDevice,FSCC_ALLOW_READ_CUTOFF,&readcutoff);

free(rdata);
fclose(fout);
close(rDevice);
	return 0;
}
