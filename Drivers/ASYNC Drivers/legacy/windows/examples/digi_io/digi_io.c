//$Id$

/************************************
  REVISION LOG ENTRY
  Revision By: ...
  Revised on 7/8/2004 1:37:53 PM
  Comments: Moved onto CD.  Comments added.
 ************************************/

/************************************
  REVISION LOG ENTRY
  Revision By: MDS
  Revised on 5/26/2004 11:37:47 AM
  Comments: Example program to demonstrate how to utilize the IOCTLs that access the
			digital input/output pins of the Fastcom:232/x-LPCI-335.
			Note:  Digital inputs float to logic high when unconnected.

 ************************************/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "..\fcapi.h"


void main(int argc, char *argv[])
{

	HANDLE hDevice;
	char buf[256];
	COM_PORT_ENHANCED regs={0};
	ULONG startport = 0;
	ULONG retsz;	//temp
	ULONG rw;
	ULONG dout1 = 0;
	ULONG dout2 = 0;
	
	memset(&regs,0,sizeof(COM_PORT_ENHANCED));

	if( (argc>5) || (argc<3) || (argc==4) )
	{
		printf("USAGE:\n");
		printf("digi_io A B [C D]\n");
		printf("A is the COM number\n");
		printf("B read/write select 0=get 1=set\n");
		printf("C DOut1\n");
		printf("D DOut2\n");
		exit(1);
	}
	else if (argc == 5)	//write operation
	{
		startport = atoi(argv[1]);	//A
		if(startport==0) return;
		rw = atoi(argv[2]);			//B
        dout1 = atoi(argv[3]);		//C
		dout2 = atoi(argv[4]);		//D
		if ( (dout1 > 1) || (dout2 > 1) )
		{
			printf("Digital Outputs can only be set to 1 or 0\nExiting\n");
			exit(1);
		}
	}
	else if (argc == 3)	//read operation
	{
		startport = atoi(argv[1]);	//A
		if(startport==0) return;
		rw = atoi(argv[2]);			//B
        regs.DigiIOData = 0;		//C
	}


	printf("Using port: COM%u\n",startport);
	
	sprintf(buf,"\\\\.\\COM%u",startport);
	hDevice = CreateFile (buf,
				  GENERIC_READ | GENERIC_WRITE,
				  FILE_SHARE_READ | FILE_SHARE_WRITE,
				  NULL,
				  OPEN_EXISTING,
				  FILE_ATTRIBUTE_NORMAL,
				  NULL
				  );
	if(hDevice== INVALID_HANDLE_VALUE)
	{
		//fubar here
		printf("cannot open handle to COM%u\r\n",startport);
		return;
	}

	if(rw == 1)	//if write selected
	{
		//display the current status of the output pins
		if(DeviceIoControl(hDevice,			//handle to device
			IOCTL_FCPORT_CURRENT_DIGI_OUT,	//operation
			&regs,							//unused input, only here to preserver structure's data
			sizeof(COM_PORT_ENHANCED),		//sizeof of above input
			&regs,							//output data buffer
			sizeof(COM_PORT_ENHANCED),		//size of above data buffer
			&retsz,							//byte count
			NULL							//overlapped information
			))
		{
			if(regs.DigiIOData & 0x10)
				printf("Current DOut1 = 1\n");
			else printf("Current DOut1 = 0\n");

			if(regs.DigiIOData & 0x20)
				printf("Current DOut2 = 1\n");
			else printf("Current DOut2 = 0\n");

			regs.DigiIOData = 0;
		}

		if (dout1 == 1) regs.DigiIOData |= 0x10;
		if (dout2 == 1) regs.DigiIOData |= 0x20;
		//send the new output settings
		if( DeviceIoControl(
			hDevice,					// handle to device
			IOCTL_FCPORT_SET_DIGI_OUT,	// operation
			&regs,						// input data buffer
			sizeof(COM_PORT_ENHANCED),	// size of input data buffer
			NULL,						// output data buffer
			0,							// size of output data buffer
			&retsz,						// byte count
			NULL						// overlapped information
			))
		{
			printf("data:0x%X\n",regs.DigiIOData);
			printf("setting DOut1 = %d\n", dout1);
			printf("setting DOut2 = %d\n", dout2);
			
		}
		else printf("Error with DeviceIOControl\n");
	}
	else if(rw == 0)
	{
		//Read the current input pins' status
		if( DeviceIoControl(
			hDevice,					//handle to device
			IOCTL_FCPORT_GET_DIGI_IN,	// operation
			&regs,						// input data buffer; unused - only here to preserve data in structure
			sizeof(COM_PORT_ENHANCED),	// size of input data buffer
			&regs,						// output data buffer
			sizeof(COM_PORT_ENHANCED),	// size of output data buffer
			&retsz,						// byte count
			NULL						// overlapped information
			))
		{
			printf("data:0x%X\n",regs.DigiIOData);
			if (regs.DigiIOData & 0x40)
			printf("Reading DIn1 = 1\n");
			else printf("Reading DIn1 = 0\n");

			if (regs.DigiIOData & 0x80)
			printf("Reading DIn2 = 1\n");
			else printf("Reading DIn2 = 0\n");
		}
		else printf("Error with DeviceIOControl\n");
	}


	CloseHandle(hDevice);

}
