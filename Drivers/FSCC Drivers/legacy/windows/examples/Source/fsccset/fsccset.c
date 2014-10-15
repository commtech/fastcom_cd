/* $Id$ */
/*
Copyright(c) 2005, Commtech, Inc.
fsccset.c -- user mode function to setup the FSCC registers & fifo's

usage:
 fsccset port setfile

 The port can be any valid fscc port (0,1).

 The setfile is a text file that contains the settings to use
 (see the example settings file 'hdlcset')

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
	FILE *fp;
	char dev[9];
	char inputline[100],*ptr;
	unsigned long value;
	DWORD ret;
	struct setup settings;
	
	if(argc!=3)
	{
		printf("%s portnum filename\n\n",argv[0]);
		printf(" The portnum is appended onto \\\\.\\FSCC\n");
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
	
	fp = fopen(argv[2],"r");
	if(fp==NULL)
	{
		printf("Cannot open file %s\n",argv[2]);
		exit(1);
	}
	
	memset(&settings,0,sizeof(struct setup));
	memset(inputline,0,sizeof(inputline));
	
	while(fgets(inputline,100,fp)!=NULL)
	{
		/* zero the comments so not to get confused with key words */
		/* # pound is the comment prefix */
		for(i=0;inputline[i]!='\n';i++)
		{
			if(inputline[i]=='#')
			{
				memset(&inputline[i],0, 100-i );	
				break;
			}
		}
		i = 0;
		while(inputline[i]!=0) 
		{
			inputline[i] = tolower(inputline[i]);
			i++;
		}
		
		if( (ptr = strchr(inputline,'='))!=NULL)
		{
			for(i=1;ptr[i]==' ' && ptr[i]!='\n';i++);
			value = (unsigned long)strtoul(ptr+i,NULL,16);
			//printf("value = 0x%lx\n",(long)value);
		}
		else
			value=0;
		
		
		if(strncmp(inputline,"fifot",5)==0) settings.fifot = value;
		if(strncmp(inputline,"cmdr",4)==0) settings.cmdr = value;
		if(strncmp(inputline,"ccr0",4)==0) settings.ccr0 = value;
		if(strncmp(inputline,"ccr1",4)==0) settings.ccr1 = value;
		if(strncmp(inputline,"ccr2",4)==0) settings.ccr2 = value;
		
		if(strncmp(inputline,"bgr",3)==0) settings.bgr = value;
		if(strncmp(inputline,"ssr",3)==0) settings.ssr = value;
		if(strncmp(inputline,"smr",3)==0) settings.smr = value;
		if(strncmp(inputline,"tsr",3)==0) settings.tsr = value;
		if(strncmp(inputline,"tmr",3)==0) settings.tmr = value;
		if(strncmp(inputline,"rar",3)==0) settings.rar = value;
		if(strncmp(inputline,"ramr",4)==0) settings.ramr = value;
		if(strncmp(inputline,"ppr",3)==0) settings.ppr = value;
		if(strncmp(inputline,"tcr",3)==0) settings.tcr = value;
		if(strncmp(inputline,"imr",3)==0) settings.imr = value;
		
		if(strncmp(inputline,"n_rbufs",7)==0) settings.n_rbufs = value;
		if(strncmp(inputline,"n_tbufs",7)==0) settings.n_tbufs = value;
		if(strncmp(inputline,"n_rfsize_max",12)==0) settings.n_rfsize_max = value;
		if(strncmp(inputline,"n_tfsize_max",12)==0) settings.n_tfsize_max = value;
		
		memset(inputline,0,100);
	}
	
	if(DeviceIoControl(hDevice,IOCTL_FSCCDRV_SETUP,&settings,sizeof(settings),NULL,0,&ret,NULL))
	{
		printf("%s settings set on %s\n",argv[2],dev);
	}
	else printf("error in ioctl:%8.8x\n",GetLastError());
	
	CloseHandle(hDevice);
	
	return 0;
}
/* $Id$ */