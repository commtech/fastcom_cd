/* $Id$ */
/*
Copyright(C) 2006, Commtech, Inc.

  rx.c -- a user mode program to read bytes from a channel and stuff them to a file
	   -- configures the port to X-sync, with a sync pattern of 0x7e00
	   -- sets the clock to 10M, the baudrate to 2.5M, using external RT and generated TT.
	   -- configures receive length check to terminate frames at 22 bytes
	   -- no zero/one insertion, no termiantion sequence, no CRC checking.
	   -- NRZ data format.
	   -- we are expecting data of the form:
	   -- 0x7E,0x00,data[0]...data[18],chksum
	   -- received frame rate is expected to be 120Hz, so buffering is set to 240 frames (2 seconds worth)
	   --

 usage:
  read port size outfile [display]

  (do not use display except for debugging, as it will likely make gaps in the data)


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
    char devname[25];
	int display =0;	
	struct setup myset;
	ULONG passval[2];

	if(argc<3)
	{
		printf("usage:\n");
		printf("read port outfile\n");
		exit(1);
	}
	if(argc>3) display=1;
	else display=0;

	size = 32;

	fout=fopen(argv[2],"wb");
	if(fout==NULL)
	{
		printf("cannot open output file %s\n",argv[2]);
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

passval[0]=10000000;
passval[1]=2;
if(DeviceIoControl(rDevice,IOCTL_FSCCDRV_SET_FREQ,&passval[0],2*sizeof(ULONG),NULL,0,&t,NULL))
	{
	printf("Actual Frequency Set to: %d @ %d ppm\n",10000000,2);
	}
else printf("Set Freq failed:%8.8x\r\n",GetLastError());


myset.fifot        = 0x04000528;//txtrigger @1024, rxtriger @1320
myset.cmdr         = 0x08020000;//reset tx,reset rx
myset.ccr0         = 0x00104005;//X-Sync,2 sync bytes, no term bytes,clock mode 1,LSB, NRZ, recevier ON, idle '1's
myset.ccr1         = 0x04007088;//CTS manual,no TX crc,no term,save sync bytes,no rx CRC. Rxclk active low, txclk active high
myset.ccr2         = 0x00150000;//Receive length check enabled, stop rx at 22 bytes
myset.bgr          = 0x00000003;//divide by 4 (10E6 / 4 = 2.5 Mbps)
myset.ssr          = 0x0000007e;//sync sequence 0x7e00
myset.smr          = 0x00000000;//no sync mask (all bits used)
myset.tsr          = 0x0000007e;//no term used
myset.tmr          = 0x00000000;//no term mask used
myset.rar          = 0x00000000;//no receive address used
myset.ramr         = 0x00000000;//no receive adress mask used
myset.ppr          = 0x00000000;//no pre/postamble used
myset.tcr          = 0x000a2c18;//timer set to 83332, timeout @83333xOSC = .0083333 Seconds/timeout ~ 120Hz
myset.imr          = 0x00000000;//no masked interrupts
myset.n_rbufs      = 0xf0;		//240 frame buffers (rx)
myset.n_rfsize_max = 0x20;		//32 bytes/buffer
myset.n_tbufs      = 0xf0;		//240 frame buffers (tx)
myset.n_tfsize_max = 0x20;		//32 bytes/buffer

if(!DeviceIoControl(rDevice,IOCTL_FSCCDRV_SETUP,&myset,sizeof(struct setup),NULL,0,&t,NULL))
	{
	printf("Setup Failed! :0x%8.8x\n",GetLastError());
	}
	
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
		if(nobytesread!=24)//24 is our 2 byte sync, 19 bytes of data, 1 byte checksum, and 2 status bytes
		{
			printf("received:%lu, expected %lu\n",nobytesread,size);
		}
		if(nobytesread!=0)
		{
			unsigned rstatus;

			totalsent+=fwrite(rdata,1,nobytesread-2,fout);//write all but status out to the file
if(display==1)
{
			for(i=0;i<nobytesread;i++) printf("%2.2x:",rdata[i]&0xff);
			printf("\n");
			printf("-----\n");
			//for(i=0;i<nobytesread-2;i++) printf("%c",rdata[i]&0xff);
			//printf("\n");
			//printf("-----\n");
			rstatus = (rdata[nobytesread-1]<<8) | rdata[nobytesread-2];
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