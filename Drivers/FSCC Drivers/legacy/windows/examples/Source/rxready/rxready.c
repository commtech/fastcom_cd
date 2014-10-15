/* $Id$ */
/*
	Copyright(C) 2006, Commtech, Inc.

	rxready.c --   a user program to get the number of buffered receive frames
			
usage:
 rxready port

 The port can be any valid fscc port (0,1).
 
*/
#include <windows.h>
#include <stdio.h>
#include "..\fscc.h"


int main(int argc, char * argv[])
{
	HANDLE dev;
	unsigned long t;
	int res;
	char devname[25];

if(argc<2)
{
printf("usage:\n%s port\n",argv[0]);
exit(1);
}
sprintf(devname,"\\\\.\\FSCC%d",atoi(argv[1]));
	dev = CreateFile(devname,GENERIC_WRITE | GENERIC_READ , FILE_SHARE_READ | FILE_SHARE_WRITE,
	NULL,OPEN_EXISTING, 0, NULL);
	
	if(dev== INVALID_HANDLE_VALUE)
	{
		printf("ERROR: Could not open device!\n");
		exit(1);
	}

    DeviceIoControl(dev, IOCTL_FSCCDRV_RX_READY,NULL,0,&t,sizeof(unsigned long),&res,NULL);
	printf("%s:#buffered received frames: %ld\n",devname,t);
	CloseHandle(dev);
	return 0;
}
/* $Id$ */