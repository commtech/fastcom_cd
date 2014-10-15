//$Id

/************************************
  REVISION LOG ENTRY
  Revision By: MDS
  Revised on 3/24/2006 9:55 AM
  Comments: Win32 Console application to run a loopback test the RTS and CTS signals

			This program will change the state of RTS and detect that state on CTS

			command line:
			rts_cts X 
			X is the COM port #

 ************************************/

#include "windows.h"
#include "stdio.h"
#include "stdlib.h"
#include "conio.h"
#include "..\fscc_serialp.h"


void main(int argc, char *argv[])
{

	HANDLE port;
	DCB mdcb;
	unsigned startport;
	char buf[128];
	
	OVERLAPPED osh;
	BOOL t;
	DWORD event;
	DWORD status;
	DWORD dwFunc;
	DWORD error;
	DWORD j;

	if(argc==2)
	{
		startport = atoi(argv[1]);
		if(startport==0) return;
	}
	else
	{
		printf("USAGE:\r\n");
		printf("rts_cts X\r\n");
		printf("X is the COM port #\r\n");
		exit(1);
	}


	printf("loop_back:\r\n");
	printf("\tport:%u\n",startport);


	memset( &osh, 0, sizeof( OVERLAPPED ) ) ;			// Wipe the overlapped structure.	

	// Create IO event used for overlapped write.

	osh.hEvent = CreateEvent( NULL,						// No security.
					TRUE,								// Explicit reset requirements.
					FALSE,								// Initial event reset.
					NULL ) ;							// No name.
	if (osh.hEvent == NULL)
	   {
		  printf("Failed to create event for thread!\n");
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
		CloseHandle(osh.hEvent);
		return;
	}


	GetCommState(port,&mdcb);							// Always get the state before making assumptions.

	mdcb.BaudRate = 115200;								// Set the baud.
	mdcb.fBinary = 1;									// Required.
	mdcb.fParity = 0;									// No parity.
	mdcb.fOutxCtsFlow = 0;								// No flow control.
	mdcb.fOutxDsrFlow = 0;
	mdcb.fOutX = 0;
	mdcb.fInX = 0;
	mdcb.fRtsControl = RTS_CONTROL_DISABLE;
	mdcb.ByteSize = 8;									// 8 bit bytes.
	mdcb.Parity = 0;
	mdcb.StopBits = 0;

	SetCommState(port,&mdcb);							// Set everything here.

	t = GetCommMask(
	  port,												// Handle to the communications device.
	  &event											// Event mask.
	);
	if(!t)												// Something went wrong.
	{
		printf("GetCommMask got error: %d",GetLastError() ); // Display error message.
		CloseHandle(osh.hEvent);						// Always close everything you can.
		CloseHandle(port);								// Close the port.
	}
	else
	{
		printf("Masked events:\n\tNone\r");
		if(event & EV_BREAK) printf("\tEV_BREAK: A break was detected on input.\n");
		if(event & EV_CTS) printf("\tEV_CTS: The CTS (clear-to-send) signal changed state.\n");
		if(event & EV_DSR) printf("\tEV_DSR: The DSR (data-set-ready) signal changed state.\n");
		if(event & EV_ERR) printf("\tEV_ERR: A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY.\n");
		if(event & EV_EVENT1) printf("\tEV_EVENT1: An event of the first provider-specific type occurred.\n");
		if(event & EV_EVENT2) printf("\tEV_EVENT2: An event of the second provider-specific type occurred.\n");
		if(event & EV_PERR) printf("\tEV_PERR: A printer error occurred.\n");
		if(event & EV_RING) printf("\tEV_RING: A ring indicator was detected.\n");
		if(event & EV_RLSD) printf("\tEV_RLSD: The RLSD (receive-line-signal-detect) signal changed state.\n");
		if(event & EV_RX80FULL) printf("\tEV_RX80FULL: The receive buffer is 80 percent full.\n");
		if(event & EV_RXCHAR) printf("\tEV_RXCHAR: A character was received and placed in the input buffer.\n");
		if(event & EV_RXFLAG) printf("\tEV_RXFLAG: The event character was received and placed in the input buffer. The event character is specified in the device's DCB structure, which is applied to a serial port by using the SetCommState function.\n");
		if(event & EV_TXEMPTY) printf("\tEV_TXEMPTY: The last character in the output buffer was sent.\n");
		printf("\n");
	}

	event = 0xffffffff;									// EV_CTS;
	t = SetCommMask(
	  port,												// Handle to the communications device.
	  event												// Mask that identifies enabled events.
	);
	if(!t)
	{
		printf("SetCommMask got error: %d",GetLastError() ); // Something went wrong.
		CloseHandle(osh.hEvent);						// Close open handles.
		CloseHandle(port);								// Close.
	}


	printf("Press any key to cycle RTS and press escape to quit.\n");
	dwFunc = SETRTS;

	do
	{
		t = EscapeCommFunction(
		  port,											// Handle to the communications device.
		  dwFunc										// Extended function to perform.
		);
		if(!t)											// Something went wrong.
		{
			printf("EscapeCommFunction reported error: %d",GetLastError() ); // Report error.
			CloseHandle(osh.hEvent);					// Close all open handles.
			CloseHandle(port);
		}

		event=0;
		t = WaitCommEvent(
		  port,											// Handle to the COM device.
		  &event,										// Event type.
		  &osh											// Overlapped structure to use.
		);

		if(!t)											// Something went wrong.
		{
			error = GetLastError();						// Retreive error.
			if(error = ERROR_IO_PENDING)				// Not really an error.
			{
				j = WaitForSingleObject( osh.hEvent, 1000 ); // 1000ms timeout
				if(j==WAIT_TIMEOUT)
				{
					printf("\nCOM%u got error: WAIT_TIMEOUT\n",startport); // Now it' an error.
					CancelIo(port);						// Cleaning time!
					CloseHandle(osh.hEvent);			// Handle and exit.
					CloseHandle(port);					// Or maybe not exit.

				}
				else if(j==WAIT_ABANDONED)
				{
					// You probably won't ever get here.
					printf("\nCOM%u got error: WAIT_ABANDONED\n",startport);
				}
				else if(j==WAIT_OBJECT_0)
				{
					GetOverlappedResult(port,&osh,&event,TRUE); // Here we finish.
				}
			}
			else
			{
				printf("WaitCommEvent got error: %d",error );
				CloseHandle(osh.hEvent);				// Close it all.
				CloseHandle(port);
			}
		}

		if (event & EV_CTS)								// If you're watching CTS..
		{
			printf("CTS state change detected!\n");		// Something happened!
			t = GetCommModemStatus(
			  port,										// Handle to the communications device.
			  &status									// Control-register values.
			);
			if(!t)										// Something went wrong.
			{
				printf("GetCommModemStatus got error: %d",GetLastError() ); // Report error.
				CloseHandle(osh.hEvent);				// Close it up.
				CloseHandle(port);
			}
			else
			{
				if(status & MS_CTS_ON) printf("The CTS (clear-to-send) signal is on.\n");
				else printf("The CTS (clear-to-send) signal is off.\n");	// Report information.
//				dwFunc = CLRRTS;
			}

		}

	}while( (_kbhit()) && (_getch()!=27) );				// Wait until the ESC key is hit.

	CloseHandle(osh.hEvent);							// Close it all up.
	CloseHandle(port);									// Close it.
}


/*
From MSDN:

The following example code opens the serial port for overlapped I/O, 
creates an event mask to monitor CTS and DSR signals, and then waits 
for an event to occur. The WaitCommEvent function should be executed 
as an overlapped operation so the other threads of the process cannot 
perform I/O operations during the wait. 

HANDLE hCom;
OVERLAPPED o;
BOOL fSuccess;
DWORD dwEvtMask;

hCom = CreateFile( "COM1",
    GENERIC_READ | GENERIC_WRITE,
    0,    // exclusive access 
    NULL, // no security attributes 
    OPEN_EXISTING,
    FILE_FLAG_OVERLAPPED,
    NULL
    );

if (hCom == INVALID_HANDLE_VALUE) 
{
    // Handle the error. 
}

// Set the event mask. 

fSuccess = SetCommMask(hCom, EV_CTS | EV_DSR);

if (!fSuccess) 
{
    // Handle the error. 
}

// Create an event object for use in WaitCommEvent. 

o.hEvent = CreateEvent(
    NULL,   // no security attributes 
    FALSE,  // auto reset event 
    FALSE,  // not signaled 
    NULL    // no name 
    );

assert(o.hEvent);

if (WaitCommEvent(hCom, &dwEvtMask, &o)) 
{
    if (dwEvtMask & EV_DSR) 
    {
         // To do.
    }

    if (dwEvtMask & EV_CTS) 
    {
         // To do. 
    }
}
*/