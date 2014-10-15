/******************************************************
 *
 * Copyright (C) 2000 Commtech, Inc. Wichita KS
 *
 * read.c -- read function for fscc-pci module
 *
 ******************************************************/

/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

If you encounter problems or have suggestions and/or bug fixes please email them to:

custserv@commtech-fastcom.com

or mailed to:

Commtech, Inc.
9011 E. 37th N.
Wichita, KS 67226
ATTN: Linux BugFix Dept

*/

#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

#define __NO_VERSION__		/* don't define kernel_verion in module.h */
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>	/* printk() */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#include <linux/slab.h>
#else
#include <linux/malloc.h>	/* kmalloc() */
#endif
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <asm/system.h>		/* cli(), *_flags */
#include <asm/segment.h>	/* memcpy and such */
#include <asm/atomic.h>
#include <asm/uaccess.h>
#include "fsccdrv.h"		/* local definitions */

ssize_t
fscc_read(struct file * filp, char *buf, size_t count, loff_t * off)
{
  count_t retval;
  unsigned i;
  struct buf *rbuf;
  int ftr;
  int szout;
#ifdef USE_2_6
  DEFINE_WAIT(wait);
#else
  unsigned long flags;
#endif
  Fscc_Dev *dev = filp->private_data;	//our device information
  PDEBUG("%u read, count:%lu, frames_pending:%u\n", dev->dev_no, (unsigned long) count,
	 atomic_read(&dev->received_frames_pending));
topofread:

//printk("read:%lu\n",(unsigned long) count);
  if (dev->port_initialized)	//if the port isn't inited then skip the read
    {
      if (dev->usedma == 1)
	{
	  volatile DESC *rdes;
	  unsigned long size;
	  unsigned long fe;
	  unsigned long readsize;
	  int skipck;
	  skipck = 0;
//printk("read.c using DMA!\n");
	  i = dev->current_rxbuf;
	  size = 0;
	  fe = 0;
	  do
	    {
	      rdes = dev->Rxbdalist[i];
//printk("%d :%8.8x\n",i,rdes->control);
	      if ((rdes->control & 0x80000000) != 0)
		{
		  //printk("%d complete\n",i);
		  //descriptor is complete!
		  if ((rdes->control & 0x40000000) != 0)
		    {
// printk("%d eof %x\n",i,rdes->control);         
		      //end of frame
		      //(rdes->control&0x3fffffff)is the total framesize
		      if ((dev->allowcutoff == 1) && (count == rdes->data_count)
			  && (count == dev->settings.n_rfsize_max))
			{
			  //if you want that partial frame, then set about retrieving it here, but you will have to figure out how many bytes are in it
			  //(ie keep a running track of how many bytes have been returned via the cutoff bit)
			  //this should be the total bytes returned since either the last read from a frame end completed, or the last flushrx/reset receiver
			  readsize = (rdes->control & 0x3fffffff) - dev->rdtotal;
			  size += readsize;
			  fe = 1;
			}
		      else
			{
			  //regular frame end code
			  readsize = (rdes->control & 0x3fffffff) - size;
			  size += readsize;
			  fe = 1;
			}
		    }
		  else if ((dev->allowcutoff == 1) && (count == rdes->data_count)
			   && (count == dev->settings.n_rfsize_max))
		    {
		      //read cutoff mode, return full descriptor/buffers
		      fe = 1;
		      readsize = (rdes->data_count);
		      size += readsize;
		    }
		  else
		    {
		      //all else fails, no frame end, just aggregate the bytes until we get to a frame end
		      size += (rdes->data_count);
		      readsize = rdes->data_count;
		    }
		}
	      else
		{
		  //The next descriptor is not complete, 
		  //so block
		  //printk("read blocked\n");
		  if (filp->f_flags & O_NONBLOCK)
		    {
		      return -EAGAIN;	//don't block if specified nonblocking read
		    }
		  //  PDEBUG("read sleaping\n");
#ifdef ALLOW_READ_TIMEOUT
		  if (dev->rblockcnt == 2)
		    {
		      dev->rblockcnt = 0;
		      return 0;
		    }
#endif
		  setup_tmr(dev);
#ifdef USE_2_6
		  prepare_to_wait(&dev->rq, &wait, TASK_INTERRUPTIBLE);
		  schedule();
		  finish_wait(&dev->rq, &wait);
#else
		  save_flags(flags);
		  cli();	//keep ints from happening before sleepon is called
		  interruptible_sleep_on(&dev->rq);
		  restore_flags(flags);
#endif
		  del_timer(&dev->rqt);
		  //printk("um unblocked?\n");
		  //if we got signaled then restart the call so it will complete correctly
		  if (signal_pending(current))
		    {
		      return -ERESTARTSYS;
		    }
		  goto topofread;
		}

	      i++;
	      if (i == dev->settings.n_rbufs)
		{
		  i = 0;
		}
	      if (i == dev->current_rxbuf)
		{
		  //we have spun the whole list, and all descriptors are complete, but there is no frame end indication
		  //likely we have a problem here...
		  printk("fsccdrv:spun the list, all complete, no FE!\n");
		  /*
		  printk("current:%d\n",dev->current_rxbuf);
		  for(i=0;i<dev->settings.n_rbufs;i++)
		  	{
		      	rdes = dev->Rxbdalist[i];
			printk("[%d]:%x\n",i,rdes->control);
		  	}
		  */
		  return -EFBIG;	//don't know what else to return here 
		}

	    }
	  while (fe == 0);
	  dev->rblockcnt = 0;
//we have a complete frame so return it here
	  if (size > count)
	    {
		/*
		  printk("current:%d\n",dev->current_rxbuf);
		  for(i=0;i<dev->settings.n_rbufs;i++)
		  	{
		      	rdes = dev->Rxbdalist[i];
			printk("[%d]:%x\n",i,rdes->control);
		  	}
		*/

	      return -EFBIG;	//we must return a complete frame,
	    }
//we determined that we have a frame above
//set about actually returning the frame here
	  i = dev->current_rxbuf;
	  size = 0;
	  readsize = 0;
	  fe = 0;
	  do
	    {
	      rdes = dev->Rxbdalist[i];
	      if ((rdes->control & 0x80000000) != 0)
		{
		  //descriptor is complete!
		  if ((rdes->control & 0x40000000) != 0)
		    {
		      if ((dev->allowcutoff == 1) && (count == rdes->data_count)
			  && (count == dev->settings.n_rfsize_max))
			{
			  readsize = (rdes->control & 0x3fffffff) - dev->rdtotal;
			  dev->rdtotal = 0;
			  //the frame end will generate the receive status, these bytes are completely bogus in this context
			  //so strip them here.
			  //you could return them, but it would make the logic for receiving data in userland more complex
			  //particularly if data ended at n_rfsize_max-2 bytes...
			  //
			  if (readsize >= 2)
			    {
			      readsize -= 2;
			    }
			  fe = 1;
			  skipck = 1;
			}
		      else
			{
			  //end of frame
			  //(rdes->control&0x3fffffff)is the total framesize
			  readsize = (rdes->control & 0x3fffffff) - size;
			  fe = 1;
			}
		    }
		  else if ((dev->allowcutoff == 1) && (count == rdes->data_count)
			   && (count == dev->settings.n_rfsize_max))
		    {
		      fe = 1;
		      readsize = (rdes->data_count);
		      skipck = 1;
		      dev->rdtotal += readsize;
		    }
		  else
		    {
		      readsize = rdes->data_count;
		    }
		  //copy readsize bytes to buffer
		  if ((skipck == 0) && (fe == 1) && ((size + readsize) != (rdes->control & 0x3fffffff)))
		    {
		      printk("fsccdrv:err, size mismatch%ld!=%d\n", size + readsize, rdes->control & 0x3fffffff);
		    }
		  if (copy_to_user(&buf[size], &dev->rxdata[i][0], readsize))
		    {
		      return -EFAULT;	//send frame to user
		    }
		  //printk("readsize:%ld\n",readsize);
		  size += readsize;
		  if ((i % dev->rirqrate) == 0)
		    {
		      rdes->control = 0x20000000;
		    }
		  else
		    {
		      rdes->control = 0;
		    }
		  i++;
		  if (i == dev->settings.n_rbufs)
		    {
		      i = 0;
		    }
		  if (fe == 1)
		    {
		      dev->current_rxbuf = i;
		      return size;
		    }
		}
	    }
	  while (i != dev->current_rxbuf);
//if here we have a problem!
	  printk("fsccdrv:past loop after fe indication!\n");
	  return -EAGAIN;
	}
      else
	{
//fscc -interrupt driven reads
//use dev->received_frames for rbuf sized returns
//
	  while (atomic_read(&dev->received_frames_pending) == 0)	//block if there are no frames
	    {
	      if (filp->f_flags & O_NONBLOCK)
		{
		  return -EAGAIN;	//don't block if specified nonblocking read
		}
//  PDEBUG("read sleaping\n");
#ifdef ALLOW_READ_TIMEOUT
	      if (dev->rblockcnt == 2)
		{
		  dev->rblockcnt = 0;
		  return 0;
		}
#endif

	      setup_tmr(dev);

#ifdef USE_2_6
	      prepare_to_wait(&dev->rq, &wait, TASK_INTERRUPTIBLE);
	      if (atomic_read(&dev->received_frames_pending) == 0)
		{
		  schedule();
		}
	      finish_wait(&dev->rq, &wait);
#else
	      save_flags(flags);
	      cli();		//keep ints from happening before sleepon is called
	      if (atomic_read(&dev->received_frames_pending) == 0)
		{
		  interruptible_sleep_on(&dev->rq);
		}
	      restore_flags(flags);
#endif
	      del_timer(&dev->rqt);
	      //printk("um unblocked?\n");
	      //if we got signaled then restart the call so it will complete correctly
//if we got signaled then restart the call so it will complete correctly
	      if (signal_pending(current))
		{
		  return -ERESTARTSYS;
		}

	    }			//end of while not pending frames
	  dev->rblockcnt = 0;
//if we get here we have at least one frame to return so return it
//we have to do this with interrupts disabled so that our ISR doesn't update
//the dev->current_rxbuf value while we are using it
	  i = dev->current_rxbuf_out;	//get the current buffer
	  ftr = 0;
	  szout = 0;
	  do
	    {
	      rbuf = &dev->fscc_rbuf[i];	//get pointer to this buffer
	      if (rbuf->valid == 1)	//it is assumed that 1 means the data in the buffer is valid
		{
		  szout += rbuf->no_bytes;
		  if (rbuf->fe == 1)
		    {
		      ftr++;
		    }
		}
	      i++;		//start at current +1
	      if (i == dev->settings.n_rbufs)
		{
		  i = 0;	//rollover
		}
	    }
	  while ((ftr < 1) && (i != dev->current_rxbuf_out));

	  if ((i == dev->current_rxbuf_out) && (ftr == 0))
	    {
	      PDEBUG("ack, received_frames_pending>0 with no FE!:%d,%d,%d,%d\n",
		     atomic_read(&dev->received_frames_pending), ftr, i, dev->current_rxbuf_out);
	      return -EBUSY;
	    }
	  if (szout > count)
	    {
	      return -EFBIG;	//we must return a complete frame,
//I realize that the book says that you can return less than count bytes
//but it really does not make sense to break up a frame into multiple reads for
//any of the operating modes execpt async.  And if you want to re-write it
//to make async return partial frames then so be it.
// if their buffer isn't big enough pump back an error...
//would really like a different one, but haven't figured errno's out quite yet
	    }
	  i = dev->current_rxbuf_out;	//get the current buffer
	  ftr = 0;
	  szout = 0;
	  do
	    {
	      rbuf = &dev->fscc_rbuf[i];	//get pointer to this buffer
	      if (rbuf->valid == 1)	//it is assumed that 1 means the data in the buffer is valid
		{
		  if (copy_to_user(&buf[szout], rbuf->frame, rbuf->no_bytes))
		    {
		      return -EFAULT;	//send frame to user
		    }
		  szout += rbuf->no_bytes;
		  rbuf->valid = 0;	//make buffer free for later
		  rbuf->no_bytes = 0;	//clear for next use
		  rbuf->ftot = 0;
		  if (rbuf->fe == 1)
		    {
		      rbuf->fe = 0;
		      retval = szout;
		      atomic_dec(&dev->received_frames_pending);	//decrement pending frame count
		      dev->current_rxbuf_out++;
		      if (dev->current_rxbuf_out == dev->settings.n_rbufs)
			{
			  dev->current_rxbuf_out = 0;	//rollover
			}

		      ftr++;
		      return retval;
		    }
		  dev->current_rxbuf_out++;
		  if (dev->current_rxbuf_out == dev->settings.n_rbufs)
		    {
		      dev->current_rxbuf_out = 0;	//rollover
		    }


//  atomic_dec(&dev->received_frames_pending);//decrement pending frame count
		}		//end of if rbuf->valid
	      i++;		//start at current +1
	      if (i == dev->settings.n_rbufs)
		{
		  i = 0;	//rollover
		}
	    }
	  while ((ftr < 1));	//keep looping until we get back to the current one
//really should not get here
	  PDEBUG("past while loop in read after received_frames_pending >0\n");
	  return -EAGAIN;
	}
    }				//end of port initialized
  else
    {
      return -EBUSY;
    }

}				//end of read
