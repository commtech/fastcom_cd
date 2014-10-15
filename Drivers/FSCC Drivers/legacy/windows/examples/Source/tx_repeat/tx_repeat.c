/*
	tx_repeat.c - a user mode program to send a single frame repetitively using hardware
				- will send back to back if param3 is 0, or by the timer timeout if param3 is 1

  usage:
	tx_repeat port frame_size usetimer

	port is the port to open
	frame_size is the size of the frame to send (1 to 4096 byte)
	usetimer is 0 to send the frames back to back, 1 to send a frame on the expiration of the onboard timer (timer must be configured, and set to periodic mode!)
*/

#include "windows.h"
#include "conio.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "..\fscc.h"


int main(int argc, char *argv[])
{
	HANDLE wDevice;/* Handle for the Fastcom:FSCC port */
	ULONG t;
	DWORD nobyteswritten;
	char *tdata;
	ULONG size;
	OVERLAPPED  wq;
	int j,x,tosend;
    char devname[25];
	ULONG k;
	ULONG passval[2];
	unsigned key=0;
	ULONG start=0;
	int usetimer=0;

	if(argc<3)
		{
		printf("usage:\n");
		printf("%s port blocksize [0|1]\n",argv[0]);
		exit(1);
		}
	if(argc>3)
		{
		if(atoi(argv[3])==1) usetimer=1;
		else usetimer=0;
		}
	else usetimer=0;

	srand( (unsigned)time( NULL ) );
	size = atol(argv[2]);
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
		printf("Failed to create event for overlapped\n");
		return 1; 
		}
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
	tdata = (char*)malloc(size+1);
	if(tdata==NULL)
		{
		printf("unable to allocate data buffer\n");
		exit(1);
		}

	DeviceIoControl(wDevice,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&t,NULL);
	if(usetimer==1)
		{
		k=1;
		DeviceIoControl(wDevice,IOCTL_FSCCDRV_TIMED_TRANSMIT,&k,sizeof(ULONG),NULL,0,&t,NULL);
		}

	k=1;
	DeviceIoControl(wDevice,IOCTL_FSCCDRV_TRANSMIT_REPEAT,&k,sizeof(ULONG),NULL,0,&t,NULL);

	// generate a random length 1 - 4095 bytes
	tosend=size;
	// generate a random string of our random length.
	for(x=0;x<tosend;x++) 	tdata[x]=(UCHAR)(rand());
	sprintf(&tdata[10],"%c%c%8.8lu%c%c",0xff,0x00,0,0x00,0xff);//put our frame counter in ascii in the data
	t = WriteFile(wDevice,&tdata[0],tosend,&nobyteswritten,&wq);

	if(t==FALSE)  
		{
		t=GetLastError();
		
	
		if(t==ERROR_IO_PENDING)
			{
			do
				{
				j = WaitForSingleObject( wq.hEvent, 5000 );//5 second timeout -- must be larger than size*8*(1/bitrate)*1000
				if(j==WAIT_TIMEOUT)
					{
					printf("timeout\n");
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
		if(nobyteswritten!=size)
			{
			printf("unexpected actual:%lu, sent:%lu\n",nobyteswritten,size); 
			}
	if(usetimer==1)
		{
		if(start==0)
			{
			start=1;
			printf("startx\n");
			passval[0] = 0x14;//CMDR register
			passval[1] = 0x00000001;//TIMR (start timer command)
			DeviceIoControl(wDevice,IOCTL_FSCCDRV_WRITE_REGISTER,&passval,2*sizeof(unsigned long),NULL,0,&t,NULL);
			}
		}		
	key = getch();	
close:
	if(usetimer==1)
		{
		k=0;
		DeviceIoControl(wDevice,IOCTL_FSCCDRV_TIMED_TRANSMIT,&k,sizeof(ULONG),NULL,0,&t,NULL);
		}
	k=0;
	DeviceIoControl(wDevice,IOCTL_FSCCDRV_TRANSMIT_REPEAT,&k,sizeof(ULONG),NULL,0,&t,NULL);

	printf("stoptx\n");
	if(usetimer==1)
		{
		passval[0] = 0x14;//CMDR register
		passval[1] = 0x00000002;//STIMR (start timer command)
		DeviceIoControl(wDevice,IOCTL_FSCCDRV_WRITE_REGISTER,&passval,2*sizeof(unsigned long),NULL,0,&t,NULL);
		}
	DeviceIoControl(wDevice,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&t,NULL);

	free(tdata);
	CloseHandle(wq.hEvent);
	CloseHandle (wDevice);
	return 0;
}
