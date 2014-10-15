/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
getclock.c -- user mode function to get programmable clock setting

usage:
 getclock port 

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


int
main(int argc, char *argv[])
{
  char nbuf[80];
  int port;
  int fscc = -1;
  unsigned long desfreq = 0;

  if (argc < 2)
    {
      printf("usage:\n%s port\n", argv[0]);
      exit(1);
    }
  port = atoi(argv[1]);

  sprintf(nbuf, "/dev/fscc%u", port);
  fscc = open(nbuf, O_RDWR);
  if (fscc == -1)
    {
      printf("cannot open %s\n", nbuf);
      perror(NULL);
      exit(1);
    }
  printf("opened %s\r\n", nbuf);

  if (ioctl(fscc, FSCC_GET_FREQ, &desfreq) == -1)
    {
      perror(NULL);
    }
  printf("%s set to %lu\n", nbuf, desfreq);
  close(fscc);
  return 0;
}
