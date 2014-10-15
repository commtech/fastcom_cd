/* $Id$ */
/*
Copyright(c) 2006 Commtech, Inc.
sendfile.c -- a user mode program to send a file out a FSCC port

 usage:
  sendfile port blocksize file_to_send


*/


#include <windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "..\fscc.h"



int main(int argc, char *argv[])
{
    HANDLE wDevice;    /* Handler for the FSCC driver */
	FILE *fd;						/* Handle for the file to Open */
	int error,t,j;
	unsigned long size;
	DWORD nobyteswritten;
	DWORD blocksize;
	char *txbuffer;
	OVERLAPPED  wq;
	char devname[80];
	ULONG temp;
	if(argc<4)
	{
		printf("usage: %s port blocksize file_to_send\n",argv[0]);
		exit(1);
	}

/********************************
******* Crate File Handle *******
********************************/
	/* Open the File to sends */
	if((fd = fopen(argv[3],"rb"))==NULL)
	{
		printf("Cannot open specified file %s\r\n",argv[3]);
		exit(1);
	}
	size=0;
	
	fseek(fd,0,SEEK_END);
	size = ftell(fd);
	fseek(fd,0,SEEK_SET);
	printf("%s: %d bytes\n",argv[3],size);

	blocksize = atol(argv[2]);
	if(blocksize==0)blocksize=1;
	
	txbuffer = (char *)malloc(blocksize+1);
	
	if(txbuffer == NULL){
		printf("Could not Allocate %d bytes of memory for txbuffer.!\n",blocksize);
		exit(1);
	}

	memset(txbuffer,0,blocksize);

/*****************************************
******* Create Port 0 Device Handle ******
*****************************************/
	memset( &wq, 0, sizeof( OVERLAPPED ) );
	wq.hEvent = CreateEvent( NULL,TRUE,FALSE,NULL);
	if (wq.hEvent == NULL)
	{
		printf("Failed to create event for thread!\n");
		return 1; 
	}
	sprintf(devname,"\\\\.\\FSCC%d",atol(argv[1]));
        wDevice = CreateFile (devname, GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,NULL);
    if (wDevice == INVALID_HANDLE_VALUE)
    {
		LPVOID lpMsgBuf;
        printf ("Cannot open FSCC%d\n",atol(argv[1]));
		
		j = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,    
			NULL,
			j,
			MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);
		
		printf("%d: %s\n",j,lpMsgBuf);
		LocalFree(lpMsgBuf);
		free(txbuffer);
		fclose(fd);
		exit(1);
	}
	DeviceIoControl(wDevice,IOCTL_FSCCDRV_FLUSH_TX,NULL,0,NULL,0,&temp,NULL);

	while(!feof(fd))
	{
		error=0;
		size= fread(txbuffer,1,blocksize,fd);
                if(size!=0)
                {
		t = WriteFile(wDevice,txbuffer,size,&nobyteswritten,&wq);
		if(t==FALSE)  
		{
			LPVOID lpMsgBuf;
			t=GetLastError();
		if(t==ERROR_IO_PENDING)
			{
				do
				{
					j = WaitForSingleObject( wq.hEvent, 1500 );//1.5 second timeout
					if(j==WAIT_TIMEOUT)
					{
						printf("Transmitter Timed out (1.5Sec).\r\n");
						if(kbhit())
							{
							if(getch()==27)
								{
								printf("aborting transmit on user request\r\n");
								goto close;
								}
							}
					}
					if(j==WAIT_ABANDONED)
					{
						printf("Wait Abandoned.\r\n");
						goto close;
					}
				} while(j!=WAIT_OBJECT_0);
			}
			else
			{
						FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,    
					NULL,   // message source
					t,  // message identifier
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // language identifier
					(LPTSTR) &lpMsgBuf,    // message buffer
					0,        // maximum size of message buffer
					NULL  // array of message inserts
					);
			printf("%s\n",lpMsgBuf);
			LocalFree(lpMsgBuf);
			}
                }
		}
	}

close:
	free(txbuffer);
	CloseHandle (wDevice);
	fclose(fd);
	return 0;
}
/* $Id$ */
