//$Id$

/************************************
  REVISION LOG ENTRY
  Revision By: MDS
  Revised on 11/13/2003
  Comments: Simple program that opens a specified COM port and sets sampling rate and 
  clock generator.

  Setting the clock generator will affect all ports on the same board.
  Setting the sampling will only affect the opened port.

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
	ULONG rate = 0;
	ULONG ratetemp = 0;
	CHAR buf[16];

	memset(&esettings,0,sizeof(COM_PORT_ENHANCED));	//wipe esettings sturcture

	if(argc!=3)
	{
		printf("USAGE:\n");
		printf("rate_setup A B\n");
		printf("A = COM number\n");
		printf("B = Desired Maximum baud rate");
		exit(1);
	}
	else
	{
		port = atoi(argv[1]);	//A
		if(port==0) return;
		rate = atoi(argv[2]);	//B
	}

	if( (rate * 16) > 50000000 )	//greater than maximum at 16X
	{
		if( (rate * 8) > 50000000 )	//greater than maximum at 8X
		{
			printf("\nData rate %d is out of the maximum range of the board!\n",rate);
			exit(1);
		}
		else	//Can't use 16, but 8 will work
		{
			esettings.Clock = rate * 8;
			esettings.Sampling = 1;
		}
	}
	else if( (rate * 16) < 6000000 )	//16X will work, but clock is too slow
	{
		ratetemp = rate * 16;
		do
		{
			ratetemp *= 2;		//double the clock rate (desired rate is still divisible)

		}while(ratetemp < 6000000);

		esettings.Clock = ratetemp;		//when the rate is greater than the minimum, we're done
		esettings.Sampling = 0;
	}
	else	//can do as is
	{
		esettings.Clock = rate * 16;
		esettings.Sampling = 0;
	}


	printf("\nSetting port: COM%u\n",port);
	
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

	//should always set the sampling before setting the clock, so that divisors can be correctly calculated
	if( DeviceIoControl(hDevice,
		IOCTL_FCPORT_SET_SAMPLING,
		&esettings,
		sizeof(COM_PORT_ENHANCED),
		NULL,
		0,
		&ret,
		NULL) )
	{
		if(esettings.Sampling==0) printf("Sampling set to 16X\n");
		else if(esettings.Sampling==1) printf("Sampling set to 8X\n");
	}
	else printf("Error with DeviceIOControl IOCTL_FCPORT_SET_SAMPLING\n");

	//set the clock
	if( DeviceIoControl(hDevice,IOCTL_FASTCOM_SET_CLOCK,
		&esettings,
		sizeof(COM_PORT_ENHANCED),
		&ret,
		sizeof(COM_PORT_ENHANCED),
		&ret,
		NULL) )
	{
		printf("Clock set to %d\n",esettings.Clock);
	}
	else printf("Error with DeviceIOControl IOCTL_FASTCOM_SET_CLOCK\n");

	printf("Clock is set for all ports on the board.\nSampling is only set for COM%d\n",port);

	CloseHandle(hDevice);	//close handle to the open device

}