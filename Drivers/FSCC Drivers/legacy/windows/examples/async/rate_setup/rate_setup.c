//$Id$

/************************************
REVISION LOG ENTRY
Revision By: JRD
Revised on 3/16/2009 1:45:55 PM
Comments: Win32 Console application to take the maximum desired baud rate and set the COM ports clock appropriately.
  
	command line:
	rate_setup Y Z
	Y is the COM port #
	Z is the maximum desired baud rate
	
************************************/


#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "..\fscc_serialp.h"


void main(int argc, char *argv[])
{

	HANDLE hDevice;
	ULONG port;
	ULONG rate = 0;
	ULONG ratetemp = 0;
	DWORD Sampling, ClockRate,ret=0,retsz=0;
	CHAR buf[16];

	if(argc!=3)
	{
		printf("Usage:\n");
		printf("rate_setup A B\n");
		printf("\tA = COM number\n");
		printf("\tB = Maximum baud rate");
		exit(1);
	}
	else
	{
		port = atoi(argv[1]);	
		if(port==0) return;
		rate = atoi(argv[2]);
	}

	if( (rate * 16) > 200000000 )		// Greater than maximum at 16X
	{
		if( (rate * 8) > 200000000 )	// Greater than maximum at 8X
		{
			printf("\nData rate %d is out of the maximum range of the board!\n",rate);
			exit(1);					// Die.
		}
		else							// Can't use 16, but 8 will work
		{
			ClockRate = rate * 8;
			Sampling = 8;				// This number can be 4-16, but we use only 16 and 8 in this example
		}
	}
	else if( (rate * 16) < 20000 )		// 16X will work, but clock is too slow
	{
		ratetemp = rate * 16;
		do
		{
			ratetemp *= 2;				// Double the clock rate (desired rate is still divisible)

		}while(ratetemp < 20000);

		ClockRate = ratetemp;			// When the rate is greater than the minimum, we're done
		Sampling = 8;
	}
	else								// Can do as is
	{
		ClockRate = rate * 16;
		Sampling = 0;
	}


	printf("\nSetting port: COM%u\n",port);
	
	sprintf(buf,"\\\\.\\COM%u",port);	// Device name

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
		// Die here. Failure.
		printf("Cannot open handle to: COM%u\n",port);
		return;
	}

	if(DeviceIoControl(hDevice,					// Here we actually set the sampling.
		IOCTL_SERIALP_SET_SAMPLE_RATE,
		&Sampling,
		sizeof(ULONG),
		&ret,
		sizeof(ULONG),
		&retsz,
		NULL) )
	{
		//printf("Sampling set to %dX\n", Sampling);
	}
	else 
	{
		printf("Error with DeviceIOControl IOCTL_SERIALP_SET_SAMPLE_RATE.\n");
		exit(1);
	}

	// Now we set the clock.
	if( DeviceIoControl(hDevice,
		IOCTL_SERIALP_SET_CLOCK,
		&ClockRate,
		sizeof(ULONG),
		&ret,
		sizeof(ULONG),
		&retsz,
		NULL) )
	{
		//printf("Clock set to %d\n",ClockRate);
	}
	else 
	{
		printf("Error with DeviceIOControl IOCTL_SERIALP_SET_CLOCK.\n");
		exit(1);
	}

	printf("Clock and sampling are set for: COM%d.\n",port);

	CloseHandle(hDevice);				// Close handle to the open device

}