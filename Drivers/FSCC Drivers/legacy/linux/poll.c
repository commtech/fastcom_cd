/******************************************************
 *
 * Copyright (C) 2000 Commtech, Inc. Wichita KS
 *
 * poll.c -- poll (select) function for fscc-pci module
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

#define __NO_VERSION__
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/module.h>	/* get MOD_DEC_USE_COUNT, not the version string */
#include <linux/version.h>	/* need it for conditionals in scull.h */

#include <linux/kernel.h>	/* printk() */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#include <linux/slab.h>
#else
#include <linux/malloc.h>	/* kmalloc() */
#endif
#include <linux/vmalloc.h>
#include <linux/fs.h>		/* everything... */
#include <linux/proc_fs.h>
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <asm/segment.h>	/* memcpy to/from fs */
#include <asm/system.h>		/* cli(), *_flags */
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include "fsccdrv.h"		/* local definitions */


unsigned int
fscc_poll(struct file *filp, poll_table * pt)
{
  Fscc_Dev *dev;
  unsigned int mask = 0;

  dev = filp->private_data;
  poll_wait(filp, &dev->rq, pt);
  poll_wait(filp, &dev->wq, pt);
  poll_wait(filp, &dev->sq, pt);

  if (dev->port_initialized)	//if the port isn't inited then skip the read
    {

      if (dev->usedma == 1)
	{
	  volatile DESC *rdes;
	  unsigned long size;
	  unsigned long fe;
	  unsigned long readsize;
	  int skipck;
	  int i;
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
// printk("%d complete\n",i);
		  //descriptor is complete!
		  if ((rdes->control & 0x40000000) != 0)
		    {
// printk("%d eof %x\n",i,rdes->control);         
		      //end of frame
		      //(rdes->control&0x3fffffff)is the total framesize
		      if ((dev->allowcutoff == 1))
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
		  else if ((dev->allowcutoff == 1))
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

		  goto checkwrite;
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
		  printk("fsccdrv:spun the list, all complete, no FE!-poll\n");
		  goto checkwrite;
		}

	    }
	  while (fe == 0);

	  if (fe > 0)
	    {
	      mask |= POLLIN | POLLRDNORM;
	    }

	}
      else
	{
//non dma read frame check
	  if (atomic_read(&dev->received_frames_pending) != 0)
	    {
	      mask |= POLLIN | POLLRDNORM;
	    }
	}

    checkwrite:
      if (dev->usedma == 1)
	{
	  if (get_t_empty(dev, 2))
	    {
	      mask |= POLLOUT | POLLWRNORM;
	    }
	}
      else
	{
	  if (atomic_read(&dev->transmit_frames_available) != 0)
	    {
	      mask |= POLLOUT | POLLWRNORM;
	    }
	}

      if (dev->status != 0)
	{
	  mask |= POLLPRI;
	}
    }

  return mask;

}
