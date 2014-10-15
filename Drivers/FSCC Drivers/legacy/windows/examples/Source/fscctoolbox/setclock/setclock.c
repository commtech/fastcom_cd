/* $Id$ */
/*
Copyright(c) 2008, Commtech, Inc.
setclock.c -- user mode function to setup the FSCC onboard clock generator using the fscctoolbox.dll

To use the fscctoolbox.DLL, you must do three things:

1. #include fscctoolbox.h in your source code
2. Add the file fscctoolbox.lib to your linker's Output/Library modules.  To do this in
   Visual Studio, Click on Settings in the Project menu.  In the Project settings dialog
   click on the Link tab.  In the Category drop down, choose General.  In the 
   Output/library modules box, add the path to fscctoolbox.lib.  It is easiest to copy the 
   file to your working directory and just add "fscctoolbox.lib"
3. The fscctoolbox.dll file must reside in the same directory as your compiled program .exe

usage:
 setclock port frequency

 The port can be any valid fscc port (0,1,2...).

 frequency is any integer in the range 20000 to 200000000

 
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
	ULONG freq;
	
	
	if(argc!=3)
	{
		printf("%s port frequency\n\n",argv[0]);
		printf(" The port is appended onto \\\\.\\FSCC\n");
		exit(1);
	}
	port=atoi(argv[1]);
	if((ret = FSCCToolBox_Create(port))<0)
	{
		printf("create:%d\n",ret);
		exit(1);
	}
	
	freq = atol(argv[2]);
	if(freq<20000) 
	{
	printf("frequency out of range %lu <20000\n",freq);
	exit(1);
	}
	if(freq>200000000)
	{
	printf("frequency out of range %lu >200000000\n",freq);
	exit(1);
	}

	        
	if((ret=FSCCToolBox_Set_Clock(port,freq))<0)
	{
		printf("SetClock Failed:%d\n",ret);
		printf("%s\n",fsccerrortext[-ret][0]);
	}
	else	printf("FSCC%d Clock set to %lu.\n",port,freq);	
	
	FSCCToolBox_Destroy(port);
	return 0;
}
/* $Id$ */