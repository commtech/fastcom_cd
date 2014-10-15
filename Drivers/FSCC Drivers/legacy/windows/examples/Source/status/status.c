/* $Id$ */
/*
Copyright(C) 2002, Commtech, Inc.
status.c -- a user mode program to read status from a FSCC port

 usage:
  status port [mask]


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
	HANDLE sDevice;
	ULONG t;
	
	OVERLAPPED  st;
	int j;
	ULONG mask;
	ULONG status;
    char devname[25];
	if(argc<2)
	{
		printf("usage:\n");
		printf("%s port [mask]\n",argv[0]);
		exit(1);
	}
	if(argc>2)
	{
		sscanf(argv[2],"%x",&mask);
	}
	else mask = 0xffffffff;
	
	sprintf(devname,"\\\\.\\FSCC%d",atoi(argv[1]));
	printf("devicename:%s\n",devname);
	
	memset( &st, 0, sizeof( OVERLAPPED ) ) ;	//wipe the overlapped struct
	st.hEvent = CreateEvent( NULL,    // no security
		TRUE,    // explicit reset req
		FALSE,   // initial event reset
		NULL ) ; // no name
	if (st.hEvent == NULL)
	{
		//MessageBox( NULL, "Failed to create event for thread!", "main Error!",
		//	MB_ICONEXCLAMATION | MB_OK ) ;
		printf("Failed to create event for thread!\n");
		exit(1);
	}
	sDevice = CreateFile (devname,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		NULL
		);
	
    if (sDevice == INVALID_HANDLE_VALUE)
    {
		//for some reason the driver won't load or isn't loaded
		printf ("Can't get a handle to fsccdrv\n");
		CloseHandle(st.hEvent);
		exit(1);
		//abort and leave here!!!
	}
	
	
	while(!kbhit())
	{
		
		status=0;
		
		t=      DeviceIoControl(sDevice,IOCTL_FSCCDRV_STATUS,&mask,sizeof(ULONG),&status,sizeof(ULONG),&t,&st);
		if(t==FALSE)  
		{
			t=GetLastError();
			if(t==ERROR_IO_PENDING)
			{
				do
				{
					j = WaitForSingleObject( st.hEvent, 5000 );//5 second timeout -- must be greater than size*8*(1/bitrate)*1000
					if(j==WAIT_TIMEOUT)
					{
						if(kbhit()) goto done;
						printf("Timeout.\r");
						//You could put a counter here and if you hit it some
						//amount of times you know something is wrong - you 
						//could then do a IOCTL_FSCCDRV_CANCEL_STATUS or flush
						//or exit or whatever you want
					}
					if(j==WAIT_ABANDONED)
					{
						printf("Abandoned.\r\n");
						goto close;
					}
				} while(j!=WAIT_OBJECT_0);
			}
			else printf("ERROR: #%x\n",t);
		}
		printf("STATUS: %8.8x\n\n",status);
		if((status&ST_CTSA)!=0) printf("ST_CTSA\n");
		if((status&ST_CDC)!=0)  printf("ST_CDC\n");
		if((status&ST_DSRC)!=0) printf("ST_DSRC\n");
		if((status&ST_CTSS)!=0) printf("ST_CTSS\n");
		if((status&ST_TIN)!=0)  printf("ST_TIN\n");
		if((status&ST_RFS)!=0)  printf("ST_RFS\n");
		if((status&ST_RFL)!=0)  printf("ST_RFL\n");
		if((status&ST_RDO)!=0)  printf("ST_RDO\n");
		if((status&ST_RFO)!=0)  printf("ST_RFO\n");
		if((status&ST_TDU)!=0)  printf("ST_TDU\n");
		if((status&ST_OVF)!=0)  printf("ST_OVF\n");
		if((status&ST_RX_DONE)!=0) printf("ST_RX_DONE\n");
		if((status&ST_ALLS)!=0)  printf("ST_ALLS\n");
		if((status&ST_TX_DONE)!=0) printf("ST_TX_DONE\n");
		if((status&ST_RFT)!=0)  printf("ST_RFT\n");
		if((status&ST_TFT)!=0)  printf("ST_TFT\n");
		if((status&ST_RFE)!=0)  printf("ST_RFE\n");
		if((status&ST_RBUF_OVERFLOW)!=0) printf("ST_RBUF_OVERFLOW\n");
	}
	
	
	//t=DeviceIoControl(sDevice,IOCTL_FSCCDRV_IMMEDIATE_STATUS,&mask,sizeof(ULONG),&status,sizeof(ULONG),&t,&st);
	//printf("STATUS: %8.8x\n\n",status);
	
done:
	getch();
	
	
close:
	CloseHandle(sDevice);
	CloseHandle(st.hEvent);
	
	return 0;
}
/* $Id $ */
