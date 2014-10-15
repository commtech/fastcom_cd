/* $Id$ */
/*
pty.c -- a utility to connect the SuperFastcom character driver to a pseudoterminal (for use with PPP).

Code originally from Michael Eriksson <Michael.Eriksson@era-t.ericsson.se>
taken from the netbsd-help list archive
modified to do what I was after, ie connecting a SFC/ESCC port to a pty.

must be compiled with -lutil as the openpty() function is located there
ie

cc -o pty pty.c -lutil

*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <pty.h>
#include <errno.h>
#include "../fsccdrv.h"

int uselog = 0;

void
reaper()
{
  int pid;
  union wait status;

  if ((pid = wait3((int *) &status, WNOHANG, 0)) > 0)
    exit(0);
}

/*
This function takes data from the pty and delivers it to the FSCC port, as is
*/
void
shuffle2sfc(int from, int to, char *name)
{
  int n, n2;
  char buf[4096];	
  uint32_t status;
  FILE *log;
  if (uselog == 1)
    {
      log = fopen(name, "wb");
    }
topw:
//get data from the pty
  while ((n = read(from, buf, (sizeof buf) - 2)) >= 0)
    {
      if (uselog == 1)
	{
	  fprintf(log, "[%s: ", name);
	  fprintf(log, "%d]\n", n);
	  fflush(log);
	}
if(n>0)
{
//if not an error then write it to the FSCC
      if ((n2 = write(to, buf, n)) != n)
	if (n2 < 0)
	  if (uselog == 1)
	    fprintf(log, "%s writeerror:%d\n", name, n2);	//err(1, "%s write", name);
	  else if (uselog == 1)
	    fprintf(log, "%s shortwrite:(%d<%d)\n", name, n2, n);	//       err(1, "%s short write (%d < %d)", name, n2, n);
}   }
  if (n < 0)
	{
    if (uselog == 1)
      fprintf(log, "%s readerror:%d\n", name, n);	//    err(1, "%s read", name);
	//here the write error is going to cause the pty program to exit, and the pppd link to shutdown.
	//you could try flushing tx and going back into the write loop, or other error recovery mechanism.
	//otherwise we terminate here.
	ioctl(to,FSCC_IMMEDIATE_STATUS,&status);
	if(uselog==1) fprintf(log,"istatus:%lx\n",status);
	ioctl(to,FSCC_FLUSH_TX);
	goto topw;
	}
  if (uselog == 1)
    fclose(log);
}

/*
This function takes data from the FSCC, strips the last word (status bytes)
and delivers the rest to the pty as is.  Note that this assumes that the
FSCC port is configured in HDLC mode (or a mode that includes the status word).
Other modes are possible with suitable modificaiton to this routine
*/
void
shuffle2pty(int from, int to, char *name)
{
  int n, n2;
  char buf[4096];
  uint32_t status;  
  FILE *log;
  if (uselog == 1)
  {
    log = fopen(name, "wb");

  }
topr:
//read data from the FSCC
  while ((n = read(from, buf, sizeof buf)) >= 0) //if you have the read timeout code on, this must be >=0 as 0 is a valid (timeout) return
    {

      if (uselog == 1)
	{
	  fprintf(log, "[%s: ", name);
	  fprintf(log, "%d]\n", n);
	  fflush(log);
	}
if(n>2)
{//only write to pty if more that the status is returned by the read
//while not an error write it to the pty (don't include the RSTA byte)
      if ((n2 = write(to, buf, n - 2)) != n - 2)
	if (n2 < 0)
	  if (uselog == 1)
	    fprintf(log, "%s writeerror:%d\n", name, n2);	//err(1, "%s write", name);
	  else if (uselog == 1)
	    fprintf(log, "%s shortwrite:(%d<%d)\n", name, n2, n - 2);	//     err(1, "%s short write (%d < %d)", name, n2, n-1);
}    }
  if (n < 0)
	{
    if (uselog == 1)
      fprintf(log, "%s readerror:%d,%d\n", name, n,errno);	//    err(1, "%s read", name);
	//right about here the pty program is going to exit, (taking pppd down and your link down with it)
	//here would be a good place to try to re-init/flush/fix whatever caused the error, and hop
	//back to the main while loop above, *if* you don't want the link to just close on an error.
	//
	ioctl(from,FSCC_IMMEDIATE_STATUS,&status);
	if(uselog==1) fprintf(log,"istatus:%lx\n",status);
	ioctl(from,FSCC_FLUSH_RX);
	goto topr;
	}
  if (uselog == 1)
    fclose(log);
}

int
main(int argc, char **argv)
{
  int master1, slave1, master2, slave2;
  struct termios tt;
  struct winsize ws;
  uint32_t status;
  char ptyname[1024];		/* XXX ? */

  if (argc < 2)
    {
      fprintf(stderr, "Usage: pty /dev/fsccX\n");
      exit(1);
    }
  if (argc > 2)
    uselog = 1;
  else
    uselog = 0;

  master2 = open(argv[1], O_RDWR);	//open the SuperFastcom/ESCC/FSCC port
  if (master2 < 0)
    err(1, "openfscc");
  ioctl(master2,FSCC_FLUSH_RX);
  ioctl(master2,FSCC_FLUSH_TX);
  ioctl(master2,FSCC_IMMEDIATE_STATUS,&status);
  /* open and set up ptys */
  if (tcgetattr(0, &tt) < 0)
    err(1, "tcgetattr");
  if (ioctl(0, TIOCGWINSZ, &ws) < 0)
    err(1, "ioctl(TIOCGWINSZ)");
  if (openpty(&master1, &slave1, ptyname, &tt, &ws) < 0)
    err(1, "openpty");
  printf("using:%s\n", ptyname);
  cfmakeraw(&tt);
  tt.c_lflag &= ~ECHO;
  if (tcsetattr(slave1, TCSAFLUSH, &tt) < 0)
    err(1, "tcsetattr");

  /* fork off... */
  //create two processes, one that runs shuffle2pty
  //and one that runs shuffle2sfc

  signal(SIGCHLD, reaper);
  switch (fork())
    {
    case -1:
      err(1, "fork");
    case 0:
      daemon(0, 0);
      switch (fork())
	{
	case -1:
	  err(1, "fork");
	case 0:
	  /* shuffle data program -> pty */
	  close(slave1);
	  close(slave2);
	  shuffle2pty(master2, master1, "copyin");
	  exit(0);
	}
      /* shuffle data pty -> program */
      close(slave2);
      shuffle2sfc(master1, master2, "copyout");
      exit(0);
    }
//display our allocated pty 
//  puts(ptyname);
  return 0;
}

/* $Id$ */
