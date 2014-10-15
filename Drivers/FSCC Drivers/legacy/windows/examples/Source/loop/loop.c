/* $Id$ */
/*
Copyright(c) 2006 Commtech, Inc.
loop.c -- a user mode program to effect a loopback on a FSCC channel

 usage:
  loop port


*/


#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "..\fscc.h"



int main(int argc, char *argv[])
{
	HANDLE wDevice;/* Handle for the FSCC port */
	HANDLE rDevice;
	ULONG t;
	DWORD nobyteswritten;
	DWORD nobytesread;
	char *tdata;
	char *rdata;
	OVERLAPPED  wq,rq;
	int j,x,error,tosend;
	__int64 totalsent;
	__int64 totalread;
	__int64 totalerror;
	__int64 loop;
    char devname[25];
	ULONG passval[2];
	ULONG val;
	ULONG temp;
	ULONG reg;
	ULONG bf;
	ULONG status;
	ULONG size;
	
	if(argc<4)
	{
		printf("usage:\n%s X Y size\n",argv[0]);
		printf("      X  tx port number\n");
		printf("      Y  rx port number\n");
		printf("      size is max packet size\n");
		
		exit(1);
	}
	size = atoi(argv[3]);
	if(size<1)
	{
		printf("size must be >1 :size=%d\n",size);
		exit(1);
	}
	srand( (unsigned)time( NULL ) );
	
	
    if(argc>=2) sprintf(devname,"\\\\.\\FSCC%d",atoi(argv[1]));
	printf("devicename:%s\n",devname);
	
	wDevice = CreateFile (devname,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		NULL
		);
    if (wDevice == INVALID_HANDLE_VALUE)
    {
		//for some reason the driver won't load or isn't loaded
		printf ("Can't get a handle to fsccdrv\n");
		exit(1);
		//abort and leave here!!!
	}
	//printf("fsccdrv handle created\n");
	
	memset( &wq, 0, sizeof( OVERLAPPED ) ) ;	//wipe the overlapped struct
	wq.hEvent = CreateEvent( NULL,    // no security
		TRUE,    // explicit reset req
		FALSE,   // initial event reset
		NULL ) ; // no name
	if (wq.hEvent == NULL)
	{
		//MessageBox( NULL, "Failed to create event for thread!", "main Error!",
		//	MB_ICONEXCLAMATION | MB_OK ) ;
		printf("Failed to create event for thread!\n");
		CloseHandle(wDevice);
		return 1; 
	}
	//printf("write overlapped event created\n");
	
	
	memset( &rq, 0, sizeof( OVERLAPPED ) ) ;	//wipe the overlapped struct
	rq.hEvent = CreateEvent( NULL,    // no security
		TRUE,    // explicit reset req
		FALSE,   // initial event reset
		NULL ) ; // no name
	if (rq.hEvent == NULL)
	{
		//MessageBox( NULL, "Failed to create event for thread!", "main Error!",
		//	MB_ICONEXCLAMATION | MB_OK ) ;
		printf("Failed to create event for thread!\n");
		CloseHandle(wDevice);
		CloseHandle(wq.hEvent);
		return 1; 
	}
	
    if(argc>=3) sprintf(devname,"\\\\.\\FSCC%d",atoi(argv[2]));
	printf("devicename:%s\n",devname);
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
		printf ("Can't get a handle to fsccdrv\n");
		exit(1);
		//abort and leave here!!!
	}
	//printf("read overlapped event created\n");
	tdata = malloc(size+1);
	rdata = malloc(size+3);
	if((tdata==NULL)||(rdata==NULL))
	{
		printf("cannot allocate memory for buffers\n");
		if(tdata) free(tdata);
		if(rdata) free(rdata);
		CloseHandle(rq.hEvent);
		CloseHandle(wq.hEvent);
		CloseHandle (wDevice);
		exit(1);
		
	}
	//rDevice = wDevice;
	/* Flush the RX Descriptors so not as to have any complete descriptors in their
	* the first read in hdlc will get those left over frames and this test program
	* would not be of any use. */
	//printf("flush rx\n");
	DeviceIoControl(rDevice,IOCTL_FSCCDRV_FLUSH_RX,NULL,0,NULL,0,&t,NULL);
	DeviceIoControl(wDevice,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&t,NULL);
	
	totalerror=0;
	totalsent=0;
	totalread=0;
	loop=0;
	while(!kbhit())
	{
		error=0;
		tosend= (ULONG)(rand() % (size));
		//tosend = atoi(argv[3]);
		if(tosend==0) tosend=1;
		//printf("tosend               :%d\n",tosend);
		//generate a random string of our random length.
		
		for(x=0;x<tosend;x++) 
		{
			tdata[x]= (UCHAR)(rand());
		}
		
		//printf("pre-write\n");
		
		t = WriteFile(wDevice,&tdata[0],tosend,&nobyteswritten,&wq);
		if(t==FALSE)  
		{
			t=GetLastError();
			printf("WRITE ERROR: #%d\n",t);
			if(t==ERROR_IO_PENDING)
			{
				do
				{
					j = WaitForSingleObject( wq.hEvent, 1500 );//1.5 second timeout
					if(j==WAIT_TIMEOUT)
					{
						printf("Transmitter Locked up... Resetting TX.\r\n");
						//DeviceIoControl(wDevice,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&returnsize,NULL);
					}
					if(j==WAIT_ABANDONED)
					{
						printf("Recieve Abandoned.\r\n");
						goto close;
					}
				} while(j!=WAIT_OBJECT_0);
			}
			else
			{
				printf("\r\na Write ERROR occured on FSCC%u\r\n",atoi(argv[1]));
				printf("Failed with 0x%X error",GetLastError());
				error++;
			}

		}
		
		t = ReadFile(rDevice,&rdata[0],size+2,&nobytesread,&rq);
		if(t==FALSE)  
		{
			t=GetLastError();
			if(t==ERROR_IO_PENDING)
			{
				do
				{
					j = WaitForSingleObject( rq.hEvent, 5000 );//5 second timeout
					if(j==WAIT_TIMEOUT)
					{
						printf("Reciever Locked up... Resetting RX.\r\n");
						//DeviceIoControl(rDevice,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&returnsize,NULL);
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
		//else printf("read returned true:%d\n",nobytesread);
		
		//printf("[%d]READ %d\n\n",t,nobytesread);
		
		
		if(nobytesread!=(ULONG)tosend+2)
		{
			printf("Byte Count ERROR rec:%d != sent:%ld\n",nobytesread,tosend+2);
			error++;
			DeviceIoControl(rDevice,IOCTL_FSCCDRV_IMMEDIATE_STATUS,NULL,0,&status,sizeof(status),&t,NULL);
			if((status&0x00000200)!=0) printf("TDU!\n");
			DeviceIoControl(rDevice,IOCTL_FSCCDRV_FLUSH_RX,NULL,0,NULL,0,&t,NULL);
			DeviceIoControl(wDevice,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&t,NULL);
			getch();
		}
		
		
		if(nobytesread!=0)
		{
			if(nobytesread<32768)
				for(x=0;x<(int)nobytesread-2;x++)
					if((rdata[x]&0xff)!=(tdata[x]&0xff))	
					{
						//	printf("[%d] %x != %x\n",x,rdata[x]&0xff,tdata[x]&0xff);
						error++;
					}
		}
		
		//printf("Found: %d errors\n",error);
		totalerror +=error;
		loop++;
		totalsent+=tosend;
		totalread+=nobytesread;
		printf("loop:%15I64d :%d\n",loop,tosend);
		
	}
	getch();
	
	printf("Found %I64d errors out of %I64d frames\n",totalerror,loop);
	printf("Wrote %I64d bytes.\n",totalsent);
	printf("Read %I64d bytes.\n",totalread-(loop*1));
	
close:
	CloseHandle (wDevice);
	if(tdata) free(tdata);
	if(rdata) free(rdata);
	CloseHandle(rq.hEvent);
	CloseHandle(wq.hEvent);
	
	return 0;
}
/* $Id$ */