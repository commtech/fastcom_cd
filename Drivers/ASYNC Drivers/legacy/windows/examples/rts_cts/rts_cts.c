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
#include "..\fcapi.h"


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
		printf("loop_back X\r\n");
		printf("X is the COM port #\r\n");
		exit(1);
	}


	printf("loop_back:\r\n");
	printf("\tport:%u\r\n",startport);


	memset( &osh, 0, sizeof( OVERLAPPED ) ) ;    //wipe the overlapped struct

	// create I/O event used for overlapped write

	osh.hEvent = CreateEvent( NULL,    // no security
					TRUE,    // explicit reset req
					FALSE,   // initial event reset
					NULL ) ; // no name
	if (osh.hEvent == NULL)
	   {
		  printf("Failed to create event for thread!\r\n");
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
		CloseHandle(osh.hEvent);
		return;
	}


	GetCommState(port,&mdcb);

	mdcb.BaudRate = 115200;
	mdcb.fBinary = 1;
	mdcb.fParity = 0;
	mdcb.fOutxCtsFlow = 0;
	mdcb.fOutxDsrFlow = 0;
	mdcb.fOutX = 0;
	mdcb.fInX = 0;
	mdcb.fRtsControl = RTS_CONTROL_DISABLE;
	mdcb.ByteSize = 8;
	mdcb.Parity = 0;
	mdcb.StopBits = 0;

	SetCommState(port,&mdcb);

	t = GetCommMask(
	  port,		// handle to communications device
	  &event	// event mask
	);
	if(!t)
	{
		printf("GetCommMask reported error:%d",GetLastError() );
		CloseHandle(osh.hEvent);
		CloseHandle(port);
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

	event = 0xffffffff;//EV_CTS;
	t = SetCommMask(
	  port,		// handle to communications device
	  event		// mask that identifies enabled events
	);
	if(!t)
	{
		printf("SetCommMask reported error:%d",GetLastError() );
		CloseHandle(osh.hEvent);
		CloseHandle(port);
	}


	printf("Press any key to cycle RTS and press escape to quit.\n");
	dwFunc = SETRTS;

	do
	{
		t = EscapeCommFunction(
		  port,  // handle to communications device
		  dwFunc   // extended function to perform
		);
		if(!t)
		{
			printf("EscapeCommFunction reported error:%d",GetLastError() );
			CloseHandle(osh.hEvent);
			CloseHandle(port);
		}

		event=0;
		t = WaitCommEvent(
		  port,     // handle to comm device
		  &event,	// event type
		  &osh		// overlapped structure
		);

		if(!t)
		{
			error = GetLastError();
			if(error = ERROR_IO_PENDING)
			{
				j = WaitForSingleObject( osh.hEvent, 1000 );//1000 ms timeout
				if(j==WAIT_TIMEOUT)
				{
					printf("\nCOM%u WAIT_TIMEOUT\n",startport);
					CancelIo(port);
					CloseHandle(osh.hEvent);
					CloseHandle(port);

				}
				else if(j==WAIT_ABANDONED)
				{
					//probably won't ever get here
					printf("\nCOM%u WAIT_ABANDONED\n",startport);
				}
				else if(j==WAIT_OBJECT_0)
				{
					GetOverlappedResult(port,&osh,&event,TRUE); //here to get the actual event
				}
			
			}
			else
			{
				printf("WaitCommEvent reported error:%d",error );
				CloseHandle(osh.hEvent);
				CloseHandle(port);
			}
		}

		if (event & EV_CTS) 
		{
			printf("CTS state change detected\n");
			t = GetCommModemStatus(
			  port,        // handle to communications device
			  &status  // control-register values
			);
			if(!t)
			{
				printf("GetCommModemStatus reported error:%d",GetLastError() );
				CloseHandle(osh.hEvent);
				CloseHandle(port);
			}
			else
			{
				if(status & MS_CTS_ON) printf("The CTS (clear-to-send) signal is on.\n");
				else printf("The CTS (clear-to-send) signal is off.\n");
//				dwFunc = CLRRTS;
			}

		}

	}while( (_kbhit()) && (_getch()!=27) );

	CloseHandle(osh.hEvent);
	CloseHandle(port);

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