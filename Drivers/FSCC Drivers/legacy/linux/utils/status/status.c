/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
status.c -- user mode function to get the fscc status

usage:
 status port 

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
unsigned long status;
unsigned long loop;
if(argc<2)
{
printf("usage:%s port [#]\n",argv[0]);
exit(1);
}
port = atoi(argv[1]);
if(argc>2)
{
loop = atol(argv[2]);
}
else loop=1;
sprintf(nbuf,"/dev/fscc%u",port);
fscc = open(nbuf,O_RDWR);
if(fscc == -1)
{
printf("cannot open %s\n",nbuf);
perror(NULL);
exit(1);
}
printf("opened %s\r\n",nbuf);
do
{
if(ioctl(fscc,FSCC_STATUS,&status)==-1) perror(NULL);
else
{
printf("%s Status: %8.8lx\n",nbuf,status);
if((status& ST_CTSA)!=0)printf("STATUS:CTS transmit abort\n");
if((status& ST_CDC)!=0)	printf("STATUS:CD state change\n");
if((status& ST_DSRC)!=0)	printf("STATUS:DSR state change\n");
if((status& ST_CTSS)!=0)	printf("STATUS:CTS state change\n");
if((status& ST_TIN)!=0)		printf("STATUS:timer expired\n");
if((status& ST_RFS)!=0)		printf("STATUS:receive frame start\n");
if((status& ST_RFL)!=0)		printf("STATUS:receive frame lost\n");
if((status& ST_RDO)!=0)		printf("STATUS:receive data overflow\n");
if((status& ST_RFO)!=0)		printf("STATUS:receive frame overflow\n");
if((status& ST_TDU)!=0)		printf("STATUS:transmit data underrun\n");
if((status& ST_OVF)!=0)		printf("STATUS:receive buffers overflow\n");
if((status& ST_RX_DONE)!=0)	printf("STATUS:receive complete\n");
if((status& ST_ALLS)!=0)	printf("STATUS:all sent\n");
if((status& ST_TX_DONE)!=0)	printf("STATUS:transmit complete\n");
if((status& ST_RFT)!=0)		printf("STATUS:receive fifo trigger\n");
if((status& ST_TFT)!=0)		printf("STATUS:transmit fifo trigger\n");
if((status& ST_RFE)!=0)		printf("STATUS:receive frame end\n");
if((status& ST_RBUF_OVERFLOW)!=0) printf("STATUS:receive buffer overflow\n");
}
loop--;
}while(loop>0);
close(fscc);
return 0;
}



