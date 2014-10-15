/******************************************************
 *
 * Copyright (C) 2000 Commtech, Inc. Wichita KS
 *
 * seek.c -- seek function for fscc-pci module
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

techsupport@commtech-fastcom.com

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
#include "fsccdrv.h"		/* local definitions */


long long
fscc_lseek(struct file *filp, loff_t off, int whence)
{
  //Fscc_Dev *dev = filp->private_data;
  long newpos;
  PDEBUG("seek-entry\n");
  newpos = 0;
//it makes 0 sense to seek this device
//absolutely no sense
//always set fpos to 0.
  switch (whence)
    {
    case 0:			/* SEEK_SET */
      //newpos = off;
      break;

    case 1:			/* SEEK_CUR */
      //newpos = filp->f_pos + off;
      break;

    case 2:			/* SEEK_END */
      // newpos = dev->size + off;
      break;

    default:			/* can't happen */
      return -EINVAL;
    }
  if (newpos < 0)
    {
      return -EINVAL;
    }
  filp->f_pos = newpos;
  return newpos;
}
