/*
Copyright(c) 2010, Commtech, Inc.
setclockbits.c -- user mode function to set the clock using the programming bits

usage:
 setbits 0

 The port can be any valid fscc port (0,1).

*/


#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include "../fscc.h"


int main(int argc, char *argv[])
{
    HANDLE hDevice;/* Handler for the FSCC driver */
	int i;
	char dev[9];
	// This is the programming words for 1 MB/s. Others can be provided.
	unsigned char progwords[20] = {0x01,0xe0,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x9a,0x72,0x48,0x01,0x84,0x01,0xff,0xff,0xff};
	DWORD ret;

	if(argc!=2)
	{
		printf("%s portnum \n\n",argv[0]);
		printf(" The portnum is appended onto \\\\.\\FSCC\n");
		exit(1);
	}
	i=atoi(argv[1]);
	sprintf(dev,"\\\\.\\FSCC%d",i);
	hDevice = CreateFile (dev,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		NULL
		);
	
    if (hDevice == INVALID_HANDLE_VALUE)
    {
		printf("Can't get a handle to fsccpdrv @ %s\n",dev);
		exit(1);
	}
	
	if(DeviceIoControl(hDevice,IOCTL_FSCCDRV_SET_PROGBITS,&progwords,sizeof(progwords),NULL,0,&ret,NULL))
	{
		printf("Progwords set on %s\n",dev);
	}
	else printf("Error in ioctl: %8.8x\n",GetLastError());
	
	CloseHandle(hDevice);
	return 0;
}