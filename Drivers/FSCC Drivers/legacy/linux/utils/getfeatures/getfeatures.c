/* $Id$ */
/*
Copyright(c) 2003, Commtech, Inc.
getfeatures.c -- user mode function to get/display the onboard feature setting for a FSCC port 

usage:
 getfeatures port 

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
  int fscc;
  unsigned long desreg;
  unsigned long temp;
  unsigned long passval[2];
  if (argc < 2)
    {
      printf("usage:  %s port \n", argv[0]);
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
  passval[0] = 0;
  passval[1] = 0;
  if (ioctl(fscc, FSCC_GET_FEATURES, &passval[1]) == -1)
    {
      perror(NULL);
    }
  else
    {
      printf("features:%8.8lx\r\n", passval[1]);
      if ((passval[1] & 1) == 0)
	{
	  printf("RX allways on\r\n");
	}
      else
	{
	  printf("RX echo	cancel ENABLED\r\n");
	}
      if ((passval[1] & 2) == 0)
	{
	  printf("SD is RS-422\r\n");
	}
      else
	{
	  printf("SD is RS-485\r\n");
	}
      if ((passval[1] & 4) == 0)
	{
	  printf("TT is RS-422\r\n");
	}
      else
	{
	  printf("TT is RS-485\r\n");
	}
      if ((passval[1] & 8) == 0)
	{
	  printf("DTR/FST is FST\r\n");
	}
      else
	{
	  printf("DTR/FST is DTR\r\n");
	}
      if ((passval[1] & 0x80000000) != 0)
	{
	  printf("ASYNC MODE!\r\n");
	}
      else
	{
	  printf("SYNC MODE!\r\n");
	}
    }
  close(fscc);
  return 0;
}


/* $Id$ */
