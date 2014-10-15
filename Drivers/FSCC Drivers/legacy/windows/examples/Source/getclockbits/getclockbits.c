/*
	Copyright(C) 2010, Commtech, Inc.

	getbits.c --   a user program to get the current clock generator bits 
			
usage:
 getclockbits port

 The port can be any valid fscc port (0,1).
 
*/
#include <windows.h>
#include <stdio.h>
#include "../fscc.h"

int main(int argc, char * argv[])
{
	HANDLE dev;
	unsigned char progbits[20];
	int res,i;
	char devname[25];
	
	if(argc!=2)
	{
		printf("usage:\n%s port\n",argv[0]);
		exit(1);
	}
	sprintf(devname,"\\\\.\\FSCC%d",atoi(argv[1]));
	dev = CreateFile(devname,
		GENERIC_WRITE | GENERIC_READ , 
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,OPEN_EXISTING, 
		0, 
		NULL);
	
	if(dev== INVALID_HANDLE_VALUE)
	{
		printf("ERROR: Could not open device!\n");
		exit(1);
	}

    if(DeviceIoControl(dev,IOCTL_FSCCDRV_GET_PROGBITS,NULL,0,&progbits,(20*sizeof(unsigned char)),&res,NULL))
	{
		printf("Programming bits are:\n");
		for(i=0;i<20;i++)
			printf("%2.2x ", progbits[i]);
	}
	else
	{
		printf("Error with IOCTL: %d",GetLastError());
	}
	CloseHandle(dev);
	return 0;
}