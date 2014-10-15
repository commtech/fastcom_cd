//$Id$

/************************************
REVISION LOG ENTRY
Revision By: JRD
Revised on 3/16/2009 1:45:55 PM
Comments: Win32 Console application to run a loopback test on serial ports using the 
standard win32 COM api.  Baud rate = 115200.

  This program will send 0-0xff to each port, and expect to get it back
  
	command line:
	set_trigger W X Y Z
	W is the COM port #
	X is the RxTrigger
	Y is the TxTrigger
	Z is the TxWriteSize
	
************************************/
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "..\fscc_serialp.h"


void main(int argc, char *argv[])
{
	
	HANDLE hDevice=0;
	char buf[256]={0};
	ULONG startport = 0;
	ULONG ret=0,retsz=0;	//temp
	ULONG TxTrigger, TxWriteSize, RxTrigger;
	
	if(argc!=5)
	{
		printf("\nUSAGE:\n");
		printf("set_trigger A B C D\n");
		printf("\tA = COM number\n");
		printf("\tB = RxTrigger\n");
		printf("\tC = TxTrigger\n");
		printf("\tD = TxWriteSize\n");
		exit(1);
	}
	else
	{
		startport = atoi(argv[1]);							// COM port number.
		if(startport==0) return;							// Funky stuff. Exit on COM0.
		RxTrigger = atoi(argv[2]);							// RxTrigger value.
        TxTrigger = atoi(argv[3]);							// TxTrigger value.
		TxWriteSize = atoi(argv[4]);						// TxWriteSize value.
	}
	
	if( (RxTrigger>127) || (RxTrigger<1) )					// Best to keep it in this range.
	{
		printf("\nRxTrigger out of range 127 <= Trigger <= 1\n");
		exit(1);
	}
	else if( (TxTrigger>127) || (TxTrigger<1) )				// Best to keep it in this range.
	{
		printf("\nTxTrigger out of range 127 <= Trigger <= 1\n");
		exit(1);
	}		
	else if( (TxWriteSize>127) || (TxWriteSize<1) )			// Best to keep it in this range.
	{
		printf("\nTxWriteSize out of range 127 <= TxWriteSize <= 1\n");
		exit(1);
	}
	else if( TxTrigger+4 > TxWriteSize)						// This is for everyone's sake.
	{
		printf("\nTxWriteSize must be larger than TxTrigger+4.\n");
	}
	else if ((TxWriteSize + TxTrigger) > 127)				// Best to keep it in this range.
	{
		printf("\nYou have violated the rule: TxWriteSize + TxFifoTriggerLevel <= 127\n");
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
		// Failure here. Die.
		printf("Cannot open handle to: COM%u\n",startport);
		return;
	}
	if(DeviceIoControl(hDevice,IOCTL_SERIALP_SET_TX_TRIGGER,&TxTrigger,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL)) {
		printf("TxTrigger set to %d.\n",TxTrigger);
	}
	else
	{
		printf("Error with: IOCTL_SERIALP_SET_TX_TRIGGER.\n");
	}
	if(DeviceIoControl(hDevice,IOCTL_SERIALP_SET_TX_WRITE_SIZE,&TxWriteSize,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL)) {
		printf("TxWriteSize set to %d.\n",TxWriteSize);
	}
	else
	{
		printf("Error with: IOCTL_SERIALP_SET_TX_WRITE_SIZE.\n");
	}
	if(DeviceIoControl(hDevice,IOCTL_SERIALP_SET_RX_TRIGGER,&RxTrigger,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL)) {
		printf("RxTrigger set to %d.\n",RxTrigger);
	}
	else
	{
		printf("Error with: IOCTL_SERIALP_SET_RX_TRIGGER.\n");
	}
	
	
	CloseHandle(hDevice);
	
}