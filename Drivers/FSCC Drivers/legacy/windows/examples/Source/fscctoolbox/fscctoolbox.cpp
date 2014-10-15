// fscctoolbox.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "fscctoolbox.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


ULONG refcount = 0;

static DWORD fscclasterror[MAXPORTS]={0};

static HANDLE hFSCC[MAXPORTS] = {NULL};
BOOL connected[MAXPORTS] = {0};
DWORD statusmask[MAXPORTS] = {0};
DWORD readsize[MAXPORTS] = {0};
DWORD maxwritesize[MAXPORTS] = {0};
HANDLE hReadThread[MAXPORTS] = {NULL};
HANDLE hWriteThread[MAXPORTS] = {NULL};
HANDLE hStatusThread[MAXPORTS] = {NULL};
static DWORD refcnt[MAXPORTS] = {0};
HMODULE mymod[MAXPORTS]= {NULL};




DWORD FAR PASCAL StatusProc( DWORD port );
DWORD FAR PASCAL ReadProc( DWORD port );
DWORD FAR PASCAL WriteProc( DWORD port );



// Function name	: FSCCToolBox_Create
// Description	    : Opens handle to the FSCC port specified in the port variable
// Return type		: DWORD --return status
// Argument         : DWORD port --port number FSCC#
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Create(INPUT DWORD port)
{
	char devname[25];
	ULONG onoff;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;	//exceeded number of allowable ports
	if(hFSCC[port]!=NULL) return PORT_IS_OPEN;	//port already open
	
	sprintf(devname,"\\\\.\\FSCC%d",port);
	
    hFSCC[port] = CreateFile (devname,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		NULL
		);
	
    if (hFSCC[port] == INVALID_HANDLE_VALUE)
    {
		//	MessageBox(NULL,"create failed\n","",MB_OK);
		fscclasterror[port]=GetLastError();
		return CREATEFILE_FAILED;	//something bad happened
	}
	onoff=0;
	DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_BLOCK_MULTIPLE_IO,&onoff,sizeof(ULONG),NULL,0,&temp,NULL);
	//sets the driver to allow a particular port to be opened in more than one handle
	
	return FSCCTOOLBOX_SUCCESS;
	
}


// Function name	: FSCCToolBox_Destroy
// Description	    : Stops and closes the status, write and read threads and closes
//					  handle to specified port
// Return type		: DWORD --return status
// Argument         : DWORD port --port number i.e. FSCC#
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Destroy(INPUT DWORD port)
{
	DWORD ret;
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	
	connected[port]=0;	//will cause any threads running on this port to stop

	if(hReadThread[port]!=NULL)
	{
		DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_CANCEL_RX,NULL,0,NULL,0,&ret,NULL);
		//causes a blocked ReadFile to return canceled
		Sleep(0);
		CloseHandle(hReadThread[port]);
	}
	if(hWriteThread[port]!=NULL)
	{
		DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_CANCEL_TX,NULL,0,NULL,0,&ret,NULL);
		//causes a blocked WriteFile to return canceled
		Sleep(0);
		CloseHandle(hWriteThread[port]);
	}
	if(hStatusThread[port]!=NULL)
	{
		DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_CANCEL_STATUS,NULL,0,NULL,0,&ret,NULL);
		//causes a blocked status request to return canceled
		Sleep(0);
		CloseHandle(hStatusThread[port]);
	}
	CloseHandle(hFSCC[port]);
	hReadThread[port]=NULL;
	hWriteThread[port]=NULL;
	hStatusThread[port]=NULL;
	hFSCC[port]=NULL;
	return FSCCTOOLBOX_SUCCESS;
	
}


// Function name	: FSCCToolBox_Set_Clock
// Description	    : Sets the programmable clock generator (ICS307-02) that supplies
//					  the OSC clock input to the serial controller
// Return type		: DWORD --return status
// Argument         : DWORD port --port number i.e. FSCC#
// Argument         : DWORD frequency --desired clock generator frequency in Hz
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Set_Clock(INPUT DWORD port,INPUT DWORD frequency)
{
	DWORD temp;
	ULONG actualf;
	ULONG passval[2];
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	passval[0] = frequency;
	passval[1] = 2;//PPM setting...increase if issues in setting
	if(	DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_SET_FREQ,&frequency,sizeof(ULONG),&actualf,sizeof(ULONG),&temp,NULL)!=0)
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}


// Function name	: FSCCToolBox_Get_Clock
// Description	    : Returns the frequency of the clock generator
// Return type		: DWORD --return status
// Argument         : DWORD port --port number i.e. FSCC#
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Get_Clock(INPUT DWORD port, OUTPUT DWORD *frequency)
{
	DWORD res;
	ULONG passval;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	if(DeviceIoControl(hFSCC[port], IOCTL_FSCCDRV_GET_FREQ,NULL,0,&passval,sizeof(ULONG),&res,NULL)!=0)
	{
		frequency[0] = passval;
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}


// Function name	: FSCCToolBox_Read_Frame
// Description	    : Standard read routing that does not callback
// Return type		: FSCCTOOLBOX_API DWORD 
// Argument         : DWORD port --FSCC# port number
// Argument         : char * rbuf --pointer to user variable where data is stored
// Argument         : DWORD szrbuf --NumBytesToRead
// Argument         : DWORD *retbytes --NumbBytesRead
// Argument         : DWORD timeout --number of miliseconds for WaitForSingleObject
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Read_Frame(INPUT DWORD port,OUTPUT char * rbuf,INPUT DWORD szrbuf,OUTPUT DWORD *retbytes,INPUT DWORD timeout)
{
	ULONG t;
	OVERLAPPED  rq;
	int j;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	memset( &rq, 0, sizeof( OVERLAPPED ) ) ;	//wipe the overlapped struct
	rq.hEvent = CreateEvent( NULL,TRUE,FALSE,NULL );
	if (rq.hEvent == NULL)
	{
		fscclasterror[port]=GetLastError();
		return OVERLAPPED_EVENT_CREATE_FAILED;
	}
	
	t = ReadFile(hFSCC[port],rbuf,szrbuf,retbytes,&rq);
	if(t==FALSE)  
	{
		t=GetLastError();
		if(t==ERROR_IO_PENDING)
		{
			do
			{
				j = WaitForSingleObject( rq.hEvent, timeout );
				if(j==WAIT_TIMEOUT)
				{
					DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_CANCEL_RX,NULL,0,NULL,0,&temp,NULL);
					retbytes[0] = 0;
					CloseHandle(rq.hEvent);
					return READFILE_TIMEOUT;
				}
				if(j==WAIT_ABANDONED)
				{
					retbytes[0] = 0;
					CloseHandle(rq.hEvent);
					return READFILE_ABANDONED;
				}
			} while(j!=WAIT_OBJECT_0);
			GetOverlappedResult(hFSCC[port],&rq,retbytes,TRUE);
		}
		else 
		{
			//		char buf[256];
			//		sprintf(buf,"%8.8x error:%8.8x\n",hFSCC[port],ret);
			//		MessageBox(NULL,buf,"ERROR",MB_OK);
			fscclasterror[port]=t;
			retbytes[0] = 0;
			CloseHandle(rq.hEvent);
			return READFILE_ERROR;
		}
	}
	CloseHandle(rq.hEvent);
	return FSCCTOOLBOX_SUCCESS;
	
	
}


// Function name	: FSCCToolBox_Write_Frame
// Description	    : 
// Return type		: DWORD --return status
// Argument         : DWORD port --port number i.e. FSCC#
// Argument         : char * tbuf --pointer to user's data
// Argument         : DWORD numbytes
// Argument         : DWORD timeout
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Write_Frame(INPUT DWORD port,OUTPUT char * tbuf,INPUT DWORD numbytes,INPUT DWORD timeout)
{
	OVERLAPPED  wq;
	BOOL t;
	DWORD nobyteswritten;
	DWORD j;
	DWORD ret;
	DWORD temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	memset( &wq, 0, sizeof( OVERLAPPED ) );
	wq.hEvent = CreateEvent( NULL,TRUE,FALSE,NULL);
	if (wq.hEvent == NULL)
	{
		fscclasterror[port]=GetLastError();
		return OVERLAPPED_EVENT_CREATE_FAILED;
	}
	
	t = WriteFile(hFSCC[port],tbuf,numbytes,&nobyteswritten,&wq);
	if(t==FALSE)  
	{
		ret=GetLastError();
		if(ret==ERROR_IO_PENDING)
		{
			do
			{
				j = WaitForSingleObject( wq.hEvent, timeout );
				if(j==WAIT_TIMEOUT)
				{
					DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_CANCEL_TX,NULL,0,NULL,0,&temp,NULL);
					CloseHandle(wq.hEvent);
					return WRITEFILE_TIMEOUT;
				}
				if(j==WAIT_ABANDONED)
				{
					CloseHandle(wq.hEvent);
					return WRITEFILE_ABANDONED;
				}
			} while(j!=WAIT_OBJECT_0);
			GetOverlappedResult(hFSCC[port],&wq,&nobyteswritten,TRUE);
		}
		else
		{
			//		char buf[256];
			//		sprintf(buf,"%8.8x error:%8.8x\n",hFSCC[port],ret);
			//		MessageBox(NULL,buf,"ERROR",MB_OK);
			fscclasterror[port]=ret;
			CloseHandle(wq.hEvent);
			return WRITEFILE_ERROR;
		}
	}
	CloseHandle(wq.hEvent);
	return FSCCTOOLBOX_SUCCESS;
	
}


// Function name	: FSCCToolBox_Write_Frame_Limit
// Description	    : 
// Return type		: DWORD --return status
// Argument         : DWORD port --port number i.e. FSCC#
// Argument         : char * tbuf --pointer to user's data
// Argument         : DWORD numbytes
// Argument         : DWORD timeout
// Argument			: DWORD limit
/*
This function is similar to the write frame function but it checks the number of frames in the outbound hardware fifo
and enforces a limit on the number of queued frames.
*/
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Write_Frame_Limit(INPUT DWORD port,OUTPUT char * tbuf,INPUT DWORD numbytes,INPUT DWORD timeout,INPUT DWORD limit)
{
	OVERLAPPED  wq;
	BOOL t;
	DWORD nobyteswritten;
	DWORD j;
	DWORD ret;
	DWORD temp;
	DWORD reg;
	DWORD val;
	DWORD queuedframes;
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	//get number of frames queued in hardware fifo:
		reg = 0x10;//FIFO frame count register
		val = 0;
		t = DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_READ_REGISTER,&reg,sizeof(unsigned long),&val,sizeof(unsigned long),&temp,NULL);
		queuedframes = (val>>16)&0x01ff;

		if(queuedframes<limit)
		{
		//if number of queued frames is less than the given limit then send the frame
		//else error out


	memset( &wq, 0, sizeof( OVERLAPPED ) );
	wq.hEvent = CreateEvent( NULL,TRUE,FALSE,NULL);
	if (wq.hEvent == NULL)
	{
		fscclasterror[port]=GetLastError();
		return OVERLAPPED_EVENT_CREATE_FAILED;
	}
	
	t = WriteFile(hFSCC[port],tbuf,numbytes,&nobyteswritten,&wq);
	if(t==FALSE)  
	{
		ret=GetLastError();
		if(ret==ERROR_IO_PENDING)
		{
			do
			{
				j = WaitForSingleObject( wq.hEvent, timeout );
				if(j==WAIT_TIMEOUT)
				{
					DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_CANCEL_TX,NULL,0,NULL,0,&temp,NULL);
					CloseHandle(wq.hEvent);
					return WRITEFILE_TIMEOUT;
				}
				if(j==WAIT_ABANDONED)
				{
					CloseHandle(wq.hEvent);
					return WRITEFILE_ABANDONED;
				}
			} while(j!=WAIT_OBJECT_0);
			GetOverlappedResult(hFSCC[port],&wq,&nobyteswritten,TRUE);
		}
		else
		{
			//		char buf[256];
			//		sprintf(buf,"%8.8x error:%8.8x\n",hFSCC[port],ret);
			//		MessageBox(NULL,buf,"ERROR",MB_OK);
			fscclasterror[port]=ret;
			CloseHandle(wq.hEvent);
			return WRITEFILE_ERROR;
		}
	}
	CloseHandle(wq.hEvent);
	return FSCCTOOLBOX_SUCCESS;
		}
		else return WRITEFILE_LIMIT;
}

// Function name	: FSCCToolBox_Get_Status
// Description	    : call the blocking status ioctl with timeout
//					  
// Return type		: DWORD --return status
// Argument         : DWORD port --port number i.e. FSCC#

FSCCTOOLBOX_API int __stdcall FSCCToolBox_Get_Status(INPUT DWORD port,OUTPUT DWORD *status,INPUT DWORD mask,INPUT DWORD timeout)
{
	ULONG t;
	ULONG temp;
	OVERLAPPED  st;
	DWORD j;

status[0]=0;
if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

memset( &st, 0, sizeof( OVERLAPPED ) ) ;	//wipe the overlapped struct
st.hEvent = CreateEvent( NULL,TRUE,FALSE,NULL);
if (st.hEvent == NULL)
	{
	fscclasterror[port]=GetLastError();
	return OVERLAPPED_EVENT_CREATE_FAILED;
	}


t=	DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_STATUS,&mask,sizeof(ULONG),status,sizeof(ULONG),&temp,&st);
if(t==FALSE)  
	{
	t=GetLastError();
	if(t==ERROR_IO_PENDING)
		{
		do
			{
			j = WaitForSingleObject( st.hEvent, timeout );
			if(j==WAIT_TIMEOUT)
				{
                DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_CANCEL_STATUS,NULL,0,NULL,0,&temp,NULL);
				CloseHandle(st.hEvent);
				return FSCCSTATUS_TIMEOUT;
				}
			if(j==WAIT_ABANDONED)
				{
				CloseHandle(st.hEvent);
				return FSCCSTATUS_ABANDONED;
				}
			} while(j!=WAIT_OBJECT_0);
		GetOverlappedResult(hFSCC[port],&st,&temp,TRUE);		
		}
	else
		{
//		char buf[256];
//		sprintf(buf,"%8.8x error:%8.8x\n",hFSCC[port],ret);
//		MessageBox(NULL,buf,"ERROR",MB_OK);
		fscclasterror[port]=t;
		CloseHandle(st.hEvent);
		return FSCCSTATUS_ERROR;
		}
	}
CloseHandle(st.hEvent);
return FSCCTOOLBOX_SUCCESS;
}

// Function name	: FSCCToolBox_Get_IStatus
// Description	    : call nonblocking status ioctl
//					  
// Return type		: DWORD --return status
// Argument         : DWORD port --port number i.e. FSCC#

FSCCTOOLBOX_API int __stdcall FSCCToolBox_Get_IStatus(DWORD port,OUTPUT DWORD *status)
{
BOOL t;
DWORD temp;
DWORD mask = 0xFFFFFFFF;

status[0]=0;
if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

t = DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_STATUS,&mask,sizeof(ULONG),status,sizeof(ULONG),&temp,NULL);
if(t) return FSCCTOOLBOX_SUCCESS;
else 
	{
	fscclasterror[port]=GetLastError();
	return DEVICEIOCONTROL_FAILED;
	}
}



// Function name	: FSCCToolBox_Flush_RX
// Description	    : Command issues a reset receiver on the serial controller, zeros all receive
//					  buffers and will issue a CANCEL_RX
// Return type		: DWORD --return status
// Argument         : DWORD port --port number i.e. FSCC#
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Flush_RX(DWORD port)
{
	DWORD temp;
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	DeviceIoControl(hFSCC[port],
		IOCTL_FSCCDRV_FLUSH_RX,                          /* Device IOCONTROL */
		NULL,								/* write data */
		0,						/* write size */
		NULL,								/* read data */
		0,									/* read size */
		&temp,								/* Returned Size */
		NULL); /* overlap */
	return FSCCTOOLBOX_SUCCESS;
}


// Function name	: FSCCToolBox_Flush_TX
// Description	    : Command issues a reset transmit on the serial controller, zeros all transmit
//					  buffers and will issue a CANCEL_TX
// Return type		: DWORD --return status
// Argument         : DWORD port --port number i.e. FSCC#
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Flush_TX(DWORD port)
{
	DWORD temp;
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	DeviceIoControl(hFSCC[port],
		IOCTL_FSCCDRV_FLUSH_TX,                          /* Device IOCONTROL */
		NULL,								/* write data */
		0,						/* write size */
		NULL,								/* read data */
		0,									/* read size */
		&temp,								/* Returned Size */
		NULL); /* overlap */
return FSCCTOOLBOX_SUCCESS;
}

// Function name	: FSCCToolBox_Read_Register
// Description	    : function to retrieve the contents of a FSCC device register
//					  
// Return type		: DWORD --register contents
// Argument         : DWORD port --port number i.e. FSCC#
// Argument			: DWORD regno -- FSCC register (offset)
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Read_Register(DWORD port,DWORD regno, OUTPUT DWORD *value)
{
DWORD val,temp;
BOOL t;
if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	t = DeviceIoControl(hFSCC[port],
                        IOCTL_FSCCDRV_READ_REGISTER,                             /* Device IOCONTROL */
			&regno,								/* write data */
			sizeof(unsigned long ),						/* write size */
			&val,								/* read data */
			sizeof(unsigned long),									/* read size */
			&temp,								/* Returned Size */
			NULL); /* overlap */
        if(temp!=0)  
			{
			value[0]=val;
			return FSCCTOOLBOX_SUCCESS;
			}
        else 
			{	
			//-1/-2/-3 are all valid returns...
			fscclasterror[port]=GetLastError();
			return DEVICEIOCONTROL_FAILED;
			}

}

// Function name	: FSCCToolBox_Write_Register
// Description	    : function to write the contents of a FSCC device register
//					  
// Return type		: DWORD --register contents
// Argument         : DWORD port --port number i.e. FSCC#
// Argument			: DWORD regno -- FSCC register (offset)
// Argument			: DWORD value -- value to put in register
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Write_Register(DWORD port,DWORD regno,DWORD value)
{
BOOL t;
DWORD passval[2];
DWORD temp;
if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
passval[0] = regno;
passval[1] = value;
	t = DeviceIoControl(hFSCC[port],
            IOCTL_FSCCDRV_WRITE_REGISTER,       /* Device IOCONTROL */
			&passval,							/* write data */
			2*sizeof(unsigned long ),			/* write size */
			NULL,								/* read data */
			0,									/* read size */
			&temp,								/* Returned Size */
			NULL); /* overlap */
        if(t)  return FSCCTOOLBOX_SUCCESS;
        else 
			{
			fscclasterror[port]=GetLastError();
			return DEVICEIOCONTROL_FAILED;
			}

}


// Function name	: FSCCToolBox_Configure
// Description	    : 
// Return type		: DWORD --return status
// Argument         : DWORD port --port number i.e. FSCC#
// Argument         : SETUP *settings --pointer to the FSCC settings structure
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Configure(DWORD port,SETUP *settings)
{
	DWORD ret;
	BOOL t;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	t =	DeviceIoControl(hFSCC[port],
		IOCTL_FSCCDRV_SETUP,         /* Device IOCONTROL */
		settings,							/* write data */
		sizeof(SETUP)				,	/* write size */
		NULL,								/* read data */
		0,									/* read size */
		&ret,								/* Returned Size */
		NULL);								/* overlap */
	if(t) return FSCCTOOLBOX_SUCCESS;
	else 
	{
		//char buf[512];
		//	sprintf(buf,"%8.8x\n%8.8x\n%8.8x",settings->ccr0,settings->ccr1,settings->ccr2);
		//	MessageBox(NULL,buf,"CONFIGERR",MB_OK);
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}






// Function name	: FSCCToolBox_Set_SD_485
// Description	    : in 485 mode, the tansmitter will be turned off when not transmitting 
//					  so that it does not drive the line
// Return type		: DWORD --return status
// Argument         : DWORD port --port number i.e. FSCC#
// Argument         : DWORD onoff = 1 SD set to 485 mode
//								  = 0 SD set to 422 mode
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Set_SD_485(DWORD port,DWORD onoff)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_GET_FEATURES,NULL,0,&desreg,sizeof(ULONG),&temp,NULL))
	{
		
		if(onoff==0) desreg &= 0x3d; //reset SD485 bit (SD set to 422 mode)
		else desreg |=0x2;          //set SD485 bit (SD set to 485 mode)
		DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_SET_FEATURES,&desreg,sizeof(ULONG),NULL,0,&temp,NULL);
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}



// Function name	: FSCCToolBox_Set_TT_485
// Description	    : 
// Return type		: DWORD --return status
// Argument         : DWORD port --port number i.e FSCC#
// Argument         : DWORD onoff = 1 TT set to 485 mode
//									0 TT set to 422 mode
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Set_TT_485(DWORD port,DWORD onoff)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_GET_FEATURES,NULL,0,&desreg,sizeof(ULONG),&temp,NULL))
	{

		if(onoff==0) desreg &= 0x3b; //reset TT485 bit (TT set to 422 mode)
		else desreg |=0x4;          //set TT485 bit (TT set to 485 mode)
		DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_SET_FEATURES,&desreg,sizeof(ULONG),NULL,0,&temp,NULL);
		return FSCCTOOLBOX_SUCCESS;
	}
	else
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}


// Function name	: FSCCToolBox_Set_RD_Echo_Cancel
// Description	    : Receive echo cancel is useful in some 2-wire 485 networks where 
//					  it may be useful for the receiver to be disabled during transmits 
//					  so that a station does not receive what it transmits.
// Return type		: DWORD --return status
// Argument         : DWORD port --port number i.e. FSCC#
// Argument         : DWORD onoff = 1 enable RxEcho Cancel
//								  = 0 disable RxEcho Cancel
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Set_RD_Echo_Cancel(DWORD port,DWORD onoff)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_GET_FEATURES,NULL,0,&desreg,sizeof(ULONG),&temp,NULL))
	{
		
		if(onoff==0) desreg &= 0x3e; //reset rxechocancel bit (disables RxEchoCancel)

		else desreg |=0x1;           //set rxechocancel bit (enables RxEcho Cancel)
		DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_SET_FEATURES,&desreg,sizeof(ULONG),NULL,0,&temp,NULL);
		return FSCCTOOLBOX_SUCCESS;
	}
	else
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}



FSCCTOOLBOX_API int __stdcall FSCCToolBox_TxActive(INPUT DWORD port,OUTPUT DWORD *txactive)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_TX_ACTIVE,NULL,0,&desreg,sizeof(ULONG),&temp,NULL))
	{
		txactive[0] = desreg;
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}
FSCCTOOLBOX_API int __stdcall FSCCToolBox_RxReady(INPUT DWORD port, OUTPUT DWORD *rxready)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_RX_READY,NULL,0,&desreg,sizeof(ULONG),&temp,NULL))
	{
		rxready[0]=desreg;
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}

}

FSCCTOOLBOX_API int __stdcall FSCCToolBox_Get_Configure(INPUT DWORD port,OUTPUT SETUP *settings)
{
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_GET_SETUP,NULL,0,settings,sizeof(SETUP),&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}

FSCCTOOLBOX_API int __stdcall FSCCToolBox_EnableTimedTransmit(INPUT DWORD port)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	desreg=1;	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_TIMED_TRANSMIT,&desreg,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}
FSCCTOOLBOX_API int __stdcall FSCCToolBox_DisableTimedTransmit(INPUT DWORD port)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	desreg=0;	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_TIMED_TRANSMIT,&desreg,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}

FSCCTOOLBOX_API int __stdcall FSCCToolBox_GetTimedTransmit(INPUT DWORD port,OUTPUT DWORD *timedtransmit)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_GET_TIMED_TRANSMIT,NULL,0,&desreg,sizeof(ULONG),&temp,NULL))
	{
		timedtransmit[0] = desreg;
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}

FSCCTOOLBOX_API int __stdcall FSCCToolBox_EnableTransmitRepeat(INPUT DWORD port)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	desreg=1;	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_TRANSMIT_REPEAT,&desreg,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}
FSCCTOOLBOX_API int __stdcall FSCCToolBox_DisableTransmitRepeat(INPUT DWORD port)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	desreg=0;	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_TRANSMIT_REPEAT,&desreg,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}
FSCCTOOLBOX_API int __stdcall FSCCToolBox_GetTransmitRepeat(INPUT DWORD port,OUTPUT DWORD *transmitrepeat)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_GET_TRANSMIT_REPEAT,NULL,0,&desreg,sizeof(ULONG),&temp,NULL))
	{
		transmitrepeat[0] = desreg;
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}

FSCCTOOLBOX_API int __stdcall FSCCToolBox_EnableDelayWriteStart(INPUT DWORD port)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	desreg=1;	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_DELAY_WRITE_START,&desreg,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}
FSCCTOOLBOX_API int __stdcall FSCCToolBox_DisableDelayWriteStart(INPUT DWORD port)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	desreg=0;	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_DELAY_WRITE_START,&desreg,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}

FSCCTOOLBOX_API int __stdcall FSCCToolBox_EnableExternalTransmit(INPUT DWORD port)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	desreg=1;	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_EXTERNAL_TRANSMIT,&desreg,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}
FSCCTOOLBOX_API int __stdcall FSCCToolBox_DisableExternalTransmit(INPUT DWORD port)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	desreg=0;	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_EXTERNAL_TRANSMIT,&desreg,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}

FSCCTOOLBOX_API int __stdcall FSCCToolBox_GetExternalTransmit(INPUT DWORD port,OUTPUT DWORD *externaltransmit)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_GET_EXTERNAL_TRANSMIT,NULL,0,&desreg,sizeof(ULONG),&temp,NULL))
	{
		externaltransmit[0]=desreg;
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}


FSCCTOOLBOX_API int __stdcall FSCCToolBox_EnableReadCutoff(INPUT DWORD port)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	desreg=1;	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_ALLOW_READ_CUTOFF,&desreg,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}
FSCCTOOLBOX_API int __stdcall FSCCToolBox_DisableReadCutoff(INPUT DWORD port)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	desreg=0;	
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_ALLOW_READ_CUTOFF,&desreg,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}

FSCCTOOLBOX_API int __stdcall FSCCToolBox_SetRxIrqRate(INPUT DWORD port, INPUT DWORD rate)
{
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_SET_RX_IRQ_RATE ,&rate,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}
FSCCTOOLBOX_API int __stdcall FSCCToolBox_SetTxIrqRate(INPUT DWORD port, INPUT DWORD rate)
{
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_SET_TX_IRQ_RATE ,&rate,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}

FSCCTOOLBOX_API int __stdcall FSCCToolBox_EnableDMA(INPUT DWORD port)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	desreg=1;
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_SET_DMA,&desreg,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}
FSCCTOOLBOX_API int __stdcall FSCCToolBox_DisableDMA(INPUT DWORD port)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;
	desreg=0;
	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_SET_DMA,&desreg,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}

FSCCTOOLBOX_API int __stdcall FSCCToolBox_SetReceiveMultiple(INPUT DWORD port, INPUT DWORD count)
{
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_SET_RECEIVE_MULTIPLE,&count,sizeof(ULONG),NULL,0,&temp,NULL))
	{
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}

FSCCTOOLBOX_API int __stdcall FSCCToolBox_GetReceiveMultiple(INPUT DWORD port,OUTPUT DWORD *receivemultiple)
{
	ULONG desreg;
	ULONG temp;
	
	if(port>MAXPORTS) return MAXPORTS_EXCEEDED;
	if(hFSCC[port]==NULL) return PORT_IS_NOT_OPEN;

	if(DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_GET_RECEIVE_MULTIPLE,NULL,0,&desreg,sizeof(ULONG),&temp,NULL))
	{
		receivemultiple[0]=desreg;
		return FSCCTOOLBOX_SUCCESS;
	}
	else 
	{
		fscclasterror[port]=GetLastError();
		return DEVICEIOCONTROL_FAILED;
	}
}




FSCCTOOLBOX_API int __stdcall FSCCToolBox_GetLastError(INPUT DWORD port,OUTPUT DWORD *value)
{
value[0] = fscclasterror[port];
return FSCCTOOLBOX_SUCCESS;
}


/*

DWORD FAR PASCAL StatusProc( DWORD port )
{
DWORD k;
BOOL t;
DWORD returnsize;
OVERLAPPED  os ;
DWORD status;

				
memset( &os, 0, sizeof( OVERLAPPED ) );


os.hEvent = CreateEvent( NULL,TRUE,FALSE,NULL );
if (os.hEvent == NULL)
   {
   return -3;
   }
do
	{

	t = DeviceIoControl(hFSCC[port],IOCTL_FSCCDRV_STATUS,&statusmask[port],sizeof(ULONG),&status,sizeof(ULONG),&returnsize,&os);
	if(t==FALSE)
		{
		if (GetLastError() == ERROR_IO_PENDING)
			{
			do
				{
				k = WaitForSingleObject( os.hEvent, 5000 );//5 second timeout
				if(k==WAIT_TIMEOUT)
					{
					}
				if(k==WAIT_ABANDONED)
					{
					}
				}while((k!=WAIT_OBJECT_0)&&((connected[port]&4)!=0));//exit if we get signaled or if the main thread quits
				
			if((connected[port]&4)!=0)
				{
				GetOverlappedResult(hFSCC[port],&os,&returnsize,TRUE); 
				t=TRUE;
				}
			}
		else
			{
			//actual error, what to do, what to do
			}
		}
	//if we get here then either it was true immediatly, so we return, or we waited and fell through and have the overlapped complete, either way signal it
	if((connected[port]&4)!=0)                   //if not connected then j is invalid
		{
		statuscallbacks[port](port,status);
		}
	}while((connected[port]&4)!=0);              //keep making requests until we want to terminate
CloseHandle( os.hEvent ) ;              //we are terminating so close the event
return 0;                          //done
}


DWORD FAR PASCAL ReadProc( DWORD port )
{
DWORD j;		
BOOL t;			
char *data;	
DWORD nobytestoread;
DWORD nobytesread;	
OVERLAPPED  os ;	

memset( &os, 0, sizeof( OVERLAPPED ) );

os.hEvent = CreateEvent( NULL,TRUE,FALSE,NULL);
if (os.hEvent == NULL)
   {
   return -3 ;
   }

data = (char *)malloc(readsize[port]);

if(data==NULL) return -4;

nobytestoread = readsize[port];

do
	{
	t = ReadFile(hFSCC[port],data,nobytestoread,&nobytesread,&os);
	if(t==FALSE)
		{
		if (GetLastError() == ERROR_IO_PENDING)
			{
			do
				{
				j = WaitForSingleObject( os.hEvent, 5000 );//5 second timeout
				if(j==WAIT_TIMEOUT)
					{
					//timeout processing
					}
				if(j==WAIT_ABANDONED)
					{
					//??
					}
				}while((j!=WAIT_OBJECT_0)&&((connected[port]&1)!=0));//stay here until we get signaled or the main thread exits
			if((connected[port]&1)!=0)
					{                                                       
					GetOverlappedResult(hFSCC[port],&os,&nobytesread,TRUE); //here to get the actual nobytesread!!!
					t=TRUE;
					}
			}
		else
			{
			//actual read error here...what to do, what to do
			//MessageBox(NULL,"READ ERROR","",MB_OK);
			}
		}
	//if we get here then either it was true immediatly, so we return, or we waited and fell through and have the overlapped complete, either way signal it
	if((connected[port]&1)!=0)
		{
		readcallbacks[port](port,data,nobytesread);
		}
	}while((connected[port]&1)!=0);              //do until we want to terminate
	CloseHandle( os.hEvent ) ;      //done with event
	free(data);
	return 0;                   //outta here
}

DWORD FAR PASCAL WriteProc( DWORD port )
{
OVERLAPPED  wq;
BOOL t;
DWORD nobyteswritten;
DWORD nobytestowrite;
DWORD j;
DWORD ret;
char *data;


memset( &wq, 0, sizeof( OVERLAPPED ) );
wq.hEvent = CreateEvent( NULL,TRUE,FALSE,NULL);
if (wq.hEvent == NULL)
	{
	return -3;
	}
data = (char *)malloc(maxwritesize[port]);
if(data==NULL) return -4;

do
{

if((connected[port]&2)!=0)                   
	{
	writecallbacks[port](port,data,&nobytestowrite);
	}
if(nobytestowrite>0)
{
t = WriteFile(hFSCC[port],data,nobytestowrite,&nobyteswritten,&wq);
if(t==FALSE)  
	{
	ret=GetLastError();
	if(ret==ERROR_IO_PENDING)
		{
		do
			{
			j = WaitForSingleObject( wq.hEvent, 5000 );
			if(j==WAIT_TIMEOUT)
				{
				//timeout processing
				}
				if(j==WAIT_ABANDONED)
				{
				//??
				}
			} while((j!=WAIT_OBJECT_0)&&((connected[port]&2)!=0));
		GetOverlappedResult(hFSCC[port],&wq,&nobyteswritten,TRUE);
		}
	else
		{
		//write error...what to do, what to do
		}
	}
//returned true, so its done or queued
}
}while((connected[port]&2)!=0);
CloseHandle(wq.hEvent);
free(data);
return 0;
}
*/