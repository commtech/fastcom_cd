/* $Id$ */
/*
Copyright(c) 2005, Commtech, Inc.
readreg.c -- user mode function to read a FSCC register

usage:
 readreg port register_offset [0|1|2]

 The port can be any valid fscc port (0,1) and it doesn't matter which
 port you select, only you must be able to successfuly open it
 (ie can't be in use by another program/user).

 The register_offset can be any in the range 0 to 0x060 (for [0]), 0 to 7 for [1], and 0 for [2],
 and is taken as a hex value from the command line.  This is the register offset as 
 given in the FSCC data sheet.
 The [0|1|2] is the BAR to offset into.  
   [0] is the default and is the FSCC registers
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
	if(argc<3) {
                printf("usage: %s port register [0|1|2]\n",argv[0]);
		exit(1);
	}

	port = atoi(argv[1]);
    sscanf(argv[2],"%lx",&reg);

if(argc>3)
	{
	type=atoi(argv[3]);
	}
else type=0;

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
    t = DeviceIoControl(hDevice,IOCTL_FSCCDRV_READ_REGISTER,&reg,sizeof(unsigned long),&val,sizeof(unsigned long),&temp,NULL);
    if(t)
		{
		if(temp!=0)  printf("BAR0 in from register:%lx -> %8.8lx\n",reg,val);
		else printf("Problem reading BAR0 register:%lx.\n",reg);/* check owners manual */
		}
	else printf("Error in ioctl:%8.8x\n",GetLastError());
	}
else if(type==1)
	{
    t = DeviceIoControl(hDevice,IOCTL_FSCCDRV_READ_REGISTER2,&reg,sizeof(unsigned long),&val,sizeof(unsigned long),&temp,NULL);
    if(t)
		{
		if(temp!=0)  printf("BAR1 in from register:%lx -> %8.8lx\n",reg,val);
		else printf("Problem reading BAR1 register:%lx.\n",reg);/* check owners manual */
		}
	else printf("Error in ioctl:%8.8x\n",GetLastError());
	}
else if(type==2)
	{
    t = DeviceIoControl(hDevice,IOCTL_FSCCDRV_READ_REGISTER3,&reg,sizeof(unsigned long),&val,sizeof(unsigned long),&temp,NULL);
    if(t)
		{
		if(temp!=0)  printf("BAR2 in from register:%lx -> %8.8lx\n",reg,val);
		else printf("Problem reading BAR2 register:%lx.\n",reg);/* check owners manual */
		}
	else printf("Error in ioctl:%8.8x\n",GetLastError());
	}

	CloseHandle(hDevice);
	return 0;
}

/* $Id$ */