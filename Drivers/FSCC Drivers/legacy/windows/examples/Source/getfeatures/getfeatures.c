/* $Id$ */
/*
Copyright(c) 2006, Commtech, Inc.
getfeatures.c -- user mode function to get/display the onboard feature setting for a FSCC port

usage:
 getfeatures port 

 The port can be any valid fscc port (0,1)



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
	
	
	if(argc!=2) 
	{
		printf("usage:  %s port \n",argv[0]);
		exit(1);
	}
	
	port = atoi(argv[1]);
	
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
	
	
	//	Feature register (32 bit register at BAR2) consists of:
	
	//	bit 16: CHA receive echo cancel if '1' RTS controls RD, if '0' RD allways on
	//  bit 17: CHA TT 485 control      if '1'/RTS controls TT, if '0' SD allways on
	//  bit 18: CHA SD 485 control      if '1'/RTS controls SD, if '0' SD allways on
	//	bit 19: CHA FST/DTR control		if '1'DB25 pin is DTR,  if '0' DB25 pin is FST
	//	bit 24: CHA Mode Select			if '1'TrueAsync mode, if '0' Synchronous mode

	//	bit 20: CHB receive echo cancel if '1' RTS controls RD, if '0' RD allways on
	//  bit 21: CHB TT 485 control      if '1'/RTS controls TT, if '0' SD allways on
	//  bit 22: CHB SD 485 control      if '1'/RTS controls SD, if '0' SD allways on
	//	bit 23: CHB FST/DTR control		if '1'DB25 pin is DTR,  if '0' DB25 pin is FST
	//	bit 24: CHB Mode Select			if '1'TrueAsync mode, if '0' Synchronous mode
	
	
	desreg = 0;
	if(DeviceIoControl(hDevice,IOCTL_FSCCDRV_GET_FEATURES,NULL,0,&desreg,sizeof(ULONG),&temp,NULL))
	{
		printf("des:%8.8x\n",desreg);

		if((desreg&1)==1)		printf("RX echo	cancel ENABLED\n");
		else					printf("RX allways on\n");

		if((desreg&2)==2)		printf("SD is RS-485\n");
		else					printf("SD is RS-422\n");

		if((desreg&4)==4)		printf("TT is RS-485\n");
		else					printf("TT is RS-422\n");

		if((desreg&8)==8)		printf("FST/DTR pin is DTR\n");
		else					printf("FST/DTR pin is FST\n");

		if((desreg&0x80000000)!=0) printf("FSCC is in ASYNC mode\n");
		else                       printf("FSCC is in SYNC mode\n");
	}
	else printf("failed:%8.8x\n",GetLastError());
	
	CloseHandle(hDevice);
	return 0;
}



/* $Id$ */