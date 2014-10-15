/* $Id$ */
/*
	Copyright(C) 2006, Commtech, Inc.

	txactive.c --   a user program to get the state of the FSCC transmitter
			
usage:
 txactive port

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
	
    DeviceIoControl(dev, IOCTL_FSCCDRV_TX_ACTIVE,NULL,0,&t,sizeof(unsigned long),&res,NULL);
	printf("returned:%x\n",t);
	if(t==1) printf("%s: tx active\n",devname);
	else printf("%s: tx idle\n",devname);
	CloseHandle(dev);
	return 0;
}
/* $Id$ */