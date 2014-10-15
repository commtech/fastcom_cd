/*
Copyright(c) 2006, Commtech, Inc.
flushrx.c -- user mode function flush FSCC receiver/buffers

usage:
 flushrx port 

 The port can be any valid FSCC port (0,1) 

*/


#include <windows.h>
#include <stdio.h>
#include <math.h> /* floor, pow */
#include "..\fscc.h"



int main(int argc, char * argv[])
{
	char nbuf[80];
	int port,t;
	HANDLE hDevice; 
    unsigned long temp;
	
	if(argc!=2) 
	{
		printf("usage: %s port\n",argv[0]);
		exit(1);
	}
	
	port = atoi(argv[1]);
	sprintf(nbuf,"\\\\.\\FSCC%u",port);
	
	printf("FLUSHING RX: %s\n",nbuf);
	
	if((hDevice = CreateFile (
		nbuf, 
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		NULL)) == INVALID_HANDLE_VALUE ) 
	{
		printf("Can't get a handle to fsccpdrv @ %s\n",nbuf);
		exit(1);
	}
	
	t = DeviceIoControl(hDevice,IOCTL_FSCCDRV_FLUSH_RX,NULL,0,NULL,0,&temp,NULL);
	CloseHandle(hDevice);
	return 0;
}
