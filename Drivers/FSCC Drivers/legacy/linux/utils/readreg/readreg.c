/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
readreg.c -- user mode function to read a FSCC register

usage:
 readreg port register bar

 The port can be any valid fscc port (0,1) 
 The register is assumed to be a hex value 
 the bar is the PCI bar to access (1 = fscc regs,2 = uart regs,3 = features regs)

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


int main(int argc, char * argv[])
{
char nbuf[80];
int port;
int fscc = -1;
uint32_t passval[2];
unsigned long bar;
if(argc<4)
{
printf("usage:\n%s port register bar\n",argv[0]);
printf("port appended to /dev/fscc\n");
printf("register as a hex value\n");
printf("bar is the register space to access (1,2,3)\n");
printf("1==fscc registers\n");
printf("2==uart registers\n");
printf("3==features register\n");
exit(1);
}
port = atoi(argv[1]);
bar = atoi(argv[3]);
sscanf(argv[2],"%lx",&passval[0]);

sprintf(nbuf,"/dev/fscc%u",port);
fscc = open(nbuf,O_RDWR);
if(fscc == -1)
{
printf("cannot open %s\n",nbuf);
perror(NULL);
exit(1);
}
if(bar==1) 
{
if(ioctl(fscc,FSCC_READ_REGISTER,&passval[0])==-1) perror(NULL);
printf("FSCC register:%s reg:%lx ==> %8.8lx\n",nbuf,passval[0],passval[1]);
}
else if(bar==2)
{
if(ioctl(fscc,FSCC_READ_REGISTER2,&passval[0])==-1) perror(NULL);
printf("UART register:%s reg:%lx ==> %2.2lx\n",nbuf,passval[0],passval[1]&0xff);
}
else if(bar==3)
{
if(ioctl(fscc,FSCC_READ_REGISTER3,&passval[0])==-1) perror(NULL);
printf("FEATURE register:%s reg:%lx ==> %8.8lx\n",nbuf,passval[0],passval[1]);
}
else
{
printf("invalid bar #\n");
exit(1);
}
close(fscc);
return 0;
}

