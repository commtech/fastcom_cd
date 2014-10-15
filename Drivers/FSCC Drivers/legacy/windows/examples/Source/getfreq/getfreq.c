/* $Id$ */
/*
	Copyright(C) 2006, Commtech, Inc.

	getfreq.c --   a user program to get the current clock generator setting 
			
usage:
 getfreq port

 The port can be any valid fscc port (0,1).
 
*/
#include <windows.h>
#include <stdio.h>
#include "..\fscc.h"

int main(int argc, char * argv[])
{
	HANDLE dev;
	unsigned long freq;
	int res;
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
	
    if(DeviceIoControl(dev, IOCTL_FSCCDRV_GET_FREQ,NULL,0,&freq,sizeof(unsigned long),&res,NULL))
		printf("Clock Generator is set to: %ld\n",freq);
	else
		printf("Error with IOCTL #%d",GetLastError());
	CloseHandle(dev);
	return 0;
}
/* $Id$ */