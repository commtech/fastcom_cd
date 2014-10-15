/* $Id$ */
/*
	tx.c - user mode app to generate frames of the form:
		   0x7E,0x00,data[0]...data[18],chksum
		   The OSC is set to 10M
		   The BGR is set to 3 (divisor of 4) yielding a 2.5Mbps line data rate
		   The Timer is set to periodic with a timeout at 83333 clock pulses (@OSC) yielding a timeout rate @ 120.0004 Hz
		   X-SYNC mode selected with a 2 byte sync pattern of 0x7e00
		   NRZ line encoding
		   no CRCs (receive or transmit)
		   no transmitted termination sequence
		   idle '1's
		   no addresses/preamble/postamble usage
		   Clock mode 1 selected (generated transmit clock, received (external) receive clock (RT).

		   Uses transmit on timer mode to generate transmitted frames @ 120Hz
		   Fills in data[2] and data[3] with a (16bit) frame counter
		   Fills in data[19] with a checksum value (8 bit summed value of data[0]-data[18]

		   Displays progress every second.
  usage:

  tx fsccport

*/
#include "windows.h"
#include "conio.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "..\..\fscc.h"

int main(int argc, char *argv[])
{
	HANDLE wDevice;/* Handle for the Fastcom:FSCC port */
	ULONG t;
	DWORD nobyteswritten;
	char *tdata;
	ULONG size;
	OVERLAPPED  wq;
	int j,x,error,tosend;
	ULONG totalsent;
	ULONG totalread;
	ULONG totalerror;
	ULONG loop;
    char devname[25];
	
	ULONG k;
	ULONG passval[2];
	unsigned key=0;
	ULONG start=0;
	FILE *fout=NULL;
	struct setup myset;
	char sync[2];

if(argc<2)
{
printf("usage:\n");
printf("%s port\n",argv[0]);
exit(1);
}


if(argc>=3)
{
printf("opening save file:%s\n",argv[2]);
	fout = fopen(argv[2],"wb");
if(fout==NULL)
	{
	printf("cannot open save file\n");
	exit(1);
	}
}
srand( (unsigned)time( NULL ) );
size = 22;
if(size==0)
{
printf("block size must be nonzero\n");
exit(1);
}
    sprintf(devname,"\\\\.\\FSCC%d",atoi(argv[1]));
	printf("devicename:%s\n",devname);
	printf("blocksize:%lu\n",size);

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
		return 1; 
	}
//	printf("write overlapped event created\n");
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
		printf ("Can't get a handle to %s\n",devname);
		exit(1);
		//abort and leave here!!!
	}
//printf("fsccdrv handle created\n");
tdata = (char*)malloc(size+1);
if(tdata==NULL)
{
printf("unable to allocate data buffer\n");
exit(1);
}
passval[0]=10000000;
passval[1]=2;
if(DeviceIoControl(wDevice,IOCTL_FSCCDRV_SET_FREQ,&passval[0],2*sizeof(ULONG),NULL,0,&t,NULL))
	{
	printf("Actual Frequency Set to: %d @ %d ppm\n",10000000,2);
	}
else printf("Set Freq failed:%8.8x\r\n",GetLastError());


DeviceIoControl(wDevice,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&t,NULL);
k=1;
DeviceIoControl(wDevice,IOCTL_FSCCDRV_TIMED_TRANSMIT,&k,sizeof(ULONG),NULL,0,&t,NULL);


myset.fifot        = 0x04000528;//txtrigger @1024, rxtriger @1320
myset.cmdr         = 0x08000000;//reset tx
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

if(!DeviceIoControl(wDevice,IOCTL_FSCCDRV_SETUP,&myset,sizeof(struct setup),NULL,0,&t,NULL))
	{
	printf("Setup Failed! :0x%8.8x\n",GetLastError());
	}

	totalerror=0;
	totalsent=0;
	totalread=0;
	loop=0;
	sync[0]=0x7e;
	sync[1]=0x00;

	while(!kbhit())
	{
		UCHAR cksum;
		int i;
		error=0;

		// generate a random length 1 - 4095 bytes
		tosend=size;
		// generate a random string of our random length.

		for(x=0;x<tosend;x++) 	tdata[x]=(UCHAR)(rand());
		tdata[3] = (UCHAR)(loop&0xff);
		tdata[2] = (UCHAR)((loop>>8)&0xff);
		cksum=0;
		for(i=0;i<19;i++) cksum += tdata[i];
		tdata[19]=cksum;
		t = WriteFile(wDevice,&tdata[0],20,&nobyteswritten,&wq);
		
		if(fout!=NULL)
			{
			sync[0]=0x7e;
			sync[1]=0x00;

			fwrite(&sync[0],1,2,fout);
			fwrite(tdata,1,20,fout);
			fflush(fout);
			}
		
		if(t==FALSE)  
		{
			t=GetLastError();
			if(t==ERROR_IO_PENDING)
			{
				do
				{
//					printf("o");
					j = WaitForSingleObject( wq.hEvent, 1000 );//1 second timeout -- must be larger than size*8*(1/bitrate)*1000
					if(j==WAIT_TIMEOUT)
					{
					//we now have all the transmit frames queued up, so start the timer to start the transmitting
					//this should make a nice transition from not transmitting to transmitting with (many) frames 
					//ready to go
					if(start==0)
						{
						start=1;
						printf("startx\n");
						passval[0] = 0x14;//CMDR register
						passval[1] = 0x00000001;//TIMR (start timer command)
						DeviceIoControl(wDevice,IOCTL_FSCCDRV_WRITE_REGISTER,&passval,2*sizeof(unsigned long),NULL,0,&t,NULL);
						}
//					else printf("block\n");
					}
					if(j==WAIT_ABANDONED)
					{
						printf("Recieve Abandoned.\r\n");
						goto close;
					}
				} while(j!=WAIT_OBJECT_0);
				GetOverlappedResult(wDevice,&wq,&nobyteswritten,TRUE);
			}
			else printf("WRITE ERROR: #%d\n",t);
		}
		if(nobyteswritten!=size-2)
		{
		printf("unexpected actual:%lu, sent:%lu\n",nobyteswritten,size-2); 
		}
		if((loop%120)==0)printf("Count:%d\n",loop);
		loop++;
		totalsent+=nobyteswritten;
	}
	key = getch();	
	printf("Wrote %lu bytes\n",totalsent);
	printf("count %lu\n",loop);

close:
Sleep(5000);
k=0;
DeviceIoControl(wDevice,IOCTL_FSCCDRV_TIMED_TRANSMIT,&k,sizeof(ULONG),NULL,0,&t,NULL);

printf("stoptx\n");
passval[0] = 0x14;//CMDR register
passval[1] = 0x00000002;//STIMR (start timer command)
DeviceIoControl(wDevice,IOCTL_FSCCDRV_WRITE_REGISTER,&passval,2*sizeof(unsigned long),NULL,0,&t,NULL);
DeviceIoControl(wDevice,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&t,NULL);

free(tdata);
CloseHandle(wq.hEvent);
CloseHandle (wDevice);
if(fout!=NULL) fclose(fout);
	return 0;


}
