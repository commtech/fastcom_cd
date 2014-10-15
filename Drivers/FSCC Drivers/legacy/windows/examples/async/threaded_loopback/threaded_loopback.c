//Copyright (C) 1999 Commtech, Inc.
//
//Win32 Console application to run a loopback test on multiple consecutive
//serial ports using the standard win32 COM api.
//mod - threaded, random data, save data to file.
//
//command line:
//looptest X Y Z
//X is the number of ports to include in the loop
//Y is the starting COM port #
//Z is the bitrate to use (115200, 57600, 38400, 19200, 9600, etc)
//This program will send 0-0xff to each port, and expect to get it back
//


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <math.h>
#include "..\fscc_serialp.h"
#include "..\..\Source\fscc.h"

#define BLOCK 1024

DWORD FAR PASCAL WriteProc( LPVOID lpData );
DWORD FAR PASCAL ReadProc( LPVOID lpData );

//BOOL connected;

typedef struct threadVars
{
		HANDLE hdev;			//pointer to device Handle
		DWORD k;
		BOOL connected;
}THREADVARS;

void main(int argc, char *argv[])
{
	
	HANDLE port[25];
	HANDLE fport;
	DWORD i=0;
	DCB mdcb;
	unsigned maxports=0;
	unsigned startport=0;
	char buf[256];
	ULONG bitrate=0;
	THREADVARS threadvars[50];
	HANDLE hwriteThread[25];	//handle to status thread 
	HANDLE hreadThread[25];	//handle to read thread
	DWORD writeID[25];		//status thread ID storage
	DWORD readID[25];		//read thread ID storage
	DWORD dwThreadID=0;		//temp Thread ID storage
	COMMTIMEOUTS cto;
	DWORD ECode=0;
	ULONG ret=0;
	ULONG retsz=0;

	ULONG clockrate = 24000000;
	ULONG SampleRate = 16;
	ULONG TxWriteSize = 65;
	ULONG TxTrigger = 64;
	ULONG RxTrigger = 64;
	ULONG maxrate;
	ULONG datain=0;
	
	maxrate = clockrate/SampleRate;


	if(argc==4)
	{
		maxports = atoi(argv[1]);
		startport = atoi(argv[2]);
		if(maxports==0)return;
		if(startport==0) return;
		bitrate = atol(argv[3]);
	}
	else
	{
		printf("USAGE:\r\n");
		printf("looptest X Y Z\r\n");
		printf("X is the number of ports to include in the loop\r\n");
		printf("Y is the starting COM port #\r\n");
		printf("Z is the bitrate to use (%ld, %ld, %ld, %ld, %ld, etc)\r\n",maxrate/1,maxrate/2,maxrate/3,maxrate/6,maxrate/12);
		exit(1);
	}
	
	if(fmod(((double)(clockrate)/(double)SampleRate),bitrate)!=0.0) 
	{
		printf("cannot obtain bitrate with current settings.\n");
		printf("please use one of:\n");
		printf("%ld\n",maxrate/1);
		printf("%ld\n",maxrate/2);
		printf("%ld\n",maxrate/3);
		printf("%ld\n",maxrate/4);
		printf("%ld\n",maxrate/5);
		printf("%ld\n",maxrate/6);
		printf("or in general %ld/x where x is an integer\n",maxrate);
		exit(1);
	}
	printf("looptest:\r\n");
	printf("starting port:%u\r\n",startport);
	printf("ending port:%u\r\n",startport+maxports);
	printf("bitrate:%lu\r\n",bitrate);

	for(i=0;i<maxports;i++)
	{
		sprintf(buf,"\\\\.\\FSCC%u",i);
		fport = CreateFile (buf,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
			NULL
			);
		
		if(port[i]== INVALID_HANDLE_VALUE)
		{
			//fubar here
			printf("cannot open handle to FSCC%u\r\n",i);
			return;
		}

		datain = 0x80000000;

		if(DeviceIoControl(fport,IOCTL_FSCCDRV_SET_FEATURES,&datain,sizeof(ULONG),NULL,0,&retsz,NULL))
		{
			printf("FSCC%d Features set\n",i);
		}
		else printf("FSCC%d Features set failed:%8.8x\r\n",i,GetLastError());

		CloseHandle(fport);
		
		sprintf(buf,"\\\\.\\COM%u",i+startport);
		port[i] = CreateFile (buf,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
			NULL
			);
		
		if(port[i]== INVALID_HANDLE_VALUE)
		{
			//fubar here
			printf("cannot open handle to COM%u\r\n",i+startport);
			return;
		}

//force port configuration
		
		DeviceIoControl(port[i],IOCTL_SERIALP_SET_CLOCK,&clockrate,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
		DeviceIoControl(port[i],IOCTL_SERIALP_SET_SAMPLE_RATE,&SampleRate,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
		DeviceIoControl(port[i],IOCTL_SERIALP_SET_TX_WRITE_SIZE,&TxWriteSize,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
		DeviceIoControl(port[i],IOCTL_SERIALP_SET_TX_TRIGGER,&TxTrigger,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
		DeviceIoControl(port[i],IOCTL_SERIALP_SET_RX_TRIGGER,&RxTrigger,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);

//configure serial parameters
		GetCommState(port[i],&mdcb);
		mdcb.BaudRate = bitrate;
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
		SetCommState(port[i],&mdcb);
		SetupComm(port[i],BLOCK*8,BLOCK*8);
		
		
//comm timeouts.  unused write(s) timeout at 100ms for reads
		cto.ReadIntervalTimeout = 0;
		cto.ReadTotalTimeoutMultiplier = 0;
		cto.ReadTotalTimeoutConstant = 100;
		cto.WriteTotalTimeoutMultiplier = 0;
		cto.WriteTotalTimeoutConstant = 0;

		SetCommTimeouts(port[i],&cto);

//start fresh
		PurgeComm(port[i],PURGE_TXABORT|PURGE_TXCLEAR|PURGE_RXABORT|PURGE_RXCLEAR);
		}

//create read threads
	for(i=0;i<maxports;i++)
	{

		threadvars[i*2].hdev = port[i];
		threadvars[i*2].k = i;
		threadvars[i*2].connected = TRUE;

		hreadThread[i] =     CreateThread( (LPSECURITY_ATTRIBUTES) NULL,
									   0, 
									   (LPTHREAD_START_ROUTINE) ReadProc,
									   &threadvars[(i*2)],
									   0, &dwThreadID );

		if(hreadThread[i]==NULL)
		{
			printf("cannot start Data read thread\n\r");
			CloseHandle(port[i]);
			exit(1);
		}
		readID[i]=dwThreadID;
	}
//pause for a second
Sleep(1000);
//create write threads
	for(i=0;i<maxports;i++)
	{
	
		threadvars[(i*2)+1].hdev = port[i];
		threadvars[(i*2)+1].k = i;
		threadvars[(i*2)+1].connected = TRUE;


		hwriteThread[i] =     CreateThread( (LPSECURITY_ATTRIBUTES) NULL,
									   0, 
									   (LPTHREAD_START_ROUTINE) WriteProc,
									   &threadvars[(i*2)+1],
									   0, &dwThreadID );

		if(hwriteThread[i]==NULL)
		{
			printf("cannot start Data write thread\n\r");
			CloseHandle(port[i]);
			exit(1);
		}
		writeID[i]=dwThreadID;

	}
	
//wait for the user to hit a key to exit program.	
	
	while ( !_kbhit() );
	_getch();

	printf("stopping\n");
	for(i=0;i<maxports;i++)
	{
		//shutdown write thread
		ECode=0;
		threadvars[(i*2)+1].connected=FALSE;
		do{
			GetExitCodeThread(hwriteThread[i],&ECode);
		}while(ECode != 0xf);
		CloseHandle (hwriteThread[i]);                //done with this thread
		//shutdown read thread
		ECode=0;
		threadvars[(i*2)].connected=FALSE;
		do{
			GetExitCodeThread(hreadThread[i],&ECode);
		}while(ECode != 0xf);
		CloseHandle (hreadThread[i]);                //done with this thread
		//done with comport
		CloseHandle(port[i]);
	}	
//finished!
}

//thread for issuing writes to comport
DWORD FAR PASCAL WriteProc( LPVOID lpData )
{
	HANDLE tdev;		//handle to the fsccdrv device
	THREADVARS *tthreadvar;
	OVERLAPPED ost;
	DWORD numtd;
	DWORD ret;
	char tbuffer[BLOCK];
	DWORD j;
	FILE *fout;
	char filename[25];
	unsigned long bttotal;
	
	bttotal=0;

	srand( (unsigned)time( NULL ) );

	memset( &ost, 0, sizeof( OVERLAPPED ) ) ;    //wipe the overlapped struct
	
	// create I/O event used for overlapped read
	
	ost.hEvent = CreateEvent( NULL,    // no security
		TRUE,    // explicit reset req
		FALSE,   // initial event reset
		NULL ) ; // no name
	if (ost.hEvent == NULL)
	{
		printf("Failed to create event for thread!\r\n");
		exit(1); 
	}
	
	//pull in our port handle and #
	tthreadvar = (THREADVARS*)lpData;
	
	sprintf(filename,"writefile%d",tthreadvar->k);

	fout=fopen(filename,"wb");
	if(fout==NULL)
	{
		printf("cannot open output file %s\n",filename);
		exit(1);
	}
	
	tdev = tthreadvar->hdev;
	memset( tbuffer, 0, BLOCK ) ;     //wipe the txbuffer
	//enter main loop, send data continuously
	do
	{
		//fill data buffer
		tbuffer[0] = (char)tthreadvar->k;
		for(j=1;j<BLOCK;j++) tbuffer[j] = (char)(rand());
		//send it out the port
		ret = WriteFile(tdev,tbuffer,BLOCK,&numtd,&ost);
		if(ret==FALSE)
		{
			if(GetLastError()==ERROR_IO_PENDING)
			{
				//blocked, wait for it to finish
				do{
					j = WaitForSingleObject(ost.hEvent,1000);
					if(j==WAIT_TIMEOUT)
					{
						//wait timed out, kill it and try again.
						printf("#%u write timeout\r\n",tthreadvar->k);
						PurgeComm(tdev,PURGE_TXABORT|PURGE_TXCLEAR);
					}
				}while(j!=WAIT_OBJECT_0);
			GetOverlappedResult(tdev,&ost,&numtd,TRUE);
			}
			else
			{
				//if not iopending, then is some kind of error!
				printf("\r\na Write ERROR occured on #%u ERROR#%d\r\n",tthreadvar->k,GetLastError());
			}
		}
		//here numtd is the number of bytes written (as from WriteFile returning true, or from the GetOverlappedResult if it blocked)
		if(numtd!=0)
			{
			//stash bytes written to file
			fwrite(tbuffer,1,numtd,fout);
			fflush(fout);
			}
		bttotal+=numtd;
		//do it all again! (until we get signaled to stop)
	}
	while(tthreadvar->connected);
	
	//done with output file
	fclose(fout);
	//done with overlapped event
	CloseHandle(ost.hEvent);
	//Display # bytes transmitted
	printf("[%d] wrote %ld bytes\n",tthreadvar->k,bttotal);
	//signal our completion
	ExitThread(0xf);
	//done!
	return 0;
}

//Read thread
DWORD FAR PASCAL ReadProc( LPVOID lpData )
{
	HANDLE rdev;		//handle to the fsccdrv device
	THREADVARS *rthreadvar;
	OVERLAPPED osr;
	DWORD numrd;
	DWORD ret;
	char rbuffer[BLOCK];
	DWORD j;
	FILE *rout;
	char filename[25];
	int numwritten;
	unsigned long bttotal;
	BOOL stop=TRUE;

	bttotal=0;
	memset( &osr, 0, sizeof( OVERLAPPED ) ) ;    //wipe the overlapped struct
	
	// create I/O event used for overlapped read
	
	osr.hEvent = CreateEvent( NULL,    // no security
		TRUE,    // explicit reset req
		FALSE,   // initial event reset
		NULL ) ; // no name
	if (osr.hEvent == NULL)
	{
		printf("Failed to create event for thread!\r\n");
		exit(1); 
	}

	//get passed in comport handle and port #
	rthreadvar = (THREADVARS*)lpData;
	
	sprintf(filename,"readfile%d",rthreadvar->k);

	//open up a file to stash the data received
	rout=fopen(filename,"wb");
	if(rout==NULL)
	{
		printf("cannot open output file %s\n",filename);
		exit(1);
	}
	
	rdev = rthreadvar->hdev;
	memset( rbuffer, 0, BLOCK ) ;     //wipe the rxbuffer	
	//main loop receive data, stash to file
	do
	{
		ret = ReadFile(rdev,rbuffer,BLOCK,&numrd,&osr);
		if(ret ==TRUE)
		{
//			printf("non-blocked #%u received %u bytes:\n",rthreadvar.k,numrd);    //display the number of bytes received
			if(numrd!=BLOCK) 
			{
				printf("\r\nerror in bytes received, count off %u != %d\r\n",numrd,BLOCK);
			}
			if(numrd!=0)
			{
				fwrite(rbuffer,1,numrd,rout);
				fflush(rout);
			}
		bttotal+=numrd;	
		}
		else if((ret==FALSE)&&(GetLastError()==ERROR_IO_PENDING))
		{
			do{
			j = WaitForSingleObject( osr.hEvent, 2000 );
			if(j==WAIT_TIMEOUT)
			{
				printf("\r\n#%u Read timeout\r\n",rthreadvar->k);
				if (rthreadvar->connected==FALSE) 
				{
					//if the wait times out, purge it to finish/close out the read
					PurgeComm(rdev,PURGE_RXABORT|PURGE_RXCLEAR);
					//we might as well drop out because if the read times out here we are not receiving data anymore
					stop=FALSE;
				}
			}
			if(j==WAIT_ABANDONED)
			{
				printf("\r\n#%u Read abandoned\r\n",rthreadvar->k);
			}
			}
			while(j!=WAIT_OBJECT_0);
			{
				GetOverlappedResult(rdev,&osr,&numrd,TRUE); //here to get the actual nobytesread!!!
//				printf("blocked #%u received %u bytes:\t",rthreadvar.k,numrd);    //display the number of bytes received
				if(numrd!=BLOCK) 
				{
					if((numrd==0)&&(rthreadvar->connected==FALSE)) stop=FALSE;//if we are running and we get a commtimeout, (completed read with 0 bytes) and the main program told us to stop, then exit
					else 
					{
						if((bttotal==0)&&(numrd==0)) printf("waiting for data:%d\n",rthreadvar->k);//if we haven't received any bytes yet then we are waiting for the transmit thread to startup
						else printf("\r\nerror in bytes received, count off %u != %d\r\n",numrd,BLOCK);//otherwise its an error
					}

				}
				if(numrd!=0)
				{
					//stash the bytes in a file
					numwritten=fwrite(rbuffer,1,numrd,rout);
					fflush(rout);
				}
			}
		bttotal+=numrd;
		}
		else
		{
			printf("\r\na read error#%d has occured on #%u\r\n",GetLastError(),rthreadvar->k);
		}
    //do it again! until we hit a stop condition
	}
	while(stop);

	printf("[%d] read %ld bytes\n",rthreadvar->k,bttotal);
	//done with the file
	fclose(rout);
	//done with the overlapped event
	CloseHandle(osr.hEvent);
	//just to be sure...
	CancelIo(rdev);
	//signal that this thread is done
	ExitThread(0xf);
	//done!
	return 1;
}