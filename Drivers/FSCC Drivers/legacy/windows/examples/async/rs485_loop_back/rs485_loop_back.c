//$Id$

/************************************
REVISION LOG ENTRY
Revision By: JRD
Revised on 3/16/2009 1:45:55 PM
Comments: Win32 Console application to run a loopback test using rs485 on serial ports using the 
standard win32 COM api.  Baud rate = 115200.

  This program will send 0-0xff to each port, and expect to get it back, as well as a counter in the data.
  
	command line:
	rs485_loop_back X 
	X is the COM port #
	
************************************/

#include "windows.h"
#include "stdio.h"
#include "stdlib.h"
#include "conio.h"
#include "..\fscc_serialp.h"
#define SIZE_TXRX 256

void main(int argc, char *argv[])
{

	HANDLE port;
	ULONG i,j;
	ULONG numwr,numrd;
	ULONG ret,retsz,Sampling,ClockRate,TxTrigger,TxWriteSize;
	ULONG loopcount;
	ULONG errors;
	ULONG Auto485=1,configreg;
	DCB mdcb;
	unsigned startport;
	char tbuffer[1024];
	char rbuffer[1024];
	char buf[128];

	OVERLAPPED osr;
	OVERLAPPED ost;
	COMMTIMEOUTS cto;

	if(argc==2)
	{
		startport = atoi(argv[1]);
		if(startport==0) return;
	}
	else
	{
		printf("USAGE:\n");
		printf("rs485_loop_back X\n");
		printf("\tX is the COM port #\n");
		exit(1);
	}


	printf("rs485_loop_back:\n");
	printf("\tPort: %u\n",startport);


	memset( &osr, 0, sizeof( OVERLAPPED ) ) ;   // Wipe the overlapped structure.
	memset( &ost, 0, sizeof( OVERLAPPED ) ) ;   // Wipe the overlapped structure.

	// Create IO event used for overlapped write

	ost.hEvent = CreateEvent( NULL,				// No security.
					TRUE,						// Explicit reset requirements.
					FALSE,						// Initial event reset.
					NULL ) ;					// No name.
	if (ost.hEvent == NULL)
	   {
		  printf("Failed to create event for thread!\n");
		  return; 
	   }

	// Create IO event used for overlapped read

	osr.hEvent = CreateEvent( NULL,				// No security.
					TRUE,						// Explicit reset requirements.
					FALSE,						// Initial event reset.
					NULL ) ;					// No name.
	if (osr.hEvent == NULL)
	   {
		  printf("Failed to create event for thread!\n");
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
		// Failure here. Die.
		printf("Cannot open handle to: COM%u\n",startport);
		CloseHandle(ost.hEvent);
		CloseHandle(osr.hEvent);
		return;
	}


	DeviceIoControl(port,IOCTL_SERIALP_GET_CONFIG_REG,NULL,0,&configreg,sizeof(ULONG),&retsz,NULL);
	DeviceIoControl(port,IOCTL_SERIALP_SET_AUTO485,&Auto485,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);

	configreg|=0x2;
	//if(Params->PortSettings.RxEchoCancel==1) configreg|=0x4;
	configreg|=0x80000000;
	DeviceIoControl(port,IOCTL_SERIALP_SET_CONFIG_REG,&configreg,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);


	Sampling = 16;								// Sampling = 16X
	// You should always set the sampling before setting the clock, 
	// so that divisors can be correctly calculated.
	
	if(DeviceIoControl(port,					// Here we actually set the sampling.
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
	
	ClockRate = 1843200;						// Now we set the clock.
	if( DeviceIoControl(port,
		IOCTL_SERIALP_SET_CLOCK,
		&ClockRate,
		sizeof(ULONG),
		&ret,
		sizeof(ULONG),
		&retsz,
		NULL) )
	{
		//		printf("Clock set to %d\n",ClockRate);
	}
	else 
	{
		printf("Error with DeviceIOControl IOCTL_SERIALP_SET_CLOCK.\n");
		exit(1);
	}

	GetCommState(port,&mdcb);					// Get the current state of the COM port.
	
	mdcb.BaudRate = 115200;						// Set the baud rate.
	mdcb.fBinary = 1;							// Required.
	mdcb.fParity = 0;							// No parity.
	mdcb.fOutxCtsFlow = 0;						// No control.
	mdcb.fOutxDsrFlow = 0;
	mdcb.fOutX = 0;
	mdcb.fInX = 0;
	mdcb.fRtsControl = 0;
	mdcb.ByteSize = 8;							// 8 bit bytes.
	mdcb.Parity = 0;
	mdcb.StopBits = 0;							// No stop bits.
	
	SetCommState(port,&mdcb);					// Here we actually set the COM port.
	
	SetupComm(port,SIZE_TXRX,SIZE_TXRX);		// Prepare it to send SIZE bytes.
	
	PurgeComm(port,PURGE_TXCLEAR|PURGE_RXCLEAR);// Clear it out.


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
	
	// IMPORTANT: A problem occurs if these are set improperly, particularly at
	// lower baud rates. Be sure to set them.
	
	TxTrigger = 4;
	TxWriteSize = 120;
	DeviceIoControl(port,IOCTL_SERIALP_SET_TX_TRIGGER,&TxTrigger,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
	DeviceIoControl(port,IOCTL_SERIALP_SET_TX_WRITE_SIZE,&TxWriteSize,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
	

	for(i=0;i<SIZE_TXRX;i++) tbuffer[i] = (char)i;
	loopcount = 0;
	errors = 0;

	do
	{
		
		tbuffer[1] = loopcount%250;				// Just a counter in the data.
		ret = WriteFile(port,tbuffer,SIZE_TXRX,&numwr,&ost);
		if(ret==FALSE)
		{
			if(GetLastError()==ERROR_IO_PENDING)
			{
				//printf("COM%u write pending\r\n",startport);
				
				j = WaitForSingleObject(ost.hEvent,1000);	//250 ms delay
				if(j==WAIT_TIMEOUT)
				{
					printf("COM%u: Write timeout.\n",startport);
				}
				else if(j==WAIT_OBJECT_0) 
				{
					//printf("COM%u write successful\r\n",startport);
				}
			}
			else
			{
				printf("\nA write error occured on COM%u.\n",startport);
				printf("Failed with 0x%X error.",GetLastError());
				errors++;
			}
		}
		
		memset( rbuffer, 0, SIZE_TXRX ) ;		// Wipe the RX buffer.
		ret = ReadFile(port,rbuffer,SIZE_TXRX,&numrd,&osr);
		if(ret ==TRUE)
		{
			//printf("com%u received %u bytes:\n\r",startport,numrd);    //display the number of bytes received
			if(numrd!=SIZE_TXRX) 
			{
				printf("\nError in bytes received, count off: %u != %d\n",numrd&0xff,SIZE_TXRX);
				errors++;
			}
			for(j=0;j<numrd;j++)				// Make sure what you send is what you get.
			{
				if(tbuffer[j]!=rbuffer[j])
				{
					errors++;
					printf("\r\nData mismatch on COM%u, %x != %x\r\n",startport,rbuffer[j]&0xff,tbuffer[j]&0xff);
					_getch();
				}
			}
			
		}
		else if((ret==FALSE)&&(GetLastError()==ERROR_IO_PENDING))
		{
			j = WaitForSingleObject( osr.hEvent, 250 );//250 ms timeout
			if(j==WAIT_TIMEOUT)
			{
				errors++;
				printf("\nCOM%u: Read timeout.\n",startport);
			}
			else if(j==WAIT_ABANDONED)
			{
				// Hopefully you won't ever get here.
				errors++;
				printf("\nCOM%u: Read abandoned.\n",startport);
			}
			else if(j==WAIT_OBJECT_0)
			{
				GetOverlappedResult(port,&osr,&numrd,TRUE); // Here to get the actual nobytesread
				//printf("com%u received %u bytes:\n\r",startport,numrd);    //display the number of bytes received
				if(numrd!=SIZE_TXRX) 
				{
					printf("\r\nError in bytes received, count off: %u != %d\n",numrd,SIZE_TXRX);
					errors++;
				}
				for(j=0;j<numrd;j++)			// Check your data.
				{
					if(tbuffer[j]!=rbuffer[j])
					{
						errors++;
						printf("\nData mismatch on COM%u, %x != %x\n",startport,rbuffer[j],tbuffer[j]);
					}
				}
			}
		}
		else
		{
			printf("\nA read error has occured on COM%u.\n",startport);
		}
		
		loopcount++;
		printf("Loop:%lu, Errors:%lu\r",loopcount,errors);	
		// Printing anything will drastically reduce your performance.
		// Remove repetitive printf's to improve performance.
		
	}while(!_kbhit());

	_getch();
	printf("\n\nTotal Loops :%lu\n",loopcount);
	printf("Total Bytes :%lu\n",loopcount*SIZE_TXRX);
	printf("Total Errors:%lu\n",errors);

	CloseHandle(ost.hEvent);
	CloseHandle(osr.hEvent);
	CloseHandle(port);

}
