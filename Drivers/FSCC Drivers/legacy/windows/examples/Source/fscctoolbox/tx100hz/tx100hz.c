#include "windows.h"
#include "conio.h"
#include "stdio.h"
#include "stdlib.h"

#include "..\fscctoolbox.h"

int main(int argc, char* argv[])
{
	int port;
	int i;
	int ret;
	SETUP settings;
	DWORD wrsz;
	DWORD value;
	int start=0;
	char *tbuf;
	int successcount;
	if(argc!=3)
	{
		printf("%s port blocksize\n\n",argv[0]);
		printf(" The port is appended onto \\\\.\\FSCC\n");
		printf(" blocksize is the number of bytes per write/frame\n");
		exit(1);
	}
	port=atoi(argv[1]);
	if((ret = FSCCToolBox_Create(port))<0)
	{
		DWORD errval;
		FSCCToolBox_GetLastError(port,&errval);
		printf("create:%d :%8.8x\n",ret,errval);
		exit(1);
	}
	wrsz=atol(argv[2]);
	if(wrsz==0)
	{
	printf("blocksize cannot be 0!\n");
	FSCCToolBox_Destroy(port);
	return -1;
	}

	tbuf=(char*)malloc(wrsz+10);
	if(tbuf==NULL)
	{
	printf("cannot allocate write buffer!\n");
	FSCCToolBox_Destroy(port);
	return -1;
	}

	if((ret=FSCCToolBox_Set_Clock(port,1000000))<0)
	{
	DWORD errval;
	printf("SetClock ");
	FSCCToolBox_GetLastError(port,&errval);
	printf("error setting clock :%d :%8.8x\n",ret,errval);
	}
	else
	{
	DWORD frequency;
	if((ret=FSCCToolBox_Get_Clock(port,&frequency))<0)
		{
		DWORD errval;
		printf("GetClock ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
		}
	printf("Clock Set to :%ld\n",frequency);
	}

	memset(&settings.fifot,0,sizeof(SETUP));

		settings.fifot = 0x08001000;
		settings.cmdr  = 0x08020000;
		settings.ccr0  = 0x00112004;
		settings.ccr1  = 0x00000018;
		settings.ccr2  = 0x00000000;
		
		settings.bgr   = 0x00000000;
		settings.ssr   = 0x0000007e;
		settings.smr   = 0x00000000;
		settings.tsr   = 0x0000007e;
		settings.tmr   = 0x00000000;
		settings.rar   = 0x00000000;
		settings.ramr  = 0x00000000;
		settings.ppr   = 0x00000000;
		settings.tcr   = 0x0028b0a3;
		settings.imr   = 0x0F000001;
		
		settings.n_rbufs = 10;
		settings.n_tbufs = 10;
		settings.n_rfsize_max = 4096;
		settings.n_tfsize_max = 4096;

	
	if((ret=FSCCToolBox_Configure(port,&settings))<0)
	{
		DWORD errval;
		printf("SetConfigure ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
	}
	else	printf("Finished Settings.\n");	
	
if((ret=FSCCToolBox_Set_SD_485(port,0))<0)//SD 422 mode
{
		DWORD errval;
		printf("SetSD485 ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
}
if((ret=FSCCToolBox_Set_TT_485(port,0))<0)//TT 422 mode
{
		DWORD errval;
		printf("SetTT485 ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
}

if((ret=FSCCToolBox_Set_RD_Echo_Cancel(port,0))<0)//no rx cancel
{
		DWORD errval;
		printf("SetRDEchoCancel ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);

}




	if((ret=FSCCToolBox_Read_Register(port,0x0000004c,&value))<0)
	{
		DWORD errval;
		printf("ReadRegister ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
	}
	else
	{
	if((value>>16)==0x000f) printf("FSCC: ");
	if((value>>16)==0x0014) printf("SuperFSCC: ");
	printf("Firmware Rev:%d.",(value>>8)&0xff);
	printf("%d\n",value&0xff);
	}

	memset(&settings.fifot,0,sizeof(SETUP));
	if((ret=FSCCToolBox_Get_Configure(port,&settings))<0)
	{
		DWORD errval;
		printf("GetConfigure ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
	}
	printf("CCR0:0x%8.8x\n",settings.ccr0);
	printf("CCR1:0x%8.8x\n",settings.ccr1);
	printf("CCR2:0x%8.8x\n",settings.ccr2);
	printf("BGR :0x%8.8x\n",settings.bgr);
	printf("IMR :0x%8.8x\n",settings.imr);
	printf("#t  :%d size:%d\n",settings.n_tbufs,settings.n_tfsize_max);
	printf("#r  :%d size:%d\n",settings.n_rbufs,settings.n_rfsize_max);
	
	if((ret=FSCCToolBox_GetTimedTransmit(port,&value))<0)
	{
		DWORD errval;
		printf("GetConfigure ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
	}
	else
	{
	if(value==0) printf("timed transmit is OFF\n");
	if(value==1) printf("timed transmit is ON\n");
	}

	if((ret=FSCCToolBox_EnableTimedTransmit(port))<0)
	{
		DWORD errval;
		printf("EnableTimedTransmit ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
	}

	if((ret=FSCCToolBox_GetTimedTransmit(port,&value))<0)
	{
		DWORD errval;
		printf("GetTimedTransmit ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
	}
	else
	{
	if(value==0) printf("timed transmit is OFF\n");
	if(value==1) printf("timed transmit is ON\n");
	}


	FSCCToolBox_Flush_RX(port);
	FSCCToolBox_Flush_TX(port);

/*
successcount=0;
	while(!kbhit())
	{
	for(i=0;i<wrsz;i++) tbuf[i]=0x55;
	if((ret = FSCCToolBox_Write_Frame(port,tbuf,wrsz,5))<0)
	{
	if(ret==WRITEFILE_TIMEOUT)
		{
		//buffer is full, so start the timed transmit by firing up the timer
		if(start==0)
			{
			printf("START!\n");
			start=1;
			FSCCToolBox_Write_Register(port,0x14,0x00000001);
			}
		printf("successcount:%d\n",successcount);
		successcount=0;
		}
	else
		{
		DWORD errval;
		printf("WriteFrame ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
		}
	}
	else
	{
		successcount++;
		//write returned success!
		//printf("WR SUCCESS\n");
	}
	}
*/
  
successcount=0;
	while(!kbhit())
	{
	for(i=0;i<wrsz;i++) tbuf[i]=0x55;
	if((ret = FSCCToolBox_Write_Frame_Limit(port,tbuf,wrsz,5,20))<0)
	{
	if(ret==WRITEFILE_LIMIT)
		{
		//buffer is full, so start the timed transmit by firing up the timer
		if(start==0)
			{
			printf("START!\n");
			start=1;
			FSCCToolBox_Write_Register(port,0x14,0x00000001);
			}
		printf("successcount:%d\n",successcount);
		successcount=0;
		Sleep(10);
		}
	else
		{
		DWORD errval;
		printf("WriteFrame ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
		}
	}
	else
	{
		successcount++;
		//write returned success!
		//printf("WR SUCCESS\n");
	}
	}






if((ret=FSCCToolBox_TxActive(port,&value))<0)
	{
		DWORD errval;
		printf("TxActive ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
	}
else 
	{
	printf("txready:%8.8x\n",value);
	if(value==0) printf("Tx Inactive!\n");
	if(value==1) printf("Tx Active!\n");
	}


	if((ret=FSCCToolBox_Read_Register(port,0x00000010,&value))<0)
	{
		DWORD errval;
		printf("ReadRegister ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
	}
	else
	{
	printf("(TX)Hardware Queued:%d\n",(value>>16));
	printf("(RX)Hardware Queued:%d\n",value&0xffff);
	}

if((ret=FSCCToolBox_RxReady(port, &value))<0)
	{
		DWORD errval;
		printf("RxReady ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
	}
else
	{
	printf("#receive buffers/frames ready:%d\n",value);
	}



//FSCCToolBox_Get_Status(port,&status,mask,timeout);
if((ret=FSCCToolBox_Get_IStatus(port,&value))<0)
	{
		DWORD errval;
		printf("GetIStatus ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
	}
	else printf("Status:%8.8x\n",value);



//FSCCToolBox_EnableTransmitRepeat(port);
//FSCCToolBox_DisableTransmitRepeat(port);
//FSCCToolBox_GetTransmitRepeat(port,&value);

//FSCCToolBox_EnableDelayWriteStart(port);
//FSCCToolBox_DisableDelayWriteStart(port);

//FSCCToolBox_EnableExternalTransmit(port);
//FSCCToolBox_DisableExternalTransmit(port);
//FSCCToolBox_GetExternalTransmit(port,&value);

//FSCCToolBox_EnableReadCutoff(port);
//FSCCToolBox_DisableReadCutoff(port);

//FSCCToolBox_SetRxIrqRate(port,rate);
//FSCCToolBox_SetTxIrqRate(port,rate);

//FSCCToolBox_EnableDMA(port);
//FSCCToolBox_DisableDMA(port);

//FSCCToolBox_SetReceiveMultiple(port,count);
//FSCCToolBox_GetReceiveMultiple(port,&value);


	FSCCToolBox_Write_Register(port,0x14,0x00000002);//Stop Timer
	FSCCToolBox_DisableTimedTransmit(port);	

	if((ret=FSCCToolBox_GetTimedTransmit(port,&value))<0)
	{
		DWORD errval;
		printf("GetTimedTransmit ");
		FSCCToolBox_GetLastError(port,&errval);
		printf("error!:%s :%8.8x\n",fsccerrortext[-ret],errval);
	}
	else
	{
	if(value==0) printf("timed transmit is OFF\n");
	if(value==1) printf("timed transmit is ON\n");
	}

	
	FSCCToolBox_Flush_RX(port);
	FSCCToolBox_Flush_TX(port);

	FSCCToolBox_Destroy(port);
return 0;
}