
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FSCCTOOLBOX_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FSCCTOOLBOX_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef FSCCTOOLBOX_EXPORTS
#define FSCCTOOLBOX_API __declspec(dllexport)
#else
#define FSCCTOOLBOX_API __declspec(dllimport)
#endif

#include "fscc.h"


#define MAXPORTS 10

#ifdef __cplusplus
extern "C" 
{
#endif
#define INPUT
#define OUTPUT

FSCCTOOLBOX_API int __stdcall FSCCToolBox_Create(INPUT DWORD port);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Destroy(INPUT DWORD port);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_Block_Multiple_Io(INPUT DWORD port,INPUT DWORD onoff);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_Set_Clock(INPUT DWORD port,INPUT DWORD frequency);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Get_Clock(INPUT DWORD port,OUTPUT DWORD *frequency);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_Read_Frame(INPUT DWORD port,OUTPUT char * rbuf,INPUT DWORD szrbuf,OUTPUT DWORD *retbytes,INPUT DWORD timeout);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Write_Frame(INPUT DWORD port,INPUT char * tbuf,INPUT DWORD numbytes,INPUT DWORD timeout);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Write_Frame_Limit(INPUT DWORD port,OUTPUT char * tbuf,INPUT DWORD numbytes,INPUT DWORD timeout,INPUT DWORD limit);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Get_Status(INPUT DWORD port,OUTPUT DWORD *status,INPUT DWORD mask,INPUT DWORD timeout);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Get_IStatus(INPUT DWORD port,OUTPUT DWORD *status);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_Flush_RX(INPUT DWORD port);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Flush_TX(INPUT DWORD port);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_Read_Register(INPUT DWORD port,INPUT DWORD regno,OUTPUT DWORD *value);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Write_Register(INPUT DWORD port,INPUT DWORD regno,INPUT DWORD value);


FSCCTOOLBOX_API int __stdcall FSCCToolBox_Configure(INPUT DWORD port,INPUT SETUP *settings);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_Set_SD_485(INPUT DWORD port,INPUT DWORD onoff);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Set_TT_485(INPUT DWORD port,INPUT DWORD onoff);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_Set_RD_Echo_Cancel(INPUT DWORD port,INPUT DWORD onoff);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_TxActive(INPUT DWORD port,OUTPUT DWORD *txactive);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_RxReady(INPUT DWORD port, OUTPUT DWORD *rxready);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_Get_Configure(INPUT DWORD port,OUTPUT SETUP *settings);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_EnableTimedTransmit(INPUT DWORD port);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_DisableTimedTransmit(INPUT DWORD port);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_GetTimedTransmit(INPUT DWORD port,OUTPUT DWORD *timedtransmit);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_EnableTransmitRepeat(INPUT DWORD port);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_DisableTransmitRepeat(INPUT DWORD port);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_GetTransmitRepeat(INPUT DWORD port,OUTPUT DWORD *transmitrepeat);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_EnableDelayWriteStart(INPUT DWORD port);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_DisableDelayWriteStart(INPUT DWORD port);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_EnableExternalTransmit(INPUT DWORD port);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_DisableExternalTransmit(INPUT DWORD port);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_GetExternalTransmit(INPUT DWORD port,OUTPUT DWORD *externaltransmit);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_EnableReadCutoff(INPUT DWORD port);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_DisableReadCutoff(INPUT DWORD port);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_SetRxIrqRate(INPUT DWORD port, INPUT DWORD rate);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_SetTxIrqRate(INPUT DWORD port, INPUT DWORD rate);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_EnableDMA(INPUT DWORD port);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_DisableDMA(INPUT DWORD port);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_SetReceiveMultiple(INPUT DWORD port, INPUT DWORD count);
FSCCTOOLBOX_API int __stdcall FSCCToolBox_GetReceiveMultiple(INPUT DWORD port,OUTPUT DWORD *receivemultiple);

FSCCTOOLBOX_API int __stdcall FSCCToolBox_GetLastError(INPUT DWORD port,OUTPUT DWORD *value);

#define FSCCTOOLBOX_SUCCESS 0
#define MAXPORTS_EXCEEDED -1
#define PORT_IS_OPEN -2
#define CREATEFILE_FAILED -3
#define PORT_IS_NOT_OPEN -4
#define DEVICEIOCONTROL_FAILED -5
#define OVERLAPPED_EVENT_CREATE_FAILED -6
#define READFILE_TIMEOUT -7
#define READFILE_ABANDONED -8
#define READFILE_ERROR -9
#define WRITEFILE_TIMEOUT -10
#define WRITEFILE_ABANDONED -11
#define WRITEFILE_ERROR -12
#define WRITEFILE_LIMIT -13
#define FSCCSTATUS_TIMEOUT -14
#define FSCCSTATUS_ABANDONED -15
#define FSCCSTATUS_ERROR -16

extern DWORD fscclasterror[MAXPORTS];
char fsccerrortext[][256] = {	"Operation Succeeded",
							"MAXPORTS Exceeded, increase and recompile toolbox",
							"Port is already open",
							"CreateFile function failed",
							"Port is not open, Call FSCCToolBox_Create() first",
							"DeviceIoControl function failed",
							"Overlapped event creation failed",
							"ReadFile Timeout",
							"ReadFile Abandoned",
							"ReadFile Error",
							"WriteFile Timeout",
							"WriteFile Abandoned",
							"WriteFile Error",
							"WriteFile Limit Reached",
							"Status Timeout",
							"Status Abandoned",
							"Status Error",
};


#ifdef __cplusplus
}
#endif
