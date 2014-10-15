/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
getdsr.c -- user mode function to get DSR value

usage:
 getdsr port 

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
  int fscc;
  char nbuf[128];
  unsigned port;
  unsigned long dsrval;
  unsigned long passval[2];
  if (argc < 2)
    {
      printf("usage:%s port\n", argv[0]);
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

  passval[0] = 0x18;
  passval[1] = 0x00;
  if (ioctl(fscc, FSCC_READ_REGISTER, &passval) == -1)
    {
      perror(NULL);
    }
  else
    {
      dsrval = ((passval[1] >> 1) & 0x01);
      printf("%s DSR ==> %d\n", nbuf, dsrval);
    }
  close(fscc);
  return 0;
}
