/* $Id$ */
/*
Copyright(c) 2008, Commtech, Inc.
setfscc.c -- user mode function to setup the FSCC registers using the fscctoolbox.dll

To use the fscctoolbox.DLL, you must do three things:

1. #include fscctoolbox.h in your source code
2. Add the file fscctoolbox.lib to your linker's Output/Library modules.  To do this in
   Visual Studio, Click on Settings in the Project menu.  In the Project settings dialog
   click on the Link tab.  In the Category drop down, choose General.  In the 
   Output/library modules box, add the path to fscctoolbox.lib.  It is easiest to copy the 
   file to your working directory and just add "fscctoolbox.lib"
3. The fscctoolbox.dll file must reside in the same directory as your compiled program .exe

usage:
 setfscc port setfile

 The port can be any valid fscc port (0,1,2...).

 The setfile is a text file that contains the settings to use.  See the default 
 settings files 'hdlcset', 'asyncset' & 'bisyncset' for your chosen mode).

 This could be modified so that the settings values were set directly by your program, 
 instead of opening an external file.

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
	int i;
	FILE *fp;
	char inputline[100],*ptr;
	unsigned long value;
	DWORD ret;
	SETUP settings;
	
	
	if(argc!=3)
	{
		printf("%s port settingsfile\n\n",argv[0]);
		printf(" The port is appended onto \\\\.\\FSCC\n");
		exit(1);
	}
	port=atoi(argv[1]);
	if((ret = FSCCToolBox_Create(port))<0)
	{
		printf("create:%d\n",ret);
		exit(1);
	}
	
	fp = fopen(argv[2],"r");
	if(fp==NULL)
	{
		printf("Cannot open file %s\n",argv[2]);
		exit(1);
	}
	
	memset(&settings,0,sizeof(SETUP));
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
		else	value=0;
		
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
	
	if((ret=FSCCToolBox_Configure(port,&settings))<0)
	{
		printf("configure:%d\n",ret);
	}
	else	printf("Finished Settings.\n");	
	
	FSCCToolBox_Destroy(port);
	return 0;
}
/* $Id$ */