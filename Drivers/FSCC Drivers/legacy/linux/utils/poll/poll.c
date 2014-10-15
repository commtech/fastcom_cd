/* $Id */
/*
 * poll.c - a user utility to test the poll/select function of the
 * superfastcom driver
 *
 * */
#define MAX_FSCC_PORTS 2	//number of minors (ports or #boards*2) that exist

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <math.h>
#include <asm/poll.h>
#include "../fsccdrv.h"


int
main(int argc, char *argv[])
{
  char buf[4096];		//received frame data buffer
  unsigned j;			//temp var
  char nbuf[80];		//port name buffer
  unsigned rcount;		//receive count
  struct pollfd pfds[MAX_FSCC_PORTS];	//poll struct
  int ret;			//return value
  unsigned long tocount;	//timeout counter
  unsigned num_open_fscc = 0;	//number of fscc ports that we are working with
  unsigned open_port[MAX_FSCC_PORTS];	//port # of open port
  int doread;			//need to read flag
  int dostatus;			//need to call status flag
  unsigned long status;
  unsigned i;			//temp var


  num_open_fscc = 0;		//start with 0

  for (i = 0; i < MAX_FSCC_PORTS; i++)	//try to open them all
    {
//give the filename

      sprintf(nbuf, "/dev/fscc%u", i);

      pfds[num_open_fscc].fd = open(nbuf, O_RDWR);
      if (pfds[num_open_fscc].fd == -1)
	{
	  //no file so skip this one
	  printf("cannot open %s\n", nbuf);
	  perror(NULL);
	}
      else
	{
//file opened so set it up

	  printf("opened %s\r\n", nbuf);
	  pfds[num_open_fscc].events = POLLIN | POLLPRI | POLLERR;	//we onlyl care about receives and errors
	  open_port[num_open_fscc] = i;	//save port#
	  num_open_fscc++;	//make next one use next in line
	}

    }


  do
    {
//main loop - continues to run until 1min without receive
      tocount = 0;		//reset timeout count on each receive
      do
	{			//keep poll call going until an event gets signaled
	  if ((ret = poll(pfds, num_open_fscc, 500)) < 0)
	    {
	      perror(NULL);
	    }
	  else
	    {
	      if (ret == 0)
		{
		  if (tocount == 0)
		    {
		      printf("\nreceive-timeout\n");
		    }
		  else
		    {
		      printf(".");	//this doesn't work well in linux, because it is linebuffered...
		    }
		  tocount++;	//inc timeout count
		  if (tocount > 120)	//if we hit 1min then drop out
		    {
		      printf("\n1min without received frame ... exiting\n");
		      goto toend;
		    }
		}
	    }
	}
      while (ret <= 0);		//as long as just timeouts keep calling poll

//if ret > 0 then at least one event is signaled in the array of poll structs
//scan them all
      for (i = 0; i < num_open_fscc; i++)
	{
	  doread = 0;
	  dostatus = 0;
	  printf("\n");
	  if ((pfds[i].revents & POLLIN) == POLLIN)
	    {
//this struct is marked as needing to be read, so lets do it
	      printf("port:%u pollin\n", open_port[i]);
	      doread = 1;
	    }
	  if ((pfds[i].revents & POLLPRI) == POLLPRI)
	    {
//this struct is marked as needing to be read(priority read), so lets do it
	      printf("port:%u pollpri\n", open_port[i]);
	      dostatus = 1;
	    }

	  if ((pfds[i].revents & POLLERR) == POLLERR)
	    {
	      //I'm not sure how this happens yet, but you could put an error
	      //handler here (it is actually status/istatus)
	      printf("port:%u pollerr\n", open_port[i]);
	    }

	  if (doread == 1)
	    {
//if flagged to read then do the read on this port
	      if ((rcount = read(pfds[i].fd, buf, 4096)) == -1)
		{
		  printf("read failed:errno = %i\n", errno);
		  perror(NULL);
		}
//this is just display stuff, an actual program would decode/do something
//with the message data here

	      for (j = 0; j < 40; j++)
		{
		  printf("-");
		}
	      printf("\nPort:%u Read returned:%i bytes\n", open_port[i], rcount);
	      for (j = 0; j < rcount - 2; j++)
		{
		  printf("%c", buf[j]);
		}
	      printf("\n");
//these only make sense for HDLC...
	      printf("last bytes:%x:%x\n", buf[rcount - 2] & 0xff, buf[rcount - 1] & 0xff);
	      if ((buf[rcount - 1] & 0x02) == 0x00)
		{
		  printf("CRC-PASSED ");
		}
	      else
		{
		  printf("CRC-FAILED ");
		}
	      if ((buf[rcount - 1] & 0x08) == 0x00)
		{
		  printf("VALID ");
		}
	      else
		{
		  printf("INVALID ");
		}
	      if ((buf[rcount - 1] & 0x20) == 0x20)
		{
		  printf("RDO ");
		}
	      if ((buf[rcount - 1] & 0x01) == 0x01)
		{
		  printf("RECEIVE ABORTED ");
		}
	      if ((buf[rcount - 1] & 0x04) == 0x04)
		{
		  printf("Receive Length Exceeded ");
		}
	      if ((buf[rcount - 1] & 0x10) == 0x10)
		{
		  printf("Receive Frame Lost ");
		}
	      printf("\n");
	      for (j = 0; j < 40; j++)
		{
		  printf("-");
		}
	      printf("\n");
	    }			//end of doread block
	  if (dostatus == 1)
	    {
	      status = 0xffffffff;	//mask value        
	      ioctl(pfds[i].fd, FSCC_STATUS, &status);
	      printf("Port:%u STATUS: 0x%8.8x\n", open_port[i], status);

	      if ((status & ST_CTSA) == ST_CTSA)
		{
		  printf("ST_CTSA --CTS abort interrupt\n");
		}
	      if ((status & ST_CDC) == ST_CDC)
		{
		  printf("ST_CDC  --CD changed state\n");
		}
	      if ((status & ST_DSRC) == ST_DSRC)
		{
		  printf("ST_DSRC --DSR changed state\n");
		}
	      if ((status & ST_CTSS) == ST_CTSS)
		{
		  printf("ST_CTSS --CTS changed state\n");
		}
	      if ((status & ST_TIN) == ST_TIN)
		{
		  printf("ST_TIN  --Timer Expired\n");
		}
	      if ((status & ST_RFS) == ST_RFS)
		{
		  printf("ST_RFS  --Receive Frame Start\n");
		}
	      if ((status & ST_RFL) == ST_RFL)
		{
		  printf("ST_RFL  --Receive Frame Lost\n");
		}
	      if ((status & ST_RDO) == ST_RDO)
		{
		  printf("ST_RDO  --Receive Data overflow\n");
		}
	      if ((status & ST_TDU) == ST_TDU)
		{
		  printf("ST_TDU  --Transmit Data underrun\n");
		}
	      if ((status & ST_OVF) == ST_OVF)
		{
		  printf("ST_OVF  --OverFlow\n");
		}
	      if ((status & ST_RX_DONE) == ST_RX_DONE)
		{
		  printf("ST_RX_DONE --Receive Complete\n");
		}
	      if ((status & ST_ALLS) == ST_ALLS)
		{
		  printf("ST_ALLS --All Sent\n");
		}
	      if ((status & ST_TX_DONE) == ST_TX_DONE)
		{
		  printf("ST_TX_DONE --Transmit Complete\n");
		}
	      if ((status & ST_RFT) == ST_RFT)
		{
		  printf("ST_RFT  --Receive Fifo Trigger\n");
		}
	      if ((status & ST_TFT) == ST_TFT)
		{
		  printf("ST_TFT  --Transmit Fifo Trigger\n");
		}
	      if ((status & ST_RFE) == ST_RFE)
		{
		  printf("ST_RFE  --Receive Frame End\n");
		}
	      if ((status & ST_RBUF_OVERFLOW) == ST_RBUF_OVERFLOW)
		{
		  printf("ST_RBUF_OVERFLOW --Receive Buffers overflowed\n");
		}
	      if ((status & ST_DMAT_S) == ST_DMAT_S)
		{
		  printf("ST_DMAT_S --Dma Transmit Stopped\n");
		}
	      if ((status & ST_DMAR_S) == ST_DMAR_S)
		{
		  printf("ST_DMAR_S --Dma Receive Stopped\n");
		}
	      if ((status & ST_TX_HI) == ST_TX_HI)
		{
		  printf("ST_TX_HI  --transmit host interrupt\n");
		}
	      if ((status & ST_RX_HI) == ST_RX_HI)
		{
		  printf("ST_RX_HI  --receive host interrupt\n");
		}
	    }
	}			//end of for i=0;i<MAX_FSCC_PORTS block
    }
  while (1);			//end of main loop

toend:
//get here on 1min timeout
  for (i = 0; i < num_open_fscc; i++)
    {
      close(pfds[i].fd);	//close all open files
    }
  return 0;			//done
}

/* $Id */
