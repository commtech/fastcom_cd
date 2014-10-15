/******************************************************
 *
 * Copyright (C) 2000 Commtech, Inc. Wichita KS
 *
 * isr.c -- Interrupt service routine for fscc-pci module
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
#include <linux/ioport.h>
#include <linux/sched.h>
#include <asm/system.h>		/* cli(), *_flags */
#include <asm/segment.h>	/* memcpy and such */
#include <asm/io.h>
#include "fsccdrv.h"		/* local definitions */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
irqreturn_t
fscc_irq(int irq, void *dev_id)
#else
irqreturn_t
fscc_irq(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
  Fscc_Dev *dev;
  //unsigned i;
  unsigned port;
  unsigned isrprt;
  uint32_t isr_value;
  unsigned servicedanint;
  unsigned irqhit;
  unsigned long flags;
//printk("I");
  servicedanint = 0;
topofisr:
//printk("K");
  irqhit = 0;

  for (isrprt = 0; isrprt < fscc_nr_devs; isrprt++)
    {
      dev = &fscc_devices[isrprt];
#ifdef USE_2_6
      spin_lock_irqsave(&dev->irqlock, flags);
#endif
    restart_isr:
      port = dev->fsccbase;
      PDEBUG("irq\n");
      if (dev->port_initialized == 1)
	{

	  isr_value = READ_INTERRUPT_STATUS(port);
	  if (isr_value == 0)
	    {
	    }
	  else
	    {
//   printk("\nisrval:%8.8lx\n",isr_value);
	      servicedanint = 1;
	      irqhit = 1;
	      //if we got here then status is != 0
	      //here we start doing the isr's
	      //these are reordered from their bit positions to give priority to the 
	      //fifo handling, the error handling, and then the state change indicators
	      if ((isr_value & RFE) == RFE)
		{
		  dev->status = dev->status | ST_RFE;
		  if (dev->usedma == 1)
		    {
		    }
		  else
		    {
		      empty_rx_fifo(dev);
		    }
		}

	      if ((isr_value & RFT) == RFT)
		{
		  dev->status = dev->status | ST_RFT;
		  //receive fifo trigger
//                      printk("RFT\n");
		  if (dev->usedma == 1)
		    {
		    }
		  else
		    {
		      empty_rx_fifo(dev);
		    }
		}
	      if ((isr_value & TFT) == TFT)
		{
		  //              unsigned long szorig;
		  //transmit fifo trigger
		  //printk("TFT\n");
		  dev->status = dev->status | ST_TFT;
		  if (dev->usedma == 1)
		    {
		    }
		  else
		    {
		      fill_tx_fifo(dev);
		    }
		}
	      if ((isr_value & ALLS) == ALLS)
		{
		  //all sent interrupt
		  //transmitter is now IDLE
		  dev->status = dev->status | ST_ALLS;

		  if (dev->usedma == 1)
		    {
		    }
		  else
		    {
//check to see if there are more frames to push, and if so...push them
		      if (fill_tx_fifo(dev))
			{
			}
		      else
			{
			  dev->is_transmitting = 0;
			  dev->bits_going_out = 0;
			}

		    }
		  //              printk("ALLS!\n");
		}
	      if ((isr_value & TDU) == TDU)
		{
		  //transmit data underrun
		  dev->status = dev->status | ST_TDU;
		}
	      if ((isr_value & RFO) == RFO)
		{
		  //receive frame overflow
		  dev->status = dev->status | ST_RFO;
		}
	      if ((isr_value & RDO) == RDO)
		{
		  //receive data overflow
		  dev->status = dev->status | ST_RDO;
		}
	      if ((isr_value & RFL) == RFL)
		{
		  //receive frame lost
		  dev->status = dev->status | ST_RFL;
		}
	      if ((isr_value & RFS) == RFS)
		{
		  //receive frame start, 
		  dev->status = dev->status | ST_RFS;
		}
	      if ((isr_value & TIN) == TIN)
		{
		  //timer interrupt
		  dev->status = dev->status | ST_TIN;
		}
	      if ((isr_value & CTSS) == CTSS)
		{
		  //change in CTS state
		  dev->status = dev->status | ST_CTSS;
		}
	      if ((isr_value & DSRC) == DSRC)
		{
		  //change in DSR state
		  dev->status = dev->status | ST_DSRC;
		}
	      if ((isr_value & CDC) == CDC)
		{
		  //change in CD state
		  dev->status = dev->status | ST_CDC;
		}
	      if ((isr_value & CTSA) == CTSA)
		{
		  //CTS triggered abort
		  dev->status = dev->status | ST_CTSA;
		}
	      if ((isr_value & 0x00008000) == 0x00008000)
		{
		  //DMA TSTOP
		  //printk("DMA STOP_T\n");
		  dev->status = dev->status | ST_DMAT_S;
		}
	      if ((isr_value & 0x00004000) == 0x00004000)
		{
		  //DMA RSTOP
		  //printk("DMA STOP_R\n");
		  dev->status = dev->status | ST_DMAR_S;
		}
	      if ((isr_value & 0x00002000) == 0x00002000)
		{
		  //DMA TFE
		  //printk("DMA TFE\n");
		  dev->status |= ST_TX_DONE;
		  atomic_inc(&dev->transmit_frames_available);
		}
	      if ((isr_value & 0x00001000) == 0x00001000)
		{
		  //DMA RFE
		  //printk("DMA_RFE\n");
		  dev->status |= ST_RX_DONE;
		}
	      if ((isr_value & 0x00000800) == 0x00000800)
		{
		  //DMA T-HI
		  //printk("DMA_THI\n");
		  dev->status |= ST_TX_HI;
		}
	      if ((isr_value & 0x00000400) == 0x00000400)
		{
		  //DMA R-HI
		  //printk("DMA_RHI\n");
		  dev->status |= ST_RX_HI;
		}
	      if (0)
		{
		  printk("FSCCDRV.SYS ISR-ERROR, nonzero and no valid bits set!\n");
		}
	      if (((dev->status & ST_RX_HI) != 0) || ((dev->status & ST_RX_DONE) != 0)
		  || (atomic_read(&dev->received_frames_pending) > 0))
		{
		  wake_up_interruptible(&dev->rq);
		}
	      if (((dev->status & ST_TX_HI) != 0) || (dev->status & ST_TX_DONE) != 0)
		{
		  wake_up_interruptible(&dev->wq);
		}
//wake_up_interruptible(&dev->wq);

	      if (dev->status != 0)
		{
		  wake_up_interruptible(&dev->sq);
		}

	      goto restart_isr;

	    }

	}			//end of if initialized
      else
	{
	  //if port is not initialized then it would be a bad idea to let the isr
	  //have at it, as there are no buffers allocated yet.
	  //printk("in isr shutting down interrupts\n");
	  WRITE_INTERRUPT_MASK(port, 0xffffffff);
	  servicedanint = 1;
	}
#ifdef USE_2_6
      spin_unlock_irqrestore(&dev->irqlock, flags);
#endif
    }

  if (irqhit == 1)
    {
      goto topofisr;		//keep doing the loop until there are no more interrupts to service
    }
#ifdef USE_2_6
  if (servicedanint == 0)
    {
      return (IRQ_NONE);
    }
  else
    {
      return (IRQ_HANDLED);
    }
#endif

}

void
check_rq_timeout(unsigned long ptr)
{
  Fscc_Dev *dev;
  dev = &fscc_devices[ptr];
  dev->rblockcnt++;
//printk("check_rq_timeout fired!\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
//if(dev->rq_not_empty==1)
//      {
//      dev->rq_not_empty=0;
  wake_up_interruptible(&dev->rq);
//      }
#else
  if (dev->rqt.function != NULL)
    {
//              dev->rq_not_empty=0;
      wake_up_interruptible(&dev->rq);
    }
#endif
}
void
check_wq_timeout(unsigned long ptr)
{
  Fscc_Dev *dev;
  dev = &fscc_devices[ptr];


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
//if(dev->wq_not_empty==1)
//      {
//      dev->wq_not_empty=0;
  wake_up_interruptible(&dev->wq);
//      }
#else
  if (dev->wqt.function != NULL)
    {
//              dev->wq_not_empty=0;
      wake_up_interruptible(&dev->wq);
    }
#endif
}
