/*
Copyright(c) 2005, Commtech, Inc.
writereg.c -- user mode function to write a FSCC register

usage:
 writereg port register_offset value [0|1|2]

 The port can be any valid fscc port (0,1) and it doesn't matter which
 port you select, only you must be able to successfuly open it
 (ie can't be in use by another program/user).

 The register_offset can be any in the range 0 to 0x060, and is taken as a
 hex value from the command line.  These are the offsets as given in the 
 FSCC data sheet.
 
 The value is a DWORD value taken in HEX
 
 The [0|1|2] is the BAR to index into
 
 [0] is the FSCC registers [the default if none given]
 [1] is the external UART registers
 [2] is the board specific registers

*/


#include <windows.h>
#include <stdio.h>

#include "..\fscc.h"

int main(int argc, char * argv[])
{
	char nbuf[80];
	int port,t;
	HANDLE hDevice; 
    unsigned long reg,val,temp;
	unsigned long type;
	unsigned long passval[2];
	
	if(argc<4) {
		printf("usage: %s port register_offset value [0|1|2]\n",argv[0]);
		exit(1);
	}
	
	port = atoi(argv[1]);
	sscanf(argv[2],"%lx",&reg);
	sscanf(argv[3],"%lx",&val);
	
	if(argc>4)
	{
		type=atoi(argv[4]);
	}
	else type=0;
	
	passval[0] = reg;
	passval[1] = val;
	
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
	if(type==0)
	{	
		t = DeviceIoControl(hDevice,IOCTL_FSCCDRV_WRITE_REGISTER,&passval,2*sizeof(unsigned long),NULL,0,&temp,NULL);
		if(t)
		{
			printf("BAR0 wrote %8.8lx -> register:%lx\n",passval[1],passval[0]);
		}
		else printf("error in ioctl:%8.8x\n",GetLastError());
	}
	if(type==1)
	{	
		t = DeviceIoControl(hDevice,IOCTL_FSCCDRV_WRITE_REGISTER2,&passval,2*sizeof(unsigned long),NULL,0,&temp,NULL);
		if(t)
		{
			printf("BAR1 wrote %8.8lx -> register:%lx\n",passval[1],passval[0]);
		}
		else printf("error in ioctl:%8.8x\n",GetLastError());
	}
	if(type==2)
	{	
		t = DeviceIoControl(hDevice,IOCTL_FSCCDRV_WRITE_REGISTER3,&passval,2*sizeof(unsigned long),NULL,0,&temp,NULL);
		if(t)
		{
			printf("BAR2 wrote %8.8lx -> register:%lx\n",passval[1],passval[0]);
		}
		else printf("error in ioctl:%8.8x\n",GetLastError());
	}
	
	CloseHandle(hDevice);
	return 0;
}
