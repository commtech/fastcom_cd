/* $Id$ */
/*
Copyright(c) 2006, Commtech, Inc.
fsccgetset.c -- user mode function to retrieve the current setup of the FSCC registers & fifo's

usage:
 fsccgetset port 

 The port can be any valid fscc port (0,1).

 
*/


#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include "..\fscc.h"


int main(int argc, char *argv[])
{
    HANDLE hDevice;/* Handler for the FSCC driver */
	int i;
	char dev[9];
	DWORD ret;
	struct setup settings;

	if(argc<2)
	{
		printf("%s port\n\n",argv[0]);
		printf(" The port is appended onto \\\\.\\FSCC\n");
		exit(1);
	}
	i=atoi(argv[1]);
	sprintf(dev,"\\\\.\\FSCC%d",i);
	hDevice = CreateFile (dev,
			  GENERIC_READ | GENERIC_WRITE,
			  0,
			  NULL,
			  OPEN_EXISTING,
			  FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
			  NULL
			  );

    if (hDevice == INVALID_HANDLE_VALUE)
    {
		printf("Can't get a handle to fsccpdrv @ %s\n",dev);
		exit(1);
	}

	memset(&settings,0,sizeof(struct setup));
	

	if(DeviceIoControl(hDevice,IOCTL_FSCCDRV_GET_SETUP,NULL,0,&settings,sizeof(settings),&ret,NULL))
		{
		printf("fifot = %8.8x\n",settings.fifot);
		printf("cmdr  = %8.8x\n",settings.cmdr);
		printf("ccr0  = %8.8x\n",settings.ccr0);
		printf("ccr1  = %8.8x\n",settings.ccr1);
		printf("ccr2  = %8.8x\n",settings.ccr2);
		printf("bgr   = %8.8x\n",settings.bgr);
		printf("ssr   = %8.8x\n",settings.ssr);
		printf("smr   = %8.8x\n",settings.smr);
		printf("tsr   = %8.8x\n",settings.tsr);
		printf("tmr   = %8.8x\n",settings.tmr);
		printf("rar   = %8.8x\n",settings.rar);
		printf("ramr  = %8.8x\n",settings.ramr);
		printf("ppr   = %8.8x\n",settings.ppr);
		printf("tcr   = %8.8x\n",settings.tcr);
		printf("imr   = %8.8x\n",settings.imr);
		printf("n_rbufs = %8.8x\n",settings.n_rbufs);
		printf("n_tbufs = %8.8x\n",settings.n_tbufs);
		printf("n_rfsize_max = %8.8x\n",settings.n_rfsize_max);
		printf("n_tfsize_max = %8.8x\n",settings.n_tfsize_max);
		}
	else printf("error in ioctl:%8.8x\n",GetLastError());

	CloseHandle(hDevice);

	return 0;
}
/* $Id$ */