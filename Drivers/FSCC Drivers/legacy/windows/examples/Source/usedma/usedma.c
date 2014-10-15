/* $Id$ */
/*
Copyright(c) 2007, Commtech, Inc.
usedma.c -- user mode example to show enable/disable SuperFSCC's DMA

usage:
 usedma.exe port [1|0]

 The port can be any valid FSCC port (0,1) 
  1 - use DMA on SuperFSCC port
  0 - use interrupt only on SuperFSCC (FSCC mode).
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
	
	if(argc<3) 
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
	temp = atol(argv[2]);
	if(temp!=0)temp=1;
	t = DeviceIoControl(hDevice,IOCTL_FSCCDRV_SET_DMA,&temp,sizeof(ULONG),NULL,0,&ret,NULL);
	if(temp==1) printf("DMA mode active\n");
	if(temp==0) printf("IRQ mode active\n");
	CloseHandle(hDevice);
	return 0;
}
/* $Id$ */