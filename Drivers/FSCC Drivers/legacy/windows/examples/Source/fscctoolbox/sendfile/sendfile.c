/* $Id$ */
/*
Copyright(c) 2008, Commtech, Inc.
sendfile.c -- user mode function to send file data out the FSCC port using the fscctoolbox.dll

To use the fscctoolbox.DLL, you must do three things:

1. #include fscctoolbox.h in your source code
2. Add the file fscctoolbox.lib to your linker's Output/Library modules.  To do this in
   Visual Studio, Click on Settings in the Project menu.  In the Project settings dialog
   click on the Link tab.  In the Category drop down, choose General.  In the 
   Output/library modules box, add the path to fscctoolbox.lib.  It is easiest to copy the 
   file to your working directory and just add "fscctoolbox.lib"
3. The fscctoolbox.dll file must reside in the same directory as your compiled program .exe

usage:
 sendfile port filetosend

 The port can be any valid fscc port (0,1,2...).

 filetosend is the file to open and say.

 
*/


#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include "..\fscctoolbox.h" /* user code header */


int main(int argc, char *argv[])
{
	int port;
	int ret;
	FILE *fin;
	ULONG frame,bytes;
	DWORD status;
	ULONG blocksize;
	ULONG sz;
	unsigned char *buf;

	if(argc!=4)
	{
		printf("%s port blocksize file\n\n",argv[0]);
		printf(" The port is appended onto \\\\.\\FSCC\n");
		exit(1);
	}
	blocksize=atol(argv[2]);
	if(blocksize<1)
	{
	printf("inavlid block size!\n");
	exit(1);
	}
	
	buf=(unsigned char*)malloc(blocksize+2);
	if(buf==NULL)
	{
	printf("cannot allocate data buffer!\n");
	exit(1);
	}

	fin = fopen(argv[3],"rb");
	if(fin==NULL)
	{
	printf("cannot open %s\n",argv[2]);
	exit(1);
	}

	port=atoi(argv[1]);
	if((ret = FSCCToolBox_Create(port))<0)
	{
		printf("create:%d\n",ret);
		exit(1);
	}
	frame=0;
	bytes=0;

	FSCCToolBox_Get_IStatus(port,&status);//clear status

	while(!feof(fin))
	{
	sz= fread(buf,1,blocksize,fin);
	if(sz>0) 
	{
	FSCCToolBox_Write_Frame(port,&buf[0],sz,1000);//1 second timeout...adjust as necessary for bitrate!
	frame++;
	bytes+=sz;
	}
	}

	//wait for allsent here!
	
	while (FSCCToolBox_Get_Status(port,&status,ST_ALLS,1000)==FSCCSTATUS_TIMEOUT)//wait a second for allsent indication (again adjust for bitrate!)
	{
		printf("waiting for allsent\n");
	}
	printf("sent %lu bytes in %lu frames\n",bytes,frame);
	FSCCToolBox_Destroy(port);
	return 0;
}
/* $Id$ */