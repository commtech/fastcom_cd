/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
setdtr.c -- user mode function to set the state of DTR

usage:
 setdtr port 0|1

 The port can be any valid fscc port (0,1) 
 

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
unsigned long dtrval;
unsigned long passval[2];

if(argc<3)
{
printf("usage:%s port 0|1\n",argv[0]);
exit(1);
}
port = atoi(argv[1]);
dtrval = atoi(argv[2]);
sprintf(nbuf,"/dev/fscc%u",port);
fscc = open(nbuf,O_RDWR);
if(fscc == -1)
{
printf("cannot open %s\n",nbuf);
perror(NULL);
exit(1);
}
printf("opened %s\r\n",nbuf);

passval[0] = 0x20;
passval[1] = 0x00000000;
if(ioctl(fscc,FSCC_READ_REGISTER,&passval[0])==-1)
 {
 perror(NULL);
 printf("error reading ccr1! aborting\n");
 exit(1);
 }
passval[1]=passval[1]&0xfffffffd;
if(dtrval==1) passval[1]|=0x02;
if(ioctl(fscc,FSCC_WRITE_REGISTER,&passval[0])==-1) perror(NULL);
else printf("%s DTR ==> %d\n",nbuf,dtrval);
close(fscc);
return 0;
}

