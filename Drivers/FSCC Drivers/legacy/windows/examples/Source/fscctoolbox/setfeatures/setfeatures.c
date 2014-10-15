/* $Id$ */
/*
Copyright(c) 2008, Commtech, Inc.
setfeatures.c -- user mode function to setup the FSCC onboard features using the fscctoolbox.dll

To use the fscctoolbox.DLL, you must do three things:

1. #include fscctoolbox.h in your source code
2. Add the file fscctoolbox.lib to your linker's Output/Library modules.  To do this in
   Visual Studio, Click on Settings in the Project menu.  In the Project settings dialog
   click on the Link tab.  In the Category drop down, choose General.  In the 
   Output/library modules box, add the path to fscctoolbox.lib.  It is easiest to copy the 
   file to your working directory and just add "fscctoolbox.lib"
3. The fscctoolbox.dll file must reside in the same directory as your compiled program .exe

usage:
 setfeatures port [0|1] [0|1] [0|1] 

 The port can be any valid fscc port (0,1,2...).

 parameter1 rx echo cancel control, 1==echo cancel ON, 0== OFF
 parameter2 SD 485 control, 1== SD is RS-485, 0== SD is RS-422
 parameter3 TT 485 control, 1== TT is RS-485, 0== TT is RS-422

 
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
	DWORD onoff;
	ULONG sd485;
	ULONG tt485;
	ULONG rxecho;
	
	if(argc<5) 
		{
		printf("usage:\r\n");
		printf("%s port [0|1] [0|1] [0|1] \r\n",argv[0]);
		printf("\r\n");
		printf("The port can be any valid fscc port (0,1)\r\n");
		printf("\r\n");
		printf("parameter1 rx echo cancel control, 1==echo cancel ON, 0== OFF\r\n");
		printf("parameter2 SD 485 control, 1== SD is RS-485, 0== SD is RS-422\r\n");
		printf("parameter3 TT 485 control, 1== TT is RS-485, 0== TT is RS-422\r\n");
		printf("\r\n");
		exit(1);
		}

	
	rxecho = atol(argv[2]);
	sd485 = atol(argv[3]);
	tt485 = atol(argv[4]);

	if((rxecho!=0)||(rxecho!=1))
	{
	printf("invalid rxecho param %u != 0|1\n",rxecho);
	exit(1);
	}
	if((sd485!=0)||(sd485!=1))
	{
	printf("invalid sd485 param %u != 0|1\n",sd485);
	exit(1);
	}
	if((tt485!=0)||(tt485!=1))
	{
	printf("invalid tt485 param %u != 0|1\n",tt485);
	exit(1);
	}

	port=atoi(argv[1]);
	if((ret = FSCCToolBox_Create(port))<0)
	{
		printf("create:%d\n",ret);
		exit(1);
	}

	onoff=rxecho;
	ret = FSCCToolBox_Set_RD_Echo_Cancel(port,onoff);
	if(ret<0)
	{
		printf("Set RD Echo Failed:%d\n",ret);
		printf("%s\n",fsccerrortext[-ret][0]);
	}

	onoff=sd485;
	ret = FSCCToolBox_Set_SD_485(port,onoff);
	if(ret<0)
	{
		printf("Set SD 485 Failed:%d\n",ret);
		printf("%s\n",fsccerrortext[-ret][0]);
	}

	onoff=tt485;
	ret = FSCCToolBox_Set_TT_485(port,onoff);
	if(ret<0)
	{
		printf("Set TT 485 Failed:%d\n",ret);
		printf("%s\n",fsccerrortext[-ret][0]);
	}



	FSCCToolBox_Destroy(port);
	return 0;
}
/* $Id$ */