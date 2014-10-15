/******************************************************
 *
 * Copyright (C) 2000 Commtech, Inc. Wichita KS
 *
 * ioctl.c -- ioctl function for fscc-pci module
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
#include <linux/kernel.h>
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
#include <asm/segment.h>	/* memcpy to/from fs */
#include <asm/system.h>		/* cli(), *_flags */
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include "fsccdrv.h"		/* local definitions */

/*
 * The ioctl() implementation
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 11))
long
fscc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
int 
fscc_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
  unsigned long flags;
  unsigned long status;
  unsigned port;
  unsigned i, j;
  char *tempbuf;
  Fscc_Dev *dev;
//board_settings board_switches;
  regsingle regs;
  unsigned long tempul;
  unsigned long features;
  struct clockstruct clk;
  uint32_t ver;
#ifdef USE_2_6
  DEFINE_WAIT(wait);
#endif

//    int err = 0;
  //int tmp;
  //int size = _IOC_SIZE(cmd); /* the size bitfield in cmd */
  dev = filp->private_data;
  port = dev->fsccbase;
  /*
   * extract the type and number bitfields, and don't decode
   * wrong cmds: return EINVAL before verify_area()
   */

  //printk("fsccp:%u ioctl-entry:cmd :%u\n",dev->dev_no,_IOC_NR(cmd));
  if (_IOC_TYPE(cmd) != FSCC_IOC_MAGIC)
    {
      return -EINVAL;
    }
  if (_IOC_NR(cmd) > FSCC_IOC_MAXNR)
    {
      return -EINVAL;
    }
//printk("fsccp:%u ioctl-entry:cmd :%u\n",dev->dev_no,_IOC_NR(cmd));
//printk("tsc:%Ld\r\n",get__tsc());
  /*
   * the direction is a bitmask, and VERIFY_WRITE catches R/W
   * transfers. `Type' is user-oriented, while
   * verify_area is kernel-oriented, so the concept of "read" and
   * "write" is reversed
   */
/* //obsoleted by v 2.1 kernel
    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        err = verify_area(VERIFY_WRITE, (void *)arg, size);
	}
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err =  verify_area(VERIFY_READ, (void *)arg, size);
	}
    if (err)
    {
    return err;
    }
*/
  switch (cmd)
    {

    case FSCC_INIT:
//printk("fscc-init\n");
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;	//don't try to init a board that isn't there
	}
      if (dev->usedma == 1)
	{
	  cleanup_allocs(dev);	//have to free up the old settings before they get overwritten in the copy step below.
	}
//get user settings
//

      if (copy_from_user(&dev->settings, (setup *) arg, sizeof(setup)))
	{
	  return -EFAULT;
	}
      WRITE_INTERRUPT_MASK(port, 0xffffffff);

      WRITE_CCR0(port, dev->settings.ccr0);
      WRITE_CCR1(port, dev->settings.ccr1);
      WRITE_CCR2(port, dev->settings.ccr2);
      WRITE_ANY(port + FTRIG, dev->settings.fifot);
      WRITE_BGR(port, dev->settings.bgr);

      WRITE_TIMER_CONTROL(port, dev->settings.tcr);
      WRITE_PREAMBLE(port, dev->settings.ppr);
      WRITE_RECEIVE_ADDRESS_MASK(port, dev->settings.ramr);
      WRITE_RECEIVE_ADDRESS(port, dev->settings.rar);
      WRITE_TERMINATION_SEQUENCE_MASK(port, dev->settings.tmr);
      WRITE_TERMINATION_SEQUENCE(port, dev->settings.tsr);
      WRITE_SYNC_SEQUENCE_MASK(port, dev->settings.smr);
      WRITE_SYNC_SEQUENCE(port, dev->settings.ssr);

      //should probably force XRES and RRES here
      if (WRITE_COMMAND(port, dev->settings.cmdr) != 0)
	{
	  return -EBUSY;	//timeout on CE
	}

//printk("fscc-init,past io writes\n");

//check limits on n_rbufs/ntbufs/n_rfsize_max/n_tfsize_max 
//and free if allready allocated
      if (dev->settings.n_rbufs < 2)
	{
	  dev->settings.n_rbufs = 2;	//
	}
      if (dev->settings.n_tbufs < 2)
	{
	  dev->settings.n_tbufs = 2;	//
	}
      if (dev->settings.n_rfsize_max < 64)
	{
	  dev->settings.n_rfsize_max = 64;	//
	}
      if (dev->settings.n_tfsize_max < 64)
	{
	  dev->settings.n_tfsize_max = 64;	//
	}
      if (dev->usedma == 1)
	{
//allocate memory for dma
	  if (allocate_desc(dev) != 0)
	    {
	      return -ENOMEM;	//its the only error possible here
	    }
	  init_tx_desc(dev);
	  init_rx_desc(dev);
	  init_rx_dma(dev);

	}
      else
	{

	  if ((dev->fscc_rbuf != NULL) && (dev->fscc_rbuf[0].frame != NULL))
	    {
	      vfree(dev->fscc_rbuf[0].frame);
	    }
	  if ((dev->fscc_tbuf != NULL) && (dev->fscc_tbuf[0].frame != NULL))
	    {
	      vfree(dev->fscc_tbuf[0].frame);
	    }
	  if (dev->fscc_rbuf != NULL)
	    {
	      vfree(dev->fscc_rbuf);
	    }
	  if (dev->fscc_tbuf != NULL)
	    {
	      vfree(dev->fscc_tbuf);
	    }
//allocate memory for buffers

	  dev->fscc_rbuf = (struct buf *) vmalloc(dev->settings.n_rbufs * sizeof(struct buf));
	  if (dev->fscc_rbuf == NULL)
	    {
	      return -ENOMEM;
	    }
	  dev->fscc_tbuf = (struct buf *) vmalloc(dev->settings.n_tbufs * sizeof(struct buf));
	  if (dev->fscc_tbuf == NULL)
	    {
	      vfree(dev->fscc_rbuf);
	      return -ENOMEM;
	    }
	  tempbuf = (char *) vmalloc(dev->settings.n_rbufs * dev->settings.n_rfsize_max);
	  if (tempbuf == NULL)
	    {
	      vfree(dev->fscc_rbuf);
	      vfree(dev->fscc_tbuf);
	      return -ENOMEM;
	    }
	  for (i = 0; i < dev->settings.n_rbufs; i++)
	    {
	      dev->fscc_rbuf[i].frame = &tempbuf[i * dev->settings.n_rfsize_max];
	      dev->fscc_rbuf[i].valid = 0;
	      dev->fscc_rbuf[i].no_bytes = 0;
	      dev->fscc_rbuf[i].max = 0;
	    }
	  tempbuf = (char *) vmalloc(dev->settings.n_tbufs * dev->settings.n_tfsize_max);
	  if (tempbuf == NULL)
	    {
	      vfree(dev->fscc_rbuf[0].frame);
	      vfree(dev->fscc_rbuf);
	      vfree(dev->fscc_tbuf);
	      return -ENOMEM;
	    }
	  for (i = 0; i < dev->settings.n_tbufs; i++)
	    {
	      dev->fscc_tbuf[i].frame = &tempbuf[i * dev->settings.n_tfsize_max];
	      dev->fscc_tbuf[i].valid = 0;
	      dev->fscc_tbuf[i].no_bytes = 0;
	      dev->fscc_tbuf[i].max = 0;
	    }
	}
      atomic_set(&dev->transmit_frames_available, dev->settings.n_tbufs);
      atomic_set(&dev->received_frames_pending, 0);
//make the board live, enable interrupts

      dev->port_initialized = 1;
      WRITE_INTERRUPT_MASK(port, dev->settings.imr);
      dev->transmit_delay = 0;
//printk("INIT_COMPLETE\n");
      break;

    case FSCC_READ_REGISTER:
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}

      if (copy_from_user(&regs, (regsingle *) arg, sizeof(regsingle)))
	{
	  return -EFAULT;
	}
      regs.data = inl(dev->fsccbase + regs.port);

      PDEBUG("READ_REGISTER in:%lx ->%lx\n", (regs.port + dev->fsccbase), regs.data);

      if (copy_to_user((regsingle *) arg, &regs, sizeof(regsingle)))
	{
	  return -EFAULT;
	}
      break;

    case FSCC_WRITE_REGISTER:
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}

      if (copy_from_user(&regs, (regsingle *) arg, sizeof(regsingle)))
	{
	  return -EFAULT;
	}

      PDEBUG("WRITE_REGISTER out:%x ->%x\n", regs.data, (regs.port + dev->fsccbase));

      outl(regs.data, dev->fsccbase + regs.port);
      break;

    case FSCC_READ_REGISTER2:
      if (dev->uartbase == 0)
	{
	  return -ENODEV;
	}

      if (copy_from_user(&regs, (regsingle *) arg, sizeof(regsingle)))
	{
	  return -EFAULT;
	}
      regs.data = inb(dev->uartbase + regs.port);

      PDEBUG("READ_REGISTER in:%lx ->%lx\n", (regs.port + dev->uartbase), regs.data);

      if (copy_to_user((regsingle *) arg, &regs, sizeof(regsingle)))
	{
	  return -EFAULT;
	}
      break;

    case FSCC_WRITE_REGISTER2:
      if (dev->uartbase == 0)
	{
	  return -ENODEV;
	}

      if (copy_from_user(&regs, (regsingle *) arg, sizeof(regsingle)))
	{
	  return -EFAULT;
	}

      PDEBUG("WRITE_REGISTER out:%x ->%x\n", regs.data, (regs.port + dev->uartbase));

      outb(regs.data, dev->uartbase + regs.port);
      break;

    case FSCC_READ_REGISTER3:
      if (dev->bsrbase == 0)
	{
	  return -ENODEV;
	}

      if (copy_from_user(&regs, (regsingle *) arg, sizeof(regsingle)))
	{
	  return -EFAULT;
	}
      down(&boardlock[dev->board]);	//don't touch(read/modify/write) bsrbase from more than one spot at a time
      regs.data = inl(dev->bsrbase + regs.port);
      up(&boardlock[dev->board]);
      PDEBUG("READ_REGISTER in:%lx ->%lx\n", (regs.port + dev->bsrbase), regs.data);

      if (copy_to_user((regsingle *) arg, &regs, sizeof(regsingle)))
	{
	  return -EFAULT;
	}
      break;

    case FSCC_WRITE_REGISTER3:
      if (dev->bsrbase == 0)
	{
	  return -ENODEV;
	}

      if (copy_from_user(&regs, (regsingle *) arg, sizeof(regsingle)))
	{
	  return -EFAULT;
	}

      PDEBUG("WRITE_REGISTER out:%x ->%x\n", regs.data, (regs.port + dev->bsrbase));

      down(&boardlock[dev->board]);	//don't touch(read/modify/write) bsrbase from more than one spot at a time
      outl(regs.data, dev->bsrbase + regs.port);
      up(&boardlock[dev->board]);
      break;
    case FSCC_STATUS:
//returns reflected IRQ status messages from the ISR to user space      
//This is the only ioctl that will block.
//If you call this without _O_NONBLOCK you must force a status event
//there is not currently a mechanism in place to cancel this operation

    readstatusagain:
#ifdef USE_2_6
      spin_lock_irqsave(&dev->irqlock, flags);
#else
      save_flags(flags);	//stop interrupts
      cli();
#endif
      status = dev->status;	//get status from driver info area
      dev->status = 0;		//clear it
#ifdef USE_2_6
      spin_unlock_irqrestore(&dev->irqlock, flags);
#else
      restore_flags(flags);	//enable interrupts
#endif
      if (status == 0)
	{
	  if (filp->f_flags & O_NONBLOCK)
	    {
	      //if given as O_NONBLOCK then don't block here
	    }
	  else
	    {
	      //not nonblocking, status is 0, so block until status changes
#ifdef USE_2_6
	      prepare_to_wait(&dev->sq, &wait, TASK_INTERRUPTIBLE);
	      if (status == 0)
		{
		  schedule();
		}
	      finish_wait(&dev->sq, &wait);	//I believe we hang here until dev->sq is signaled
#else
	      save_flags(flags);
	      cli();
	      if (status == 0)
		{
		  interruptible_sleep_on(&dev->sq);
		}
	      restore_flags(flags);
#endif
	      if (signal_pending(current))
		{
		  goto leavestatus;
		}
	      goto readstatusagain;	//get the new status and return it
	    }
	}
    leavestatus:
      if (copy_to_user((unsigned long *) arg, &status, sizeof(unsigned long)))
	return -EFAULT;

      PDEBUG("STATUS:%lx\n", status);

      break;

    case FSCC_TX_ACTIVE:

      PDEBUG("TX_ACTIVE:%u\n", dev->is_transmitting);

      if (dev->is_transmitting == 0)
	{
	  put_user(0, (unsigned long *) arg);
	}
      else
	{
	  put_user(1, (unsigned long *) arg);
	}
      break;

    case FSCC_RX_READY:

      PDEBUG("RX_READY:%u\n", atomic_read(&dev->received_frames_pending));

      put_user(atomic_read(&dev->received_frames_pending), (unsigned long *) arg);
      break;

    case FSCC_FLUSH_RX:

      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}

      if (dev->port_initialized == 0)
	{
	  return -ENODEV;	//could use a better error value, needs to convey that the port has not been initialized yet
	}
      PDEBUG("FLUSH_RX\n");
      dev->rdtotal = 0;
      if (dev->usedma == 1)
	{
	  init_rx_dma(dev);
	  atomic_set(&dev->received_frames_pending, 0);
	  wake_up_interruptible((&dev->rq));
	  //printk("control:%8.8x\n",dev->Rxbdalist[0]->control);
	}
      else
	{
#ifdef USE_2_6
	  spin_lock_irqsave(&dev->irqlock, flags);
#else
	  save_flags(flags);	//stop interrupts
	  cli();
#endif
	  WRITE_COMMAND(dev->fsccbase, RRES);
	  for (i = 0; i < dev->settings.n_rbufs; i++)
	    {
	      dev->fscc_rbuf[i].valid = 0;
	      dev->fscc_rbuf[i].no_bytes = 0;
	      dev->fscc_rbuf[i].max = 0;
	      dev->fscc_rbuf[i].fe = 0;
	      dev->fscc_rbuf[i].ftot = 0;
	    }
	  dev->current_rxbuf = 0;
	  dev->current_rxbuf_out = 0;
	  atomic_set(&dev->received_frames_pending, 0);
#ifdef USE_2_6
	  spin_unlock_irqrestore(&dev->irqlock, flags);
#else
	  restore_flags(flags);	//enable interrupts
#endif
	  wake_up_interruptible((&dev->rq));
	}
      break;

    case FSCC_FLUSH_TX:
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}

      if (dev->port_initialized == 0)
	{
	  return -ENODEV;	//could use a better error value, needs to convey that the port has not been initialized yet
	}
      PDEBUG("FLUSH_TX\n");
      if (dev->usedma == 1)
	{
	  uint32_t tempul;
	  unsigned long timeout;
	  /*      for(i=0;i<dev->settings.n_tbufs;i++)
	     {
	     printk("tdesc[%d].control:%8.8x\n",i,dev->Txbdalist[i]->control);
	     }
	   */
	  tempul = 0;
	  if (dev->channel == 0)
	    {
	      tempul = READ_PORT_ULONG(dev->bsrbase + 0x04);
	      tempul = tempul & 0x03000000;	//mask dma enable bits and retain them
	    }
	  if (dev->channel == 1)
	    {
	      tempul = READ_PORT_ULONG(dev->bsrbase + 0x08);
	      tempul = tempul & 0x03000000;	//mask dma enable bits and retain them
	    }
	  //need to reset the dma controller here...so lets go to fullstop
	  //
	  WRITE_COMMAND(dev->fsccbase, TRES);
	  timeout = 0;
	  while (((READ_STATUS(dev->fsccbase) & CE) == CE) && (timeout < 100000))
	    {
	      timeout++;
	    }
	  if (timeout == 100000)
	    {
	      printk("fsccdrv:uh oh, timeout on CEC after TRES! in flushtx\n");
	    }

	  if ((dev->channel == 0) && ((status & 0x02) == 0x00))
	    {
	      WRITE_PORT_ULONG(dev->bsrbase + 0x04, (tempul & 0x01000000) | 0x00000200);	//stop ta
	    }
	  if ((dev->channel == 1) && ((status & 0x08) == 0x00))
	    {
	      WRITE_PORT_ULONG(dev->bsrbase + 0x08, (tempul & 0x01000000) | 0x00000200);	//stop tb
	    }
	  if (dev->channel == 0)
	    {
	      WRITE_PORT_ULONG(dev->bsrbase + 0x04, (tempul & 0x01000000) | 0x00000020);	//reset ta
	    }
	  if (dev->channel == 1)
	    {
	      WRITE_PORT_ULONG(dev->bsrbase + 0x08, (tempul & 0x01000000) | 0x00000020);	//reset tb
	    }
	  init_tx_desc(dev);
	  if (dev->channel == 0)
	    {
	      WRITE_PORT_ULONG(dev->bsrbase + 0x04, tempul);	//enable dma
	    }
	  if (dev->channel == 1)
	    {
	      WRITE_PORT_ULONG(dev->bsrbase + 0x08, tempul);	//enable dma
	    }

	  dev->current_txbuf = 0;
	  dev->is_transmitting = 0;
	  dev->transmit_delay = 0;
	  atomic_set(&dev->transmit_frames_available, dev->settings.n_tbufs);
	  wake_up_interruptible(&dev->wq);
	}
      else
	{
	  unsigned long timeout;

#ifdef USE_2_6
	  spin_lock_irqsave(&dev->irqlock, flags);
#else
	  save_flags(flags);	//stop interrupts
	  cli();
#endif
	  for (i = 0; i < dev->settings.n_tbufs; i++)
	    {
	      dev->fscc_tbuf[i].valid = 0;
	      dev->fscc_tbuf[i].no_bytes = 0;
	      dev->fscc_tbuf[i].max = 0;
	      dev->fscc_tbuf[i].fe = 0;
	      dev->fscc_tbuf[i].ftot = 0;
	    }
	  dev->current_txbuf = 0;
	  dev->is_transmitting = 0;
	  atomic_set(&dev->transmit_frames_available, dev->settings.n_tbufs);
	  WRITE_COMMAND(dev->fsccbase, TRES);
	  timeout = 0;
	  while (((READ_STATUS(dev->fsccbase) & CE) == CE) && (timeout < 100000))
	    {
	      timeout++;
	    }
	  if (timeout == 100000)
	    {
	      printk("fsccdrv:uh oh, timeout on CEC after TRES! in flushtx\n");
	    }

#ifdef USE_2_6
	  spin_unlock_irqrestore(&dev->irqlock, flags);
#else
	  restore_flags(flags);	//enable interrupts
#endif
	  wake_up_interruptible(&dev->wq);
	}
      break;

    case FSCC_IMMEDIATE_STATUS:
//this one allways returns immediatly with 0 
//if there are no status events
//(the same as calling _STATUS opened as _O_NONBLOCK)
#ifdef USE_2_6
      spin_lock_irqsave(&dev->irqlock, flags);
#else
      save_flags(flags);	//stop interrupts
      cli();
#endif
      status = dev->status;
      dev->status = 0;
#ifdef USE_2_6
      spin_unlock_irqrestore(&dev->irqlock, flags);
#else
      restore_flags(flags);	//enable interrupts
#endif
      if (copy_to_user((unsigned long *) arg, &status, sizeof(unsigned long)))
	{
	  return -EFAULT;
	}

      PDEBUG("IMMEDIATE_STATUS:%lx\n", status);

      break;

    case FSCC_SET_FREQ:
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}

      if (copy_from_user(&clk, (struct clockstruct *) arg, sizeof(struct clockstruct)))
	{
	  return -EFAULT;
	}
      PDEBUG("SETFREQ:freq:%ld \n", clk.freq);

      //special casing for FSCC(board rev2.2) vs SuperFSCC(board rev 3.1)
      //get VSTR
      ver = READ_VERSION(dev->fsccbase);
      if( (ver&0xffff0000) == 0x000f0000 )	//FSCC Board rev <=2.2, e.g. Green board
         {
            if( ((ver>>8)&0x000000ff) <=6)	//If the firmware major number is 6 or less
            {
	      //firmware rev less than 6 indicates force CLK
	      clk.progword[15] = (clk.progword[15] & 0xfb);
	      //note if VSTR reads back all 0's (original firmware) then
	      //we end up here, which should be the correct path.
               
            }
         }
      else 
      {
         //This should be all other blue boards...PCI, 104 and cPCI should all use XTAL
         clk.progword[15] = (clk.progword[15] & 0xfb) | 0x04;
      }
/*
      if ((ver & 0xffff0000) == 0x00140000)
	{
	  if (((ver >> 8) & 0x00000ff) >= 2)
	    {
	      //override CLK/XTAL bit and force XTAL          
	      clk.progword[15] = (clk.progword[15] & 0xfb) | 0x04;
	    }
	  else
	    {
	      //prototypes need force CLK
	      clk.progword[15] = (clk.progword[15] & 0xfb);
	    }
	}
      else
	{
	  if (((ver >> 8) & 0x000000ff) <= 6)
	    {
	      //firmware rev less than 6 indicates force CLK
	      clk.progword[15] = (clk.progword[15] & 0xfb);
	      //note if VSTR reads back all 0's (original firmware) then
	      //we end up here, which should be the correct path.
	    }
	  else
	    {
	      //firmware rev greater than 6 should be "fixed" and require force XTAL
	      clk.progword[15] = (clk.progword[15] & 0xfb) | 0x04;
	    }
	}
*/
//printk("ver:%x, clk15:%x\n",ver,clk.progword[15]);
      down(&boardlock[dev->board]);	//it doesn't matter which channel sets the clock, but
      //if both try to do it at the same time bad things will
      //happen.
      dev->freq = clk.freq;

      if (dev->channel == 0)
	{
	  SetICS30703Bits(dev->bsrbase, clk.progword);
	}
      else
	{
	  SetICS30703Bits2(dev->bsrbase, clk.progword);
	}

      up(&boardlock[dev->board]);
      break;
    case FSCC_GET_FREQ:
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}
      if (copy_to_user((unsigned long *) arg, &dev->freq, sizeof(unsigned long)))
	{
	  return -EFAULT;
	}
      break;
    case FSCC_GET_FEATURES:
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}
      tempul = inl(dev->bsrbase);
      features = 0;
      if (dev->channel == 0)
	{
	  if ((tempul & 0x00010000) != 0)
	    {
	      features |= 1;
	    }
	  if ((tempul & 0x00020000) != 0)
	    {
	      features |= 4;
	    }
	  if ((tempul & 0x00040000) != 0)
	    {
	      features |= 2;
	    }
	  if ((tempul & 0x00080000) != 0)
	    {
	      features |= 8;
	    }
	  if ((tempul & 0x01000000) != 0)
	    {
	      features |= 0x80000000;
	    }

	}
      else
	{
	  if ((tempul & 0x00100000) != 0)
	    {
	      features |= 1;
	    }
	  if ((tempul & 0x00200000) != 0)
	    {
	      features |= 4;
	    }
	  if ((tempul & 0x00400000) != 0)
	    {
	      features |= 2;
	    }
	  if ((tempul & 0x00800000) != 0)
	    {
	      features |= 8;
	    }
	  if ((tempul & 0x02000000) != 0)
	    {
	      features |= 0x80000000;
	    }
	}

      if (copy_to_user((unsigned long *) arg, &features, sizeof(unsigned long)))
	{
	  return -EFAULT;
	}
      break;
    case FSCC_SET_FEATURES:
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}
      if (copy_from_user(&dev->features, (unsigned long *) arg, sizeof(unsigned long)))
	{
	  return -EFAULT;
	}
      down(&boardlock[dev->board]);	//don't touch(read/modify/write) bsrbase from more than one spot at a time
      tempul = inl(dev->bsrbase);
      //      Feature register (32 bit register at BAR2) consists of:
      //re-write for FSCC...
      //      bit 16: CHA receive echo cancel if '0' RTS controls RD, if '1' RD allways on
      //  bit 17: CHA TT 485 control      if '0'/RTS controls TT, if '1' SD allways on
      //  bit 18: CHA SD 485 control      if '0'/RTS controls SD, if '1' SD allways on
      //  bit 19: CHA DTR/FST control     if '0'DTR/FST is FST, if '1' DTR/FST is DTR
      //      bit 20: CHB receive echo cancel if '0' RTS controls RD, if '1' RD allways on
      //  bit 21: CHB TT 485 control      if '0'/RTS controls TT, if '1' SD allways on
      //  bit 22: CHB SD 485 control      if '0'/RTS controls SD, if '1' SD allways on
      //  bit 23: CHB DTR/FST control     if '0'DTR/FST is FST, if '1' DTR/FST is DTR

      if (dev->channel == 0)
	{
	  //do channel 0 bits
	  //assumed incomming data of format:
	  //bit0 = rxechodisable
	  //bit1 = sd485
	  //bit2 = tt485
	  if ((tempul & 0x01000000) == 0x00000000)
	    {
	      //only change feature bits if sync port is active (don't wack async port settings!)
	      tempul = tempul & 0xfef0ffff;
	      if ((dev->features & 1) == 1)
		{
		  tempul |= 0x00010000;
		}
	      if ((dev->features & 2) == 2)
		{
		  tempul |= 0x00040000;
		}
	      if ((dev->features & 4) == 4)
		{
		  tempul |= 0x00020000;
		}
	      if ((dev->features & 8) == 8)
		{
		  tempul |= 0x00080000;
		}
	      if ((dev->features & 0x80000000) == 0x80000000)
		{
		  tempul |= 0x01000000;
		}
	    }
	  else
	    {
	      //have to allow change between sync/async somewhere...
	      tempul = tempul & 0xfeffffff;
	      if ((dev->features & 0x80000000) == 0x80000000)
		{
		  tempul |= 0x01000000;
		}
	    }
	}
      else
	{
	  //do channel 1 bits
	  if ((tempul & 0x02000000) == 0x00000000)
	    {
	      //only change feature bits if sync port is active (don't wack async port settings!)
	      tempul = tempul & 0xfd0fffff;
	      if ((dev->features & 1) == 1)
		{
		  tempul |= 0x00100000;
		}
	      if ((dev->features & 2) == 2)
		{
		  tempul |= 0x00400000;
		}
	      if ((dev->features & 4) == 4)
		{
		  tempul |= 0x00200000;
		}
	      if ((dev->features & 8) == 8)
		{
		  tempul |= 0x00800000;
		}
	      if ((dev->features & 0x80000000) == 0x80000000)
		{
		  tempul |= 0x02000000;
		}
	    }
	  else
	    {
	      //have to allow change between sync/async somewhere...
	      tempul = tempul & 0xfdffffff;
	      if ((dev->features & 0x80000000) == 0x80000000)
		{
		  tempul |= 0x02000000;
		}
	    }
	}
      WRITE_PORT_ULONG(dev->bsrbase, tempul);
      up(&boardlock[dev->board]);
      break;

    case FSCC_TIMED_TRANSMIT:
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}
      if (copy_from_user(&dev->timed_transmit, (unsigned long *) arg, sizeof(unsigned long)))
	{
	  return -EFAULT;
	}
      break;

case FSCC_EXTERNAL_TRANSMIT:
      if (dev->fsccbase == 0)
        {
          return -ENODEV;
        }
      if (copy_from_user(&dev->external_transmit, (unsigned long *) arg, sizeof(unsigned long)))        {
          return -EFAULT;
        }
      break;
    case FSCC_TRANSMIT_REPEAT:
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}
      if (copy_from_user(&dev->transmit_repeat, (unsigned long *) arg, sizeof(unsigned long)))
	{
	  return -EFAULT;
	}
      break;
    case FSCC_TRANSMIT_DELAY:
      //printk("transmit delay ioctl\n");
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}
      if (copy_from_user(&dev->transmit_delay, (unsigned long *) arg, sizeof(unsigned long)))
	{
	  return -EFAULT;
	}
      printk("fsccdrv:transmit delay:%ld\n", dev->transmit_delay);
      break;
    case FSCC_TRANSMIT_IRQ_RATE:
      //printk("transmit irq rate ioctl\n");
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}
      if (copy_from_user(&dev->tirqrate, (unsigned long *) arg, sizeof(unsigned long)))
	{
	  return -EFAULT;
	}
      printk("fsccdrv:transmit irqrate:%ld\n", dev->tirqrate);
      break;
    case FSCC_RECEIVE_IRQ_RATE:
      //printk("receive irq rate ioctl\n");
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}
      if (copy_from_user(&dev->rirqrate, (unsigned long *) arg, sizeof(unsigned long)))
	{
	  return -EFAULT;
	}
      printk("fsccdrv:receive irqrate:%ld\n", dev->rirqrate);
      break;

    case FSCC_DUMP_RDESC:
      for (i = 0; i < dev->settings.n_rbufs; i++)
	{
	  printk("[%d]@      :%8.8x\n", i, (uint32_t) dev->phys_rxbdalist[i]);
	  printk("[%d]control:%8.8x\n", i, dev->Rxbdalist[i]->control);
	  printk("[%d]data ad:%8.8x\n", i, dev->Rxbdalist[i]->data_address);
	  printk("[%d]count  :%8.8x\n", i, dev->Rxbdalist[i]->data_count);
	  printk("[%d]next   :%8.8x\n", i, dev->Rxbdalist[i]->next_desc);
	  for (j = 0; j < 16; j++)
	    {
	      printk("%x:", dev->rxdata[i][j]);
	    }
	  printk("\n");
	  printk("============\n");
	}
      break;
    case FSCC_DUMP_TDESC:
      for (i = 0; i < dev->settings.n_tbufs; i++)
	{
	  printk("[%d]@      :%8.8x\n", i, (uint32_t) dev->phys_txbdalist[i]);
	  printk("[%d]control:%8.8x\n", i, dev->Txbdalist[i]->control);
	  printk("[%d]data ad:%8.8x\n", i, dev->Txbdalist[i]->data_address);
	  printk("[%d]count  :%8.8x\n", i, dev->Txbdalist[i]->data_count);
	  printk("[%d]next   :%8.8x\n", i, dev->Txbdalist[i]->next_desc);
	  for (j = 0; j < 16; j++)
	    {
	      printk("%x:", dev->txdata[i][j]);
	    }
	  printk("\n");
	  printk("============\n");
	}

      break;
    case FSCC_ALLOW_READ_CUTOFF:
      if (dev->fsccbase == 0)
	{
	  return -ENODEV;
	}
      if (copy_from_user(&dev->allowcutoff, (unsigned long *) arg, sizeof(unsigned long)))
	{
	  return -EFAULT;
	}
      break;

    default:			/* redundant, as cmd was checked against MAXNR */
      return -EINVAL;
    }
  return 0;

}
