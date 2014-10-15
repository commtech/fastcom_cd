/* $Id$ */
/*
Copyright(c) 2009, Commtech, Inc.
setfeatures.c -- user mode function to set the onboard feature setting for a FSCC port


usage:
 setfeatures port RxEcho TxD-485 TxC-485 MODE FSTDTR

	The port can be any valid fscc port (0,1)
	RxEcho: Receive Echo Cancel function. 0=Disabled, 1=Enabled
	TxD-485: Transmit Data 422/485 control. 0=TxD is RS422, 1=TxD is RS485 (tristate)
	TxC-485: Transmit Clock Output 422/485 control. 0=TxClkO is RS422, 1=TxClkO is RS485 (tristate)
	MODE: Port Sync/Async control. 0=Port is Synchronous mode, 1=Port is TruAsync mode.
	FSTDTR: FST/DTR pin selection (DB25 pins 18&6). 0 = Pin is FST, 1 = Pin is DTR


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
	ULONG rxecho;
	ULONG sd485;
	ULONG tt485;
	ULONG sync_async;
	unsigned fstdtr;
	
	if(argc!=7) 
	{
		printf("usage:\n");
		printf("%s port RxEcho TxD-485 TxC-485 MODE FSTDTR \n",argv[0]);
		printf("\n");
		printf("\n  port: can be any valid fscc port (0,1,2,etc.)\n");
		printf("\n  RxEcho: Receive Echo Cancel function.\n     0=Disabled, 1=Enabled\n");
		printf("\n  TxD-485: Transmit Data 422/485 control.\n     0=TxD is RS422, 1=TxD is RS485 (tristate)\n");
		printf("\n  TxC-485: Transmit Clock Output 422/485 control.\n     0=TxClkO is RS422, 1=TxClkO is RS485 (tristate)\n");
		printf("\n  MODE: Port Sync/Async control.\n     0=Port is Synchronous mode, 1=Port is TruAsync mode.\n");
		printf("\n  FSTDTR: FST/DTR pin selection (DB25 pins 18&6).\n     0 = Pin is FST, 1 = Pin is DTR\n");
		printf("\n");
		exit(1);
	}
	
	port = atoi(argv[1]);
	rxecho = atol(argv[2]);
	sd485 = atol(argv[3]);
	tt485 = atol(argv[4]);
	sync_async=atol(argv[5]);
	fstdtr = atoi(argv[6]);
	
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
	
	
	desreg = 0;
	if(rxecho     == 1)		desreg|=0x01;
	if(sd485      == 1)		desreg|=0x02;
	if(tt485      == 1)		desreg|=0x04;
	if(fstdtr     == 1)		desreg|=0x08;
	if(sync_async == 1)     desreg|=0x80000000;
	
	if(DeviceIoControl(hDevice,IOCTL_FSCCDRV_SET_FEATURES,&desreg,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		if(rxecho     == 0)		printf("RX allways on\n");
		else					printf("RX echo	cancel ENABLED\n");
		if(sd485      == 0)		printf("TxD is RS-422\n");
		else					printf("TxD is RS-485\n");
		if(tt485      == 0)		printf("TxClkO is RS-422\n");
		else					printf("TxClkO is RS-485\n");
		if(sync_async == 0)     printf("FSCC port in SYNC mode\n");
		else                    printf("FSCC port in ASYNC mode\n");
		if(fstdtr     == 0)		printf("FST/DTR pin is FST\n");
		else					printf("FST/DTR pin is DTR\n");
	}
	else printf("failed:%8.8x\n",GetLastError());
	
	CloseHandle(hDevice);
	return 0;
}



/* $Id$ */