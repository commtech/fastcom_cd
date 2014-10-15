/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
setfscc.c -- user mode function to set the FSCC registers

usage:
 setfscc port settingsfile

 The port can be any valid fscc port (0,1) 
 

*/



#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
 #include "../fsccdrv.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <math.h>
#include <sys/poll.h>


int main(int argc, char * argv[])
{
FILE *fin;
setup settings;
char nbuf[80];
int port;
int fscc = -1;
int i;
long long  value;
char *numptr;
char inputline[512];


memset(inputline,0,512);

if(argc<3)
{
printf("usage:\n%s port settingsfile\n",argv[0]);
exit(1);
}
fin = fopen(argv[2],"rb");
if(fin==NULL)
{
printf("cannot open settings file :%s\n",argv[2]);
exit(1);
}

port = atoi(argv[1]);
sprintf(nbuf,"/dev/fscc%u",port);
printf("opening %s\n",nbuf);
fscc = open(nbuf,O_RDWR);
if(fscc == -1)
{
printf("cannot open %s\n",nbuf);
error(NULL);
exit(1);
}
memset(&settings,0,sizeof(setup));

//default settings just in case they give us a bogus
//settings file, this will keep the 82532 from enabling 
//interrupts (which is what would happen if we just wrote all 0's to it)
settings.imr = 0xffffffff;

while(fgets(inputline,256,fin)!=NULL)
{
//pull settings from file
i = 0;
while(inputline[i]!=0) 
			{
			inputline[i] = tolower(inputline[i]);
			i++;
			}
if( (numptr = strchr(inputline,'='))!=NULL)
	{
	value = strtoll(numptr+1,NULL,16);
	}
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
}

if(ioctl(fscc,FSCC_INIT,&settings)==-1)
{
 perror(NULL);
 printf("failed settings ioctl call\n");
close(fscc);
exit(1);
}
else 
{
printf("%s settings set\n",nbuf);
}
fclose(fin);
close(fscc);
return 0;
}





