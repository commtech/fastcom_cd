/* $Id$ */
/*
Copyright(c) 2007, Commtech, Inc.
mapping.c -- user mode function to set the character mapping feature (on/off)


usage:
 mapping port [0|1] 

 The port can be any valid fscc port (0,1)

 0 =turn mapping off
 1 =turn mapping on


*/


#include <windows.h>
#include <stdio.h>
#include <math.h> /* floor, pow */
#include "..\fscc.h"


int main(int argc, char * argv[])
{
	//clkset clock;
	char nbuf[80];
	int port;
	HANDLE hDevice; 
	ULONG  desreg;
	unsigned long temp;
	ULONG mapping;


	if(argc<3) 
		{
		printf("usage:\r\n");
		printf("%s port [0|1]\r\n",argv[0]);
		printf("\r\n");
		printf("The port can be any valid fscc port (0,1)\r\n");
		printf("\r\n");
		printf("0==control character mapping off\r\n");
		printf("1==control character mapping on\r\n");
		printf("\r\n");
		exit(1);
		}

	port = atoi(argv[1]);
	mapping = atol(argv[2]);

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



if(DeviceIoControl(hDevice,IOCTL_FSCCDRV_SET_CHARACTER_MAP_ENABLE,&mapping,sizeof(ULONG),NULL,0,&temp,NULL))
	{
	if(mapping    == 0)		printf("Control Character Mapping OFF\r\n");
	else					printf("Control Character Mapping ON\r\n");
	}
else printf("failed:%8.8x\r\n",GetLastError());

CloseHandle(hDevice);
	return 0;
}



/* $Id$ */