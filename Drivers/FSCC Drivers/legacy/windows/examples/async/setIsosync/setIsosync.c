//$Id$
//Copyright (C) 2007 Commtech, Inc.
//
//Win32 Console application to setup IsoSync using the RI, DSR and
//DTR signals of an ASYNC FSCC port.
//
//

#include "windows.h"
#include "stdio.h"
#include "conio.h"
#include "..\fscc_serialp.h"

int main(int argc,char *argv[])
{
	char devname[64];
	HANDLE hcom;
	ULONG temp,temp2;
	ULONG t;
	BOOL ret;
	if(argc!=3) 
	{
		printf("usage:\n");
		printf("%s port [0|1|2]\n",argv[0]);
		exit(1);
	}
	
	sprintf(devname,"\\\\.\\COM%d",atoi(argv[1]));
	
	hcom= CreateFile(devname,
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(hcom==INVALID_HANDLE_VALUE)
	{
		printf("unable to open %s\n",devname);
		exit(1);
	}
	
	temp = atoi(argv[2]);
	
	switch(temp)
	{
	case 0:
		printf("IsoSync disabled\n");
		break;
	case 1:
		printf("IsoSync Enabled: \n\tReceiver = 1X external clock\n\tTransmitter = 1X external clock\n");
		printf("\tRxClk=DSR pin, TxClkInput=RI pin, TxClkOutput=DTR pin\n\t950 CKS register=0xDD\n");
		break;
	case 2:
		printf("IsoSync Enabled: \n\tReceiver=1X external clock\n\tTransmitter=Normal\n");
		printf("\tRxClk=DSR pin, TxClkInput=baud rate generator, TxClkOutput=DTR pin\n\t950 CKS register=0x19\n");
		break;
	default:
		printf("Invalid selection.  Valid options are 0, 1 & 2.\n");
		printf("\t0: IsoSync disabled\n\n");

		printf("\t1: IsoSync Enabled: \n\tReceiver=1X external clock\n\tTransmitter=1X external clock\n");
		printf("\tRxClk=DSR pin, TxClkInput=RI pin, TxClkOutput=DTR pin\n\t950 CKS register=0xDD\n\n");

		printf("\t2: IsoSync Enabled: \n\tReceiver=1X external clock\n\tTransmitter=Normal\n");
		printf("\tRxClk=DSR pin, TxClkInput=baud rate generator, TxClkOutput=DTR pin\n\t950 CKS register=0x19\n\n");
		
		CloseHandle(hcom);
		exit(1);
	}
	
	ret = DeviceIoControl(hcom,IOCTL_SERIALP_SET_ISOSYNC,&temp,sizeof(ULONG),&temp2,sizeof(ULONG),&t,NULL);
	if(!ret)
	{
		printf("getlasterror:%x\n",GetLastError());
	}
	
	CloseHandle(hcom);
	return 0;	
}
