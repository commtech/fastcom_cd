/******************************************************
 *
 * Copyright (C) 2000 Commtech, Inc. Wichita KS
 *
 * sync.c -- sync function for fscc-pci module
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
#include <linux/timer.h>
#ifdef USE_2_6
#include <linux/wait.h>
#endif
#include <asm/system.h>		/* cli(), *_flags */
#include <asm/segment.h>	/* memcpy and such */
#include <asm/atomic.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "fsccdrv.h"		/* local definitions */


void sync_timeout(unsigned long ptr);
#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,35)
int
fscc_fsync(struct file *filp, int datasync)
#elif LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)
int
fscc_fsync(struct file *filp, struct dentry *de, int datasync)
#else
int
fscc_fsync(struct file *filp, struct dentry *de)
#endif
{
  Fscc_Dev *dev;
#ifdef USE_2_6
#else
  struct timer_list fscc_fsync_t1;
#endif

  dev = filp->private_data;
  PDEBUG("fsync -entry\n");
  if (dev->usedma == 1)
    {
      while (get_numt_empty(dev) != dev->settings.n_tbufs)
	{
	  //should block here...but on what...
#ifdef USE_2_6
	  wait_event_interruptible_timeout(dev->fscc_fsync_wait, (get_numt_empty(dev) == dev->settings.n_tbufs), HZ);
#else
	  init_timer(&fscc_fsync_t1);
	  fscc_fsync_t1.function = sync_timeout;
	  fscc_fsync_t1.data = dev;
	  fscc_fsync_t1.expires = jiffies + HZ;
	  add_timer(&fscc_fsync_t1);
	  interruptible_sleep_on(&dev->fscc_fsync_wait);
#endif
	  PDEBUG("fsync-timeout\n");
	  if (signal_pending(current))
	    {
	      return -EBUSY;
	    }

	}
    }
  else
    {
      while (atomic_read(&dev->transmit_frames_available) != dev->settings.n_tbufs)
	{
	  //should block here...but on what...
#ifdef USE_2_6
	  wait_event_interruptible_timeout(dev->fscc_fsync_wait,
					   (atomic_read(&dev->transmit_frames_available) == dev->settings.n_tbufs), HZ);
#else
	  init_timer(&fscc_fsync_t1);
	  fscc_fsync_t1.function = sync_timeout;
	  fscc_fsync_t1.data = dev;
	  fscc_fsync_t1.expires = jiffies + HZ;
	  add_timer(&fscc_fsync_t1);
	  interruptible_sleep_on(&dev->fscc_fsync_wait);
#endif
	  PDEBUG("fsync-timeout\n");
	}
    }
//should check for ST_ALLS also here...
  PDEBUG("fsync -exit\n");
  return 0;
}

void
sync_timeout(unsigned long ptr)
{
  Fscc_Dev *dev;

  dev = (Fscc_Dev *) ptr;

  wake_up_interruptible(&dev->fscc_fsync_wait);
}
