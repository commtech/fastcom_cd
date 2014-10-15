//$Id$

/************************************
REVISION LOG ENTRY
Revision By: JRD
Revised on 3/16/2009 1:45:55 PM
Comments: Win32 Console application to run a loopback test on serial ports using the 
standard win32 COM api.

  This program will send a file out a port and expect to get it back
  
	command line:
	asyncit V W X Y [Z] 
	V is the COM port #
	W is the baud rate to use
	X is the size of frames to write
	Y is the file to send
	Z is an optional field to allow handshaking or not
	
************************************/


#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "..\fscc_serialp.h"

int create_ports(unsigned port);
void destroy_ports();
void write_packet(char *data_packet,unsigned bytes_to_send);
void write_packet_nowait(char *data_packet,unsigned bytes_to_send);

//globals
DCB dcb;
HANDLE cporth;
OVERLAPPED osw;
OVERLAPPED osw2;
OVERLAPPED *os_use;
OVERLAPPED *os_check;

unsigned cport;
unsigned long speed;
unsigned long blocksize;
unsigned handshaking;
char fname[256];

void main(int argc, char *argv[])
{
	FILE *fin;
	DWORD numbytes;
	DWORD totbytes;
	char *buffer;
	
	if(argc<5)
	{
		printf("Usage:\r\n");
		printf("%s COM speed size file [handshaking]\r\n",argv[0]);
		printf("\tValid parameters are:\n");
		printf("\tCOM			-- COM port# to use\n");
		printf("\tspeed			-- speed of port\n");
		printf("\tsize			-- block size to write\n");
		printf("\tfile			-- file name to process\n");	
		printf("\thandshaking		-- handshaking ON(1) or OFF(0)\n");
		exit(1);
	}
	
	cport = atoi(argv[1]);
	speed = atoi(argv[2]);
	blocksize = atoi(argv[3]);
	if(argv[5]) handshaking = 1;
	
	create_ports(cport);					// Open and initalize port.
	
	if((fin = fopen(argv[4],"rb"))==NULL)	// Open file for reading.
	{
		printf("Cannot open specified file:; %s\r\n",argv[4]);
		exit(1);							// Die if file can't be opened.
	}

	buffer = (char *)malloc(blocksize*2);	// Allocate space.
	if(buffer==NULL)						
	{
		printf("Cannot allocate buffer.\n");
		exit(1);							// Die if allocation failed.
	}
	
	totbytes = 0;
	numbytes = 0;
	numbytes = fread(buffer,sizeof(char),blocksize,fin);
	totbytes += numbytes;
	os_use = &osw2;							// Set up overlapped for transmitting
	write_packet_nowait(buffer, numbytes);	// Write the first packet.
	os_check = &osw2;						// Set up overlapped for next transmit.
	os_use = &osw;							// Set up overlapped for next transmit.
	
	do										// Loop for writing.
	{										// Refill buffer with new data in here.					
		numbytes = 0;
		numbytes = fread(buffer,sizeof(char),blocksize,fin);
		totbytes += numbytes;
		write_packet(buffer, numbytes);		// Send data.
		
	}while((!kbhit())&&(!feof(fin)));		// Loop until keystroke or EOF.
	if(!feof(fin)) getch();					// Clear out the keystroke
	Sleep((((10*blocksize*2)/speed)+1)*1000);// Should wait until the buffer is clear. Not very elegant.
	
	destroy_ports();						// Close port and overlapped events.
	free(buffer);							// Always free after allocating.
	printf("Total bytes sent: %lu\n",totbytes); 
}

void destroy_ports()
{
//	unsigned i;
	CancelIo(cporth);						// Cancel all pending IO
	CloseHandle(osw.hEvent);				// Close your overlapped structs.
	CloseHandle(osw2.hEvent);				// Close your overlapped structs.
	CloseHandle(cporth);					// Close the port.
}

int create_ports(unsigned port)
{
	unsigned i;
	char buf[50];
	ULONG ret=0, TxTrigger=4, TxWriteSize=120, retsz=0;

	sprintf(buf,"\\\\.\\COM%u",port);
	cporth  =   CreateFile( buf, GENERIC_READ | GENERIC_WRITE,
		0,									// Exclusive access.
		NULL,								// No security attributes.
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | 
		FILE_FLAG_OVERLAPPED,				// Use overlapped IO.
		NULL );
	
	SetupComm( cporth, blocksize*2, blocksize*2 );
	i = 0;
											// Create events for overlapped writes.
	memset( &osw, 0, sizeof( OVERLAPPED ) );
	
											// Create IO event used for overlapped read.
	
	osw.hEvent = CreateEvent( NULL,			// No security.
		TRUE,								// Explicit reset requirements.
		FALSE,								// Initial event reset
		NULL ) ;							// No name.
	if (osw.hEvent == NULL)
	{
		printf("Error: Failed to create osw.hEvent.\n");
		return ( FALSE ) ;
	}
	
											// Create events for overlapped writes.
	memset( &osw2, 0, sizeof( OVERLAPPED ) ) ;
	
											// Create I/O event used for overlapped read
	
	osw2.hEvent = CreateEvent( NULL,		// No security.
		TRUE,								// Explicit reset requirements.
		FALSE,								// Initial event reset
		NULL ) ;							// No name.
	if (osw2.hEvent == NULL)
	{
		printf("Error: Failed to create osw2.hEvent.\n");
		return ( FALSE ) ;
	}
	
	// Set the TX_TRIGGER and TX_WRITE_SIZE. If not properly set, this can cause weird problems.
	DeviceIoControl(cporth,IOCTL_SERIALP_SET_TX_TRIGGER,&TxTrigger,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
	DeviceIoControl(cporth,IOCTL_SERIALP_SET_TX_WRITE_SIZE,&TxWriteSize,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);


	//setup dcb structs and comport settings
	dcb.DCBlength = sizeof( DCB ) ;			// Set up DCB structure.
	GetCommState( cporth, &dcb ) ;			// Set up COM port settings.
			
	dcb.BaudRate = speed;					// Set the baud rate.
	dcb.ByteSize = 8 ;						// Set the data bits.
	dcb.Parity = EVENPARITY;				// Even parity.
	dcb.fParity = TRUE;						// Enable parity checking.
	dcb.StopBits =ONESTOPBIT;				// Set one stopbit.
	
	dcb.fBinary = TRUE ;					// Required.
	dcb.fOutxCtsFlow =FALSE;				// No flow control.
	dcb.fOutxDsrFlow =FALSE;				// No flow control.
	dcb.fDtrControl = DTR_CONTROL_DISABLE;	// Not using DTR.
	dcb.fDsrSensitivity = FALSE;			// Not using DSR.
	dcb.fOutX = FALSE;						// Not using XON/XOFF.
	dcb.fInX = FALSE;						// Not using XON/XOFF.
	dcb.fRtsControl = RTS_CONTROL_DISABLE;	// No RTS.
	dcb.fOutxCtsFlow =FALSE;				// No flow control
	if(handshaking==1)  dcb.fRtsControl = RTS_CONTROL_HANDSHAKE; // RTS on.
	if(handshaking==1)  dcb.fOutxCtsFlow =TRUE;					 // Flow control on.
	
	dcb.fNull =FALSE;
	SetCommState(cporth, &dcb );			// Actually set the settings.
											// Now we are good to go to transmit.
	return 0;
}

void write_packet(char *data_packet,unsigned bytes_to_send )
{
	DWORD t;
	DWORD dwBytesWritten;
	DWORD ret;
	OVERLAPPED *temp;
	t = WriteFile( cporth, (LPSTR) data_packet, bytes_to_send,
                           &dwBytesWritten, os_use ) ; // This is where we actually transmit.
   if ((t==FALSE) && (GetLastError() == ERROR_IO_PENDING))
   {
      // Give it a moment for our transmit to complete.
      if ((ret = WaitForSingleObject( os_check->hEvent, (((10*blocksize)/speed)+1)*1000 ))!=WAIT_OBJECT_0)// timeout
	  {
		  // We appear to be timing out.
		  printf("TX stuck: %x\n",GetLastError());
		  dwBytesWritten = 0 ;
		  // Should put some kind of handling here.
		  // I suggest flushing TX and retrying, or aborting and notifying.
	  }
      else
      {
		  // There we go. Write finished.
		  GetOverlappedResult( cporth,
                              os_check,
                              &dwBytesWritten, TRUE ) ;
		  // You could add further error handling in here.
      }
   }
	temp = os_check;	
    os_check = os_use;						// Next time check the one we used now.
	os_use = temp;							// Next time use the free overlapped struct.


}

void write_packet_nowait(char *data_packet,unsigned bytes_to_send )
{
	DWORD t;
	DWORD dwBytesWritten;
	t = WriteFile( cporth, (LPSTR) data_packet, bytes_to_send,
                           &dwBytesWritten, os_use ) ;
	if(t==FALSE)
	{
		t = GetLastError();
		if(t!=0x3e5) printf("Write error: %x\n",t);
	}
}
