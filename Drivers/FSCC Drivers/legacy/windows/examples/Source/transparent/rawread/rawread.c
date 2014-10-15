/* $Id$ */
/*
Copyright(C) 2002, Commtech, Inc.
rawread.c -- a user mode program to read bytes from a channel and stuff them to a file (assuming transparent mode operation, and forcing readcutoff)

 usage:
  rawread port size outfile


*/


#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "..\..\fscc.h"



int main(int argc, char *argv[])
{
	HANDLE rDevice;
	FILE *fout;
	ULONG t,temp;
	DWORD nobytesread;
	char *rdata;
	ULONG size;
	OVERLAPPED  rq;
	unsigned i,j,error;
	ULONG totalsent;
	ULONG totalread;
	ULONG totalerror;
	ULONG loop;
    char devname[25];
	struct setup settings;

	if(argc!=4)
	{
		printf("usage:\n");
		printf("read port size outfile\n");
		printf("size must be the same as n_rfsize_max!!!\n");
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
	
	sprintf(devname,"\\\\.\\FSCC%d",atoi(argv[1]));
	printf("devicename:%s\n",devname);
	
	memset( &rq, 0, sizeof( OVERLAPPED ) ) ;	//wipe the overlapped struct
	rq.hEvent = CreateEvent( NULL,    // no security
		TRUE,    // explicit reset req
		FALSE,   // initial event reset
		NULL ) ; // no name
	if (rq.hEvent == NULL)
	{
		printf("Failed to create event for thread!\n");
		fclose(fout);
		exit(1);
	}
	rDevice = CreateFile (devname,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		NULL
		);
	
    if (rDevice == INVALID_HANDLE_VALUE)
    {
		//for some reason the driver won't load or isn't loaded
		printf ("Can't get a handle to %s\n",devname);
		fclose(fout);
		CloseHandle(rq.hEvent);
		exit(1);
		//abort and leave here!!!
	}
	//allocate memory for read/write
	rdata = (char*)malloc(size+1);
	if(rdata==NULL)
	{
		printf("cannot allocate memory for data area\n");
		fclose(fout);
		CloseHandle(rDevice);
		CloseHandle(rq.hEvent);
		exit(1);
	}


    DeviceIoControl(rDevice,IOCTL_FSCCDRV_GET_SETUP,NULL,0,&settings,sizeof(struct setup),&t,NULL);
	if(settings.n_rfsize_max!=size)
	{
		printf("aborting\n");
		printf("size must == n_rfsize_max!\n");
		fclose(fout);
		CloseHandle(rDevice);
		CloseHandle(rq.hEvent);
		exit(1);
	
	}


	temp=1;//enable readcutoff
    DeviceIoControl(rDevice,IOCTL_FSCCDRV_ALLOW_READ_CUTOFF,&temp,sizeof(ULONG),NULL,0,&t,NULL);


	DeviceIoControl(rDevice,IOCTL_FSCCDRV_FLUSH_RX,NULL,0,NULL,0,&t,NULL);
	
	totalerror=0;
	totalsent=0;
	totalread=0;
	loop=0;
	
	while(!kbhit())
	{
		
		error=0;
		t = ReadFile(rDevice,&rdata[0],size,&nobytesread,&rq);
		if(t==FALSE)  
		{
			t=GetLastError();
			if(t==ERROR_IO_PENDING)
			{
				do
				{
					j = WaitForSingleObject( rq.hEvent, 5000 );//5 second timeout -- must be greater than size*8*(1/bitrate)*1000
					if(j==WAIT_TIMEOUT)
					{
						if(kbhit()) goto done;
						printf("Reciever Timeout.\r\n");
						//DeviceIoControl(rDevice,IOCTL_FSCCDRV_FLUSH_RX,NULL,0,NULL,0,&returnsize,NULL);
					}
					if(j==WAIT_ABANDONED)
					{
						printf("Reciever Abandoned.\r\n");
						goto close;
					}
				} while(j!=WAIT_OBJECT_0);
				GetOverlappedResult(rDevice,&rq,&nobytesread,TRUE);
			}
			else printf("READ ERROR: #%x\n",t);
		}
		//      else printf("read returned true:%d\n",nobytesread);
		
		//printf("[%d]READ %d\n\n",t,nobytesread);
		if(nobytesread!=size)
		{
			printf("received:%lu, expected %lu\n",nobytesread,size);
		}
		if(nobytesread!=0)
		{
			totalsent+=fwrite(rdata,1,nobytesread,fout);//in transparent mode with cutoff enabled, all bytes are good
		}
		loop++;
		totalread+=nobytesread;
	}
done:
	getch();
	printf("Read  %lu bytes\n",totalread);
	printf("Wrote %lu bytes\n",totalsent);
	printf("Count %lu\n",loop);
	
	
close:
	temp=0;//disable readcutoff
    DeviceIoControl(rDevice,IOCTL_FSCCDRV_ALLOW_READ_CUTOFF,&temp,sizeof(ULONG),NULL,0,&t,NULL);

	free(rdata);
	fclose(fout);
	CloseHandle(rDevice);
	CloseHandle(rq.hEvent);
	return 0;
}
/* $Id$ */