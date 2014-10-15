//$Id$

/************************************
REVISION LOG ENTRY
Revision By: JRD
Revised on 3/16/2009 1:45:55 PM
Comments: Win32 Console application to set up a port for rs485 (or to turn it off).

	command line:
	rs485_setup X Y Z
	X is the COM port #
	Y is 485 [1:0]
	Z is Receive Echo Cancel [1:0]
	
************************************/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "..\fscc_serialp.h"


void main(int argc, char *argv[])
{
	
	HANDLE hDevice=0;
	ULONG ret=0;	//temp
	ULONG port = 0, retsz = 0, onoff = 0, echocancel = 0, Auto485 = 0, configreg = 0;
	CHAR buf[16];
	
	
	if(argc!=4)
	{
USAGE:
	printf("\nUSAGE:\n");
	printf("\trs485_setup A B C\n");
	printf("\tA = COM number\n");
	printf("\tB = 485 On / Off\n\t\t0 = off\t1 = on\n");
	printf("\tC = RxEchoCancel On / Off\n\t\t0 = off\t1 = on\n");
	exit(1);
	}
	else
	{
		port = atoi(argv[1]);			// The port to be altered.
		if(port==0) return;				// If it's 0, there's clearly been a mistake.
		onoff = atoi(argv[2]);			// On or off?
		echocancel = atoi(argv[3]);		// Receive Echo Cancel On or Off. Automatically off if 485 is off.
	}
	sprintf(buf,"\\\\.\\COM%u",port);	// Device name.
	
	hDevice = CreateFile (buf,			// Open handle to the device.
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
		printf("Cannot open handle to: COM%u.\n",port);
		return;
	}
	// Get the current settings. Pretty important. Don't change things you don't need to.
	DeviceIoControl(hDevice,IOCTL_SERIALP_GET_CONFIG_REG,NULL,0,&configreg,sizeof(ULONG),&retsz,NULL);
	
	if(onoff==0)
	{
		printf("\nAuto485: Off");
		printf("\nRxEchoCancel: Off (automatic)");
		configreg&=0x7FFFFFF9;			// This turns off 485 and RxEchoCancel.
		Auto485=0;
	}
	else if(onoff==1)
	{
		printf("\nAuto485: On");
		printf("\nRxEchoCancel: ");
		if(echocancel = 1) {
			printf("On");
			configreg|=0x4;				// This will turn on RxEchoCancel.
		}
		else 
		{
			printf("Off");
			configreg&=0xFFFFFFFB;		// This will turn off RxEchoCancel.
		}
		configreg|=0x80000002;			// This will turn 485 on.
		Auto485=1;						// And so will this.
		
	}
	else
		goto USAGE;						// Something is wrong. Exit.

	DeviceIoControl(hDevice,IOCTL_SERIALP_SET_CONFIG_REG,&configreg,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
	DeviceIoControl(hDevice,IOCTL_SERIALP_SET_AUTO485,&Auto485,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
	printf("\nConfiguring port: COM%u",port);
	printf("\nSettings were set for: COM%u",port);
	
	CloseHandle(hDevice);				// Close handle to the open device.	
}