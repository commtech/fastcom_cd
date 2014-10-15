/* $Id$ */
//#define ESCC
#define ESCCP


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef ESCCP
 #include "../esccpdrv.h"
#endif
#ifdef ESCC
 #include "../esccdrv.h"
#endif
#include <sys/ioctl.h>
#include <errno.h>
#include <math.h>
#include <sys/poll.h>

int main(int argc, char * argv[])
{
unsigned long passval;
unsigned long status;
char nbuf[80];
int port;
FILE *fin;
int escc = -1;
char buf[4096];
unsigned rcount;
unsigned wcount;
unsigned long wtotal = 0;
unsigned loop = 0;
unsigned delay = 0;


if(argc<2)
{
printf("usage:\n%s port\n",argv[0]);
exit(1);
}
port = atoi(argv[1]);
#ifdef ESCCP
sprintf(nbuf,"/dev/esccp%u",port);
#endif
#ifdef ESCC
sprintf(nbuf,"/dev/escc%u",port);
#endif

printf("opening %s\n",nbuf);


escc = open(nbuf,O_RDWR);
if(escc == -1)
{
#ifdef ESCCP
printf("cannot open esccp%u\n",port);
#endif
#ifdef ESCC
printf("cannot open escc%u\n",port);
#endif

perror(NULL);
exit(1);
}
#ifdef ESCCP
printf("opened esccp%u\r\n",port);
#endif
#ifdef ESCC
printf("opened escc%u\r\n",port);
#endif

//lets start clean
while(1)
{
passval =0;
if(ioctl(escc,ESCC_TX_BYTES_LEFT,&passval)==-1) perror(NULL);
printf("val:%d\n",passval);
}
close(escc);
return 0;
}


