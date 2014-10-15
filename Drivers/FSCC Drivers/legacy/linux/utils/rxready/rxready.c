/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
rxready.c -- user mode function to return the number of receive frames available to read

usage:
 rxready port 

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
unsigned long rxready;
if(argc<2)
{
printf("usage:%s port\n",argv[0]);
exit(1);
}
port = atoi(argv[1]);
sprintf(nbuf,"/dev/fscc%u",port);
fscc = open(nbuf,O_RDWR);
if(fscc == -1)
{
printf("cannot open %s\n",nbuf);
perror(NULL);
exit(1);
}
printf("opened %s\r\n",nbuf);


if(ioctl(fscc,FSCC_RX_READY,&rxready)==-1) perror(NULL);
else printf("%s RX READY ==> %d\n",nbuf,rxready);
close(fscc);
return 0;
}

