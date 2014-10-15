/* $Id$ */
/*
Copyright(c) 2002, Commtech, Inc.
write_random.c -- a user mode program to write a random bitstream (with counter) to a port

 usage:
  write_random port size [savefile]

port is the FSCC port to use
size is the blocksize to use for writes
savefile is the optional file to dump the data to as well as pumping it out the port

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
	FILE *fout=NULL;
	unsigned key=0;
if(argc<3)
{
printf("usage:\n");
printf("%s port blocksize [savefile]\n",argv[0]);
exit(1);
}
if(argc>3)
{
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
		printf ("Can't get a handle to fsccdrv\n");
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

DeviceIoControl(wDevice,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&t,NULL);

	totalerror=0;
	totalsent=0;
	totalread=0;
	loop=0;
	
	while(!kbhit())
	{
		error=0;

		// generate a random length 1 - 4095 bytes
		tosend=size;
		// generate a random string of our random length.
		for(x=0;x<tosend;x++) 
			tdata[x]=(UCHAR)(rand());
sprintf(&tdata[10],"%c%c%8.8lu%c%c",0xff,0x00,loop,0x00,0xff);//put our frame counter in ascii in the data
//printf("pre-write\n");
		t = WriteFile(wDevice,&tdata[0],tosend,&nobyteswritten,&wq);

//		Sleep(1);	
		
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
						printf("Transmitter Locked up... Resetting TX.\r\n");
                                                //DeviceIoControl(wDevice,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&returnsize,NULL);
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
		//printf("loop:%d\n",loop);
		loop++;
		totalsent+=nobyteswritten;
		if(fout!=NULL) fwrite(tdata,1,tosend,fout);
	
	}
	key = getch();	
	printf("Wrote %lu bytes\n",totalsent);
	printf("count %lu\n",loop);

close:
free(tdata);
CloseHandle(wq.hEvent);
CloseHandle (wDevice);
if(fout!=NULL) fclose(fout);
	return 0;
}
/* $Id$ */
