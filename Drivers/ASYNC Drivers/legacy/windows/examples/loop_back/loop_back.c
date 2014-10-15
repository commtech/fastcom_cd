//$Id$

/************************************
REVISION LOG ENTRY
Revision By: MDS
Revised on 7/8/2004 1:45:55 PM
Comments: Win32 Console application to run a loopback test on serial ports using the 
standard win32 COM api.  Baud rate = 115200.

  This program will send 0-0xfff to each port, and expect to get it back
  
	command line:
	loop_back X 
	X is the COM port #
	
************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "..\fcapi.h"


void main(int argc, char *argv[])
{
	
	HANDLE port;
	ULONG i,j,waittimeout;
	ULONG numwr,numrd;
	ULONG ret;
	ULONG loopcount;
	ULONG errors;
	DCB mdcb;
	unsigned startport;
	char tbuffer[1024];
	char rbuffer[1024];
	char buf[128];
	
	OVERLAPPED osr;
	OVERLAPPED ost;
	COMMTIMEOUTS cto;
	COM_PORT_ENHANCED esettings;
	
	if(argc==2)
	{
		startport = atoi(argv[1]);
		if(startport==0) return;
	}
	else
	{
		printf("USAGE:\r\n");
		printf("loop_back X\r\n");
		printf("X is the COM port #\r\n");
		exit(1);
	}
	
	
	printf("loop_back:\r\n");
	printf("\tport:%u\r\n",startport);
	
	
	memset( &osr, 0, sizeof( OVERLAPPED ) ) ;    //wipe the overlapped struct
	memset( &ost, 0, sizeof( OVERLAPPED ) ) ;    //wipe the overlapped struct
	memset(&esettings,0,sizeof(COM_PORT_ENHANCED));	//wipe the COM_PORT_ENHANCED struct
	
	// create I/O event used for overlapped write
	
	ost.hEvent = CreateEvent( NULL,    // no security
		TRUE,    // explicit reset req
		FALSE,   // initial event reset
		NULL ) ; // no name
	if (ost.hEvent == NULL)
	   {
		printf("Failed to create event for thread!\r\n");
		return; 
	   }
	
	// create I/O event used for overlapped read
	
	osr.hEvent = CreateEvent( NULL,    // no security
		TRUE,    // explicit reset req
		FALSE,   // initial event reset
		NULL ) ; // no name
	if (osr.hEvent == NULL)
	   {
		printf("Failed to create event for thread!\r\n");
		CloseHandle(ost.hEvent);
		return; 
	   }
	
	sprintf(buf,"\\\\.\\COM%u",startport);
	port = CreateFile (buf,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		NULL
		);
	
	if(port == INVALID_HANDLE_VALUE)
	{
		//fubar here
		printf("cannot open handle to COM%u\r\n",startport);
		CloseHandle(ost.hEvent);
		CloseHandle(osr.hEvent);
		return;
	}
	
	esettings.Sampling = 0;		//sampling = 16X
	//should always set the sampling before setting the clock, so that divisors can be correctly calculated
	if( DeviceIoControl(port,
		IOCTL_FCPORT_SET_SAMPLING,
		&esettings,
		sizeof(COM_PORT_ENHANCED),
		NULL,
		0,
		&ret,
		NULL) )
	{
		//		if(esettings.Sampling==0) printf("Sampling set to 16X\n");
		//		else if(esettings.Sampling==1) printf("Sampling set to 8X\n");
	}
	else 
	{
		printf("Error with DeviceIOControl IOCTL_FCPORT_SET_SAMPLING\n");
		exit(1);
	}
	
	esettings.Clock = 16000000;
	//esettings.Clock = 14745600;
	//set the clock
	if( DeviceIoControl(port,
		IOCTL_FASTCOM_SET_CLOCK,
		&esettings,
		sizeof(COM_PORT_ENHANCED),
		&ret,
		sizeof(COM_PORT_ENHANCED),
		&ret,
		NULL) )
	{
		//		printf("Clock set to %d\n",esettings.Clock);
	}
	else 
	{
		printf("Error with DeviceIOControl IOCTL_FASTCOM_SET_CLOCK\n");
		exit(1);
	}
	
	GetCommState(port,&mdcb);
	
	mdcb.BaudRate = 1000000;
	//	mdcb.BaudRate = 115200;
	mdcb.fBinary = 1;
	mdcb.fParity = 0;
	mdcb.fOutxCtsFlow = 0;
	mdcb.fOutxDsrFlow = 0;
	mdcb.fOutX = 0;
	mdcb.fInX = 0;
	mdcb.fRtsControl = 0;
	mdcb.ByteSize = 8;
	mdcb.Parity = 0;
	mdcb.StopBits = 0;
	
	SetCommState(port,&mdcb);
	
	SetupComm(port,1024,1024);
	
	PurgeComm(port,PURGE_TXCLEAR|PURGE_RXCLEAR);
	
	
	cto.ReadIntervalTimeout = 25;
	cto.ReadTotalTimeoutMultiplier = 2;
	cto.ReadTotalTimeoutConstant = 0;
	cto.WriteTotalTimeoutMultiplier = 2;
	cto.WriteTotalTimeoutConstant = 0;
	/*
	IMPORTANT: refer to the MSDN to determine how to set COMMTIMEOUTS to 
	best suit your needs.
	*/
	SetCommTimeouts(port,&cto);	
	
	
	
	for(i=0;i<256;i++) tbuffer[i] = (char)i;
	loopcount = 0;
	errors = 0;
	
	do
	{
		ret = WriteFile(port,tbuffer,256,&numwr,&ost);
		if(ret==FALSE)
		{
			if(GetLastError()==ERROR_IO_PENDING)
			{
				waittimeout=j=0;
				while(j!=WAIT_OBJECT_0)
				{
					j = WaitForSingleObject(ost.hEvent,1000);	//250 ms delay
					if(j==WAIT_TIMEOUT)
					{
						waittimeout++;
						if(waittimeout>4) break;
						printf("COM%u: Write timeout.\n",startport);
					}
				}
				if(waittimeout>4)
				{
					errors++;
					printf("Writefile failed! Purging buffers and canceling Write()!\n");
					PurgeComm(port, PURGE_TXABORT|PURGE_TXCLEAR);
				}
			}
			else
			{
				printf("\r\na Write ERROR occured on COM%u\r\n",startport);
				printf("Failed with 0x%X error",GetLastError());
				errors++;
			}
		}
		
		memset( rbuffer, 0, 256 ) ;     //wipe the rxbuffer
		ret = ReadFile(port,rbuffer,256,&numrd,&osr);
		if(ret ==TRUE)
		{
			//printf("com%u received %u bytes:\n\r",startport,numrd);    //display the number of bytes received
			if(numrd!=256) 
			{
				printf("\r\nerror in bytes received, count off %u != 256\r\n",numrd);
				errors++;
			}
			for(j=0;j<numrd;j++)
			{
				if(tbuffer[j]!=rbuffer[j])
				{
					errors++;
					printf("\r\ndata mismatch on com%u, %x != %x\r\n",startport,rbuffer[j],tbuffer[j]);
				}
			}
			
		}
		else if((ret==FALSE)&&(GetLastError()==ERROR_IO_PENDING))
		{
			waittimeout=j=0;
			while(j!=WAIT_OBJECT_0)
			{
				j = WaitForSingleObject( osr.hEvent, 250 );//250 ms timeout
				if(j==WAIT_TIMEOUT)
				{
					waittimeout++;
					if(waittimeout>4) break;
					printf("\nCOM%u: Read timeout.\n",startport);
				}
				else if(j==WAIT_ABANDONED)
				{
					// Hopefully you won't ever get here.
					errors++;
					printf("\nCOM%u: Read abandoned.\n",startport);
					break;
				}				
			}
			if(waittimeout>4)
			{
				errors++;
				printf("Readfile failed! Purging buffers and canceling Read()!\n");
				PurgeComm(port, PURGE_RXABORT|PURGE_RXCLEAR);
			}
			GetOverlappedResult(port,&osr,&numrd,TRUE); //here to get the actual nobytesread!!!
			if(numrd!=256) 
			{
				printf("\r\nError in bytes received, count off: %u != 256\n",numrd);
				errors++;
			}
			for(j=0;j<numrd;j++)				// Check your data.
			{
				if(tbuffer[j]!=rbuffer[j])
				{
					errors++;
					printf("\nData mismatch on COM%u, %x != %x\n",startport,rbuffer[j],tbuffer[j]);
				}
			}
		}
		else
		{
			printf("\nA read error has occured on COM%u.\n",startport);
		}
		
		loopcount++;
		printf("Loop:%lu, Errors:%lu\r",loopcount,errors);	
		//printing anything will drastically reduce your performance
		//remove repetitive printf's to improve performance
		
		
	}while(!_kbhit());
	
	_getch();
	printf("\r\n\r\nTotal Loops:%lu\r\n",loopcount);
	printf("Total Bytes :%lu\r\n",loopcount*256);
	printf("Total Errors:%lu\r\n",errors);
	
	CloseHandle(ost.hEvent);
	CloseHandle(osr.hEvent);
	CloseHandle(port);
	
}
