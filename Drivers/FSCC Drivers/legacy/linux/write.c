/******************************************************
 *
 * Copyright (C) 2000 Commtech, Inc. Wichita KS
 *
 * write.c -- write function for fscc-pci module
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
#include <asm/io.h>
#include <asm/uaccess.h>
#include "fsccdrv.h"		/* local definitions */

ssize_t
fscc_write(struct file *filp, const char *buf, size_t count, loff_t * off)
{
  struct buf *tbuf;
  unsigned i;
  unsigned port;
  unsigned long flags;
  int reqdes;
  int temp;
#ifdef USE_2_6
  DEFINE_WAIT(wait);
#endif
  Fscc_Dev *dev = filp->private_data;
  port = dev->fsccbase;		//needed for register defines to work with portio functions calls
topofwrite:

//printk("%u write, count:%lu, available:%u current:%u\n",dev->dev_no,(unsigned long)count,atomic_read(&dev->transmit_frames_available),dev->current_txbuf);

  if (dev->port_initialized)
    {
      //check to make sure it will fit in the txbuffer
      if (count > (dev->settings.n_tbufs * dev->settings.n_tfsize_max))
	{
	  return -EFBIG;
	}

      reqdes = count / dev->settings.n_tfsize_max;
      if ((count % dev->settings.n_tfsize_max) != 0)
	{
	  reqdes++;
	}
      if (dev->usedma == 1)
	{
	  reqdes++;		//we need 1 for our "stop" descriptor
	}
// printk("write,%d :%d\n",get_t_empty(dev,2),reqdes);
      while (!(temp=get_t_empty(dev, reqdes)))
	{
//	printk("blocking:temp:%d\n",temp);
	  if (dev->transmit_delay == 1)
	    {
	      dev->transmit_delay = 0;
	      if (dev->usedma == 1)
		{
		  //        fscc_start_tx_dma(dev);
		  //
		  if ((dev->channel == 0))
		    {
//              printk("txstart0\n");
		      WRITE_PORT_ULONG(dev->bsrbase + 0x10, dev->phys_txbdalist[0]);	//poke address into base descriptor register
		      WRITE_PORT_ULONG(dev->bsrbase + 0x04, 0x00000002);	//gota request
		    }
		  else if ((dev->channel == 1))
		    {
		      WRITE_PORT_ULONG(dev->bsrbase + 0x018, dev->phys_txbdalist[0]);	//poke address into base descriptor register
		      WRITE_PORT_ULONG(dev->bsrbase + 0x08, 0x00000002);	//issue gotb request
		    }
		}
	      else
		{
//kind of by definition this should only be entered when the transmitter is idle
//so it is kind of overkill to block interrupts here, but...
#ifdef USE_2_6
		  spin_lock_irqsave(&dev->irqlock, flags);
#else
		  save_flags(flags);
		  cli();
#endif
		  fscc_tx_start(dev);
#ifdef USE_2_6
		  spin_unlock_irqrestore(&dev->irqlock, flags);
#else
		  restore_flags(flags);
#endif
		}
	    }
	  if (filp->f_flags & O_NONBLOCK)
	    {
	      return -EAGAIN;	//if nonblocking then dump
	    }
//  PDEBUG("write sleeping \n");//otherwise block on a free buffer
#ifdef USE_2_6
	  prepare_to_wait(&dev->wq, &wait, TASK_INTERRUPTIBLE);
	  if (!get_t_empty(dev, reqdes))
	    {
	      schedule();
	    }
	  finish_wait(&dev->wq, &wait);
#else

	  save_flags(flags);
	  cli();
	  if (!get_t_empty(dev, reqdes))
	    {
	      interruptible_sleep_on(&dev->wq);
	    }
	  restore_flags(flags);
#endif

	  if (signal_pending(current))
	    {
	      return -ERESTARTSYS;
	    }
	  goto topofwrite;
	}			//end of while not transmit_buffers_available

//here we have at least 1 transmit buffer, so fill it up and possibly start 
//the interrupt tx chain
      if (dev->usedma == 1)
	{
	  DESC *tdesc;
	  unsigned long writesize;
	  unsigned long size;
	  unsigned long offset;
//printk("write.c using DMA!\n");
	  i = dev->current_txbuf;
	  size = count;
	  offset = 0;
	  atomic_dec(&dev->transmit_frames_available);	//ISR increments this as it finishes sending frames, we decrement it as we queue them 

	  do
	    {
	      tdesc = dev->Txbdalist[i];
	      if ((tdesc->control == 0x40000000) || (tdesc->control == 0x60000000))
		{
		  //untouched desc, fill er up
		  writesize = (size >= dev->settings.n_tfsize_max) ? dev->settings.n_tfsize_max : size;
		  if (copy_from_user(dev->txdata[i], &buf[offset], writesize))
		    {
		      return -EFAULT;
		    }
		  if (size == count)
		    {
		      tdesc->control = 0x80000000 | (count & 0x1fffffff);
		    }
		  else
		    {
		      tdesc->control = size & 0x1fffffff;
		    }
		  if ((i % dev->tirqrate) == 0)
		    {
		      tdesc->control |= 0x20000000;
		    }
		  tdesc->data_count = writesize;
		  size -= writesize;
		  offset += writesize;
//printk("tdesc->control:%8.8x\n",tdesc->control);
//printk("tdesc->data_count = %d\n",tdesc->data_count);
//printk("tdesc->next_desc:%8.8x\n",tdesc->next_desc);
		}
	      i++;
	      if (i == dev->settings.n_tbufs)
		{
		  i = 0;	//rollover
		}
	      if (size == 0)
		{
		  tdesc = dev->Txbdalist[i];
		  if ((tdesc->control & 0x1fffffff) != 0)
		    {
		      printk
			("um we have a problem, the last descriptor in the chain is not complete and we are whacking it\n");
		    }
		  tdesc->control = 0x40000000;	//this is the end, will get overwritten later if we do another write, or it will get consumed as a "stop" indicator
		  if ((i % dev->tirqrate) == 0)
		    {
		      tdesc->control |= 0x20000000;
		    }

		  if (dev->transmit_delay == 0)
		    {
		      fscc_start_tx_dma(dev);
		    }
		  dev->current_txbuf = i;
		  return count;
		}
	    }
	  while (i != dev->current_txbuf);

	  printk("fsccdrv:wrapped tbufs in write!\n");
	  return -EAGAIN;

	}
      else
	{
	  int cpsz;
	  int sz;
	  int szin;
//find the free txbuffer
//note it must be done while isr is disabled
#ifdef USE_2_6
	  spin_lock_irqsave(&dev->irqlock, flags);
#else

	  save_flags(flags);
	  cli();		//XDU occurs above this, isr handles it and all is well, we will go through the is_transmitting =0 case below and start up a new transmit
//XDU occurs between the cli() and the restore_flags() then it won't get to the 
//isr until after the restore flags, and is_transmitting =1, the command is lost
//along with the 32 bytes of data, but the new xdu/exe handler will restart the send

#endif
	  szin = 0;
	  sz = count;
	  i = dev->current_txbuf;
	  tbuf = &dev->fscc_tbuf[i];
	  while (tbuf->valid == 1)
	    {
	      i++;
	      if (i == dev->settings.n_tbufs)
		{
		  i = 0;	//rollover
		}
	      tbuf = &dev->fscc_tbuf[i];
	      if (i == dev->current_txbuf)
		{
		  printk("ack...it said we had enough buffers, but we scanned them all! and none are ready\n");
		  return -EBUSY;
		}
	    }
#ifdef USE_2_6
	  spin_unlock_irqrestore(&dev->irqlock, flags);
#else
	  restore_flags(flags);
#endif

	  tbuf->fe = 1;
	  while (sz > 0)
	    {
	      tbuf = &dev->fscc_tbuf[i];
	      if (tbuf->valid == 0)
		{
		  atomic_dec(&dev->transmit_frames_available);	//ISR increments this as it finishes sending frames, we decrement it as we queue them 
		  cpsz = (sz > dev->settings.n_tfsize_max) ? dev->settings.n_tfsize_max : sz;

		  if (copy_from_user(tbuf->frame, &buf[szin], cpsz))
		    {
		      return -EFAULT;
		    }
		  tbuf->max = cpsz;
		  tbuf->no_bytes = 0;
		  tbuf->valid = 1;
		  tbuf->ftot = count;
		  if (sz != count)
		    {
		      tbuf->fe = 0;
		    }
		  szin += cpsz;
		  sz -= cpsz;
		  //check to see if we need to start the send.

		}		//end of if valid==0;

	      i++;
	      if (i == dev->settings.n_tbufs)
		{
		  i = 0;	//rollover
		}
	    }
//	  if (dev->is_transmitting == 0)
//	    {
//printk("is transmitting is 0, (tx is idle)\n");
	      //is_transmitting flag is 0 so we are not currently transmitting, and there
	      //are not going to be any more scheduled interrupts of the transmit varity 
	      //until we issue a transmit command, so lets start-er up.

#ifdef USE_2_6
		  spin_lock_irqsave(&dev->irqlock, flags);
#else
		  save_flags(flags);
		  cli();
#endif
		  fscc_tx_start(dev);
#ifdef USE_2_6
		  spin_unlock_irqrestore(&dev->irqlock, flags);
#else
		  restore_flags(flags);
#endif


//	    }			//end of is_transmitting check

	  return count;

	}
    }				//end of if port_initialized
  else
    {
      return -EBUSY;
    }


}
