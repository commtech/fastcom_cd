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
	ULONG ftype;
	if(argc<3)
	{
		printf("usage:\n");
		printf("%s port blocksize [savefile]\n",argv[0]);
		exit(1);
	}
	if(argc>4)
	{
		ftype=atoi(argv[4]);
	}
	else ftype=0;
	
	if(argc>3)
	{
		printf("opening save file:%s\n",argv[3]);
		fout = fopen(argv[3],"wb");
		if(fout==NULL)
		{
			printf("cannot open save file\n");
			exit(1);
		}
	}
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
//		MessageBox( NULL, "Failed to create event for thread!", "main Error!",
//			MB_ICONEXCLAMATION | MB_OK ) ;
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
		printf ("Can't get a handle to fsccdrv\n");
		exit(1);
		//abort and leave here!!!
	}
//	printf("fsccdrv handle created\n");
	tdata = (char*)malloc(size+1);
	if(tdata==NULL)
	{
		printf("unable to allocate data buffer\n");
		exit(1);
	}
	
	DeviceIoControl(wDevice,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&t,NULL);
	k=1;
	DeviceIoControl(wDevice,IOCTL_FSCCDRV_TIMED_TRANSMIT,&k,sizeof(ULONG),NULL,0,&t,NULL);
	
	totalerror=0;
	totalsent=0;
	totalread=0;
	loop=0;
	
	while(!kbhit())
	{
		error=0;
		
		//generate a random length 1 - 4095 bytes
		tosend=size;
		//generate a random string of our random length.
		if(ftype==0)	for(x=0;x<tosend;x++) 	tdata[x]=(UCHAR)(rand());
		if(ftype==1)	for(x=0;x<tosend;x++) 	tdata[x]=(UCHAR)(0x55);
		if(ftype==2)	for(x=0;x<tosend;x++) 	tdata[x]=(UCHAR)(0xaa);
		if(ftype==3)    for(x=0;x<tosend;x++) 	tdata[x]=(UCHAR)(0x00);
		if(ftype==4)	for(x=0;x<tosend;x++) 	tdata[x]=(UCHAR)(0xff);
		sprintf(&tdata[10],"%c%c%8.8lu%c%c",0xff,0x00,loop,0x00,0xff);//put our frame counter in ascii in the data
//		printf("pre-write\n");
//		printf("tosend:%d\n",tosend);
		t = WriteFile(wDevice,&tdata[0],tosend,&nobyteswritten,&wq);
		
//		Sleep(1);	
		
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
//						else printf("block\n");
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
		printf("loop:%d\n",loop);
		loop++;
		totalsent+=nobyteswritten;
		
//		if(fout!=NULL) fwrite(tdata,1,tosend,fout);
	}
	key = getch();	
	printf("Wrote %lu bytes\n",totalsent);
	printf("count %lu\n",loop);
	
close:
	k=0;
	DeviceIoControl(wDevice,IOCTL_FSCCDRV_TIMED_TRANSMIT,&k,sizeof(ULONG),NULL,0,&t,NULL);
	
	printf("stoptx\n");
	passval[0] = 0x14;//CMDR register
	passval[1] = 0x00000002;//STIMR (stop timer command)
	DeviceIoControl(wDevice,IOCTL_FSCCDRV_WRITE_REGISTER,&passval,2*sizeof(unsigned long),NULL,0,&t,NULL);
	DeviceIoControl(wDevice,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&t,NULL);
	
	free(tdata);
	CloseHandle(wq.hEvent);
	CloseHandle (wDevice);
//	if(fout!=NULL) fclose(fout);
	return 0;
}
