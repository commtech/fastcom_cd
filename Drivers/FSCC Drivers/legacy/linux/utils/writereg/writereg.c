/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
writereg.c -- user mode function to set a FSCC register

usage:
 writereg port register value bar

 The port can be any valid fscc port (0,1) 
 register is any FSCC register (in hex)
 value is any valid value (0-FFFFFFFF)
 bar is the pci address space to write to (1=FSCC,2=UART,3=Features)

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "../fsccdrv.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <math.h>
#include <sys/poll.h>

int main(int argc,char*argv[])
{
int fscc;
char nbuf[128];
unsigned port;
unsigned long passval[2];
unsigned long bar;

if(argc<5)
{
printf("usage:%s port register value bar\n",argv[0]);
exit(1);
}
port = atoi(argv[1]);
sscanf(argv[2],"%x",&passval[0]);
sscanf(argv[3],"%x",&passval[1]);
bar = atol(argv[4]);
sprintf(nbuf,"/dev/fscc%u",port);
fscc = open(nbuf,O_RDWR);
if(fscc == -1)
{
printf("cannot open %s\n",nbuf);
perror(NULL);
exit(1);
}
printf("opened %s\r\n",nbuf);

if(bar==1)
{
if(ioctl(fscc,FSCC_WRITE_REGISTER,&passval[0])==-1) perror(NULL);
else printf("FSCC register:%s port:%lx ==> %8.8lx\n",nbuf,passval[0],passval[1]);
}
else if(bar==2)
{
if(ioctl(fscc,FSCC_WRITE_REGISTER2,&passval[0])==-1) perror(NULL);
else printf("UART register:%s port:%lx ==> %2.2lx\n",nbuf,passval[0],passval[1]);
}
else if(bar==3)
{
if(ioctl(fscc,FSCC_WRITE_REGISTER3,&passval[0])==-1) perror(NULL);
else printf("Features register:%s port:%lx ==> %8.8lx\n",nbuf,passval[0],passval[1]);
}
else 
{
printf("invalid bar #\n");
exit(1);
}
close(fscc);
return 0;
}

