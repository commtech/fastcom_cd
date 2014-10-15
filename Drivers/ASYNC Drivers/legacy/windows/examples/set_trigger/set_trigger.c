//$Id$
/************************************
  REVISION LOG ENTRY
  Revision By: MDS
  Revised on 11/13/2003
  Comments: Simple program that opens a specified COM port and sets the Tx and Rx 
  trigger values to the specified on the command line.
 ************************************/


#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "..\fcapi.h"


void main(int argc, char *argv[])
{

	HANDLE hDevice=0;
	char buf[256]={0};
	COM_PORT_ENHANCED trig={0};
	ULONG startport = 0;
	ULONG ret=0;	//temp

	memset(&trig,0,sizeof(COM_PORT_ENHANCED));

	if(argc!=5)
	{
		printf("\nUSAGE:\n");
		printf("settrig A B C D\n");
		printf("A = COM number\n");
		printf("B = RxTrigger\n");
		printf("C = TxTrigger\n");
		printf("D = TxWriteSize\n");
		exit(1);
	}
	else
	{
		startport = atoi(argv[1]);	//A
		if(startport==0) return;
		trig.RxFifoTriggerLevel = atoi(argv[2]);	//B
        trig.TxFifoTriggerLevel = atoi(argv[3]);	//C
		trig.TxWriteSize = atoi(argv[4]);
	}

	if( (trig.RxFifoTriggerLevel>63) || (trig.RxFifoTriggerLevel<1) )
	{
		printf("\nRxTrigger out of range 63 <= Trigger <= 1\n");
		exit(1);
	}
	else if( (trig.TxFifoTriggerLevel>63) || (trig.TxFifoTriggerLevel<1) )
	{
		printf("\nTxTrigger out of range 63 <= Trigger <= 1\n");
		exit(1);
	}		
	else if( (trig.TxWriteSize>63) || (trig.TxWriteSize<1) )
	{
		printf("\nTxWriteSize out of range 63 <= TxWriteSize <= 1\n");
		exit(1);
	}
	else if ((trig.TxWriteSize + trig.TxFifoTriggerLevel) > 64)
	{
		printf("\nYou have violated the rule: TxWriteSize + TxFifoTriggerLevel <= 64\n");
		exit(1);
	}

	printf("\nSetting port: COM%u\n",startport);
	
	sprintf(buf,"\\\\.\\COM%u",startport);
	hDevice = CreateFile (buf,
				  GENERIC_READ | GENERIC_WRITE,
				  FILE_SHARE_READ | FILE_SHARE_WRITE,
				  NULL,
				  OPEN_EXISTING,
				  FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
				  NULL
				  );

	if(hDevice== INVALID_HANDLE_VALUE)
	{
		//fubar here
		printf("cannot open handle to COM%u\r\n",startport);
		return;
	}

	if( DeviceIoControl(
		hDevice,					// handle to device
		IOCTL_FCPORT_SET_RXTX_TRIG, // operation
		&trig,						// input data buffer
		sizeof(COM_PORT_ENHANCED),	// size of input data buffer
		NULL,						// output data buffer
		0,							// size of output data buffer
		&ret,						// byte count
		NULL						// overlapped information
	))
	{
		printf("RxTrigger set to %d\n",trig.RxFifoTriggerLevel);
		printf("TxTrigger set to %d\n",trig.TxFifoTriggerLevel);
		printf("TxWriteSize set to %d\n",trig.TxWriteSize);
	}
	else printf("Error with DeviceIOControl\n");

	CloseHandle(hDevice);

}