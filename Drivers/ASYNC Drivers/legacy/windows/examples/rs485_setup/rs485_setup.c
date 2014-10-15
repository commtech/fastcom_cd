//$Id$

/************************************
  REVISION LOG ENTRY
  Revision By: MDS
  Revised on 07/26/2005
  Comments: Simple program that opens a specified COM port and configures automatic 485

  Setting the RxEcho Cancel is optional, but is included as a demonstration.

  Setting the 485 settings will only affect the opened port, however the RxEcho will affect
  all ports on the board.

 ************************************/


#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "..\fcapi.h"


void main(int argc, char *argv[])
{

	HANDLE hDevice=0;
	COM_PORT_ENHANCED esettings;
	ULONG port = 0;
	ULONG ret=0;	//temp
	ULONG retsz=0;
	ULONG onoff = 0;
	CHAR buf[16];

	memset(&esettings,0,sizeof(COM_PORT_ENHANCED));	//wipe esettings sturcture

	if(argc!=3)
	{
USAGE:
		printf("\nUSAGE:\n");
		printf("  rs485_setup.exe A B\n");
		printf("  A = COM number\n");
		printf("  B = 485 On / Off\n\t0 = off\n\t1 = on\n");
		exit(1);
	}
	else
	{
		port = atoi(argv[1]);	//A
		if(port==0) return;
		onoff = atoi(argv[2]);	//B
	}

	if(onoff==0)
	{
		esettings.Auto485 = 0;
		esettings.Auto485TxEn = 0;
		esettings.RxEchoCancel = 0;
		printf("\nAuto485: off\nAuto485TxEnable: off\nRxEchoCancel: off\n");
	}
	else if(onoff==1)
	{
		esettings.Auto485 = 1;
		esettings.Auto485TxEn = 1;
		esettings.RxEchoCancel = 1;
		printf("\nAuto485: on\nAuto485TxEnable: on\nRxEchoCancel: on\n");
	}
	else
		goto USAGE;

	printf("\nConfiguring port: COM%u\n",port);
	
	sprintf(buf,"\\\\.\\COM%u",port);	//device name

	hDevice = CreateFile (buf,		//open handle to device
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
		printf("cannot open handle to COM%u\r\n",port);
		return;
	}

	if( DeviceIoControl(hDevice,
		IOCTL_FCPORT_SET_RS485,
		&esettings,
		sizeof(COM_PORT_ENHANCED),
		&ret,
		sizeof(ULONG),
		&retsz,
		NULL))
	{
		if(esettings.Auto485==0) printf("Auto485 set to off\n");
		else if(esettings.Auto485==1) printf("Auto485 set to on\n");
	}
	else printf("Error with DeviceIOControl IOCTL_FCPORT_SET_RS485\n");

	if( DeviceIoControl(hDevice,
		IOCTL_FCPORT_SET_485_TXEN,
		&esettings,
		sizeof(COM_PORT_ENHANCED),
		&ret,
		sizeof(ULONG),
		&retsz,
		NULL) )
	{
		if(esettings.Auto485TxEn==0) printf("Auto485TxEnable set to off\n");
		else if(esettings.Auto485TxEn==1) printf("Auto485TxEnable set to on\n");
	}
	else printf("Error with DeviceIOControl IOCTL_FCPORT_SET_485_TXEN\n");

	if( DeviceIoControl(hDevice,
		IOCTL_FASTCOM_SET_RXECHO,
		&esettings,
		sizeof(COM_PORT_ENHANCED),
		&ret,
		sizeof(ULONG),
		&retsz,
		NULL))
	{
		if(esettings.RxEchoCancel==0) printf("RxEchoCancel set to off\n");
		else if(esettings.RxEchoCancel==1) printf("RxEchoCancel set to on\n");
	}
	else printf("Error with DeviceIOControl IOCTL_FASTCOM_SET_RXECHO\n");

	printf("RxEchoCancel was set for all ports on the board.\nAuto485 and Auto485TxEnable was only set for COM%d\n",port);

	CloseHandle(hDevice);	//close handle to the open device

}