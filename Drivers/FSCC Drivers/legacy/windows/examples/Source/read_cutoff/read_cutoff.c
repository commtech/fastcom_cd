/* $Id$ */
/*
Copyright(c) 2007, Commtech, Inc.
read_cutoff.c -- user mode example to show use of IOCTL_FSCCDRV_ALLOW_READ_CUTOFF

usage:
  read_cutoff PORT [1|0] 

  PORT can be any valid FSCC port (0,1) 
  [1|0] enables/disables the read cutoff function.


  This utility can be used in X-SYNC or Transparent mode to receive partial frames, or frames where you have no "end" condition
  If you enable the read cutoff, then when a driver buffer is full it can be returned by a ReadFile asking for the size
  of the buffer.  There will be no receive status appended to the end of this data.
  Set the ReadFile() size == n_rfsize_max setting, and it will return n_rfsize_max bytes when they have been received.

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
	unsigned long ret;
	
	if(argc<2) 
	{
		printf("usage: %s port [1|0]\n",argv[0]);
		exit(1);
	}
	
	port = atoi(argv[1]);
	sprintf(nbuf,"\\\\.\\FSCC%u",port);
	
	
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
	printf("Setting RX cutoff on %s\n",nbuf);
	temp = atol(argv[2]);
	if(temp!=0)temp=1;
	t = DeviceIoControl(hDevice,IOCTL_FSCCDRV_ALLOW_READ_CUTOFF,&temp,sizeof(ULONG),NULL,0,&ret,NULL);
	if(temp==1)printf("ReadFile() will now return partial frames of n_rfsize_max size.\n");
	else if(temp==0)printf("ReadFile() will only return complete frames)\n");
	CloseHandle(hDevice);
	return 0;
}
/* $Id$ */