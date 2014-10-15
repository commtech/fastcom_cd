/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
setfreq.c -- user mode function to set the OSC input frequency to the FSCC

usage:
 setclock port frequency

 The port can be any valid fscc port (0,1)

 The frequency can be any in the range 20000 to 200000000

If error 0x1C, your PPM is too low.
*/


#include <windows.h>
#include <stdio.h>
#include "..\fscc.h"

int main(int argc, char * argv[])
{
	char nbuf[80];
	int port;
	HANDLE hDevice; 
	ULONG  desfreq;
	unsigned long temp;
	unsigned long ppm;
	unsigned long passval[2];	
	
	if(argc<3) 
	{
        printf("usage: %s port frequency [ppm] \n",argv[0]);
		exit(1);
	}
	
	port = atoi(argv[1]);
	desfreq = atol(argv[2]);
	if(argc>3) ppm = atol(argv[3]);
	else ppm=10;
	
    sprintf(nbuf,"\\\\.\\FSCC%u",port);
	
	printf("Opening: %s\n",nbuf);
	
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
	
	printf("Desired Frequency: %d\n",desfreq);
	passval[0] = desfreq;
	passval[1] = ppm;
	if(argc>3)
	{
		printf("set frequency with user PPM=%d\n",ppm);
		if(DeviceIoControl(hDevice,IOCTL_FSCCDRV_SET_FREQ,&passval[0],2*sizeof(ULONG),NULL,0,&temp,NULL))
		{
			printf("Actual Frequency Set to: %d @ %d ppm\n",desfreq,ppm);
		}
		else printf("failed:%8.8x\r\n",GetLastError());
		
	}
	else
	{
		printf("set frequency with fixed PPM=%d\n",ppm);
		if(DeviceIoControl(hDevice,IOCTL_FSCCDRV_SET_FREQ,&desfreq,sizeof(ULONG),NULL,0,&temp,NULL))
		{
			printf("Actual Frequency Set to: %d\n",desfreq);
		}
		else printf("failed:%8.8x\r\n",GetLastError());
	}
	
	
	CloseHandle(hDevice);
	return 0;
}



/* $Id$ */
