/* $Id$ */
/*
Copyright(C) 2007, Commtech, Inc.
readmult.c -- a user mode program to read bytes from a channel and stuff them to a file, 
              assuming rx multiple is set

 usage:
  read PORT SIZE OUTFILE [DISPLAY]
	PORT is any valid fscc port (0,1)
	SIZE is the expected receive framesize (not including the status bytes)
	OUTFILE is the file to store received the data bytes in.
	if DISPLAY is present frame data is printed to screen.

  NOTE: In order for this to work well you will want to adjust the number of 
  buffers (n_rbufs) in your settings.  The appropriate number will depend on your
  application, but you will definitely want to increase the number.
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
	ULONG t;
	DWORD nobytesread;
	char *rdata;
	ULONG size;
	OVERLAPPED  rq;
	unsigned i,j,error;
	ULONG totalsent;
	ULONG totalread;
	ULONG totalerror;
	ULONG loop;
	ULONG rxmult;
    char devname[25];
	int display_frame_data=0;	
	if(argc<=4)
	{
		printf("usage:\n");
		printf("read port size outfile display\n");
		exit(1);
	}
	if(argc>4) display_frame_data=1;
	else display_frame_data=0;
	
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
	
	DeviceIoControl(rDevice,IOCTL_FSCCDRV_GET_RECEIVE_MULTIPLE,NULL,0,&rxmult,sizeof(ULONG),&t,NULL);
	
	//allocate memory for read/write
	rdata = (char*)malloc(((size+2)*rxmult) +2);
	if(rdata==NULL)
	{
		printf("cannot allocate memory for data area\n");
		fclose(fout);
		CloseHandle(rDevice);
		CloseHandle(rq.hEvent);
		exit(1);
	}
	
	DeviceIoControl(rDevice,IOCTL_FSCCDRV_FLUSH_RX,NULL,0,NULL,0,&t,NULL);
	
	totalerror=0;
	totalsent=0;
	totalread=0;
	loop=0;
	
	while(!kbhit())
	{
		
		error=0;
		t = ReadFile(rDevice,&rdata[0],((size+2)*rxmult),&nobytesread,&rq);
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
						printf("Reciever Locked up... Resetting RX.\r\n");
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
		
		printf("[%d]READ %d\n\n",t,nobytesread);
		if(nobytesread!=((size+2)*rxmult))
		{
			printf("received:%lu, expected %lu\n",nobytesread,((size+2)*rxmult));
		}
		if(nobytesread!=0)
		{
			unsigned rstatus;
			for(i=0;i<rxmult;i++)
			{
				
				totalsent+=fwrite(&rdata[(size+2)*i],1,size,fout);
			}
			
			if(display_frame_data==1)
			{
				for(i=0;i<rxmult;i++)
				{	
					int bt;
					
					for(bt=0;bt<size;bt++) printf("%2.2x:",rdata[((size+2)*i)+bt]&0xff);
					printf("\n");
					printf("-----\n");
					//for(i=0;i<nobytesread-2;i++) printf("%c",rdata[i]&0xff);
					//printf("\n");
					//printf("-----\n");
					rstatus = (rdata[((size+2)*i)+bt]) | (rdata[((size+2)*i)+(bt+1)])<<8;
					printf("RSTATUS:%4.4x\n",rstatus);
					if((rstatus&0x2000)!=0) printf("RDO!  --Receive Data Overflow\n");
					if((rstatus&0x1000)!=0) printf("RFL!  --Receive Frame Lost\n");
					if((rstatus&0x0800)!=0) printf("VFR!  --Invalid Frame Received\n");
					if((rstatus&0x0400)!=0) printf("RLEX! -- Receive Length Check Exceeded\n");
					if((rstatus&0x0200)!=0) printf("CRC!  --CRC Error\n");
					if((rstatus&0x0100)!=0) printf("ABF!  --Aborted Frame Detected\n");
					
					printf("=====\n");
				}
			}
			
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
	free(rdata);
	fclose(fout);
	CloseHandle(rDevice);
	CloseHandle(rq.hEvent);
	return 0;
}
/* $Id$ */