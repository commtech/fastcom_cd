/******************************************************
 *
 * Copyright (C) 2000 Commtech, Inc. Wichita KS
 *
 * fsccdrv.c -- init stuff for fscc-pci module
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
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/module.h>
#include <linux/version.h>
MODULE_LICENSE("GPL");

#include <linux/kernel.h>	/* printk() */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#include <linux/slab.h>
#else
#include <linux/malloc.h>	/* kmalloc() */
#endif
#include <linux/vmalloc.h>
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/ioport.h>
#include <asm/system.h>		/* cli(), *_flags */
#include <asm/segment.h>	/* memcpy and such */
#include <asm/atomic.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
#include <linux/semaphore.h>
#else
#include <asm/semaphore.h>
#endif
#include <linux/string.h>

#include <linux/pci.h>		//for PCI stuff

#include "fsccdrv.h"		/* local definitions */

int fscc_major = FSCC_MAJOR;
int fscc_nr_devs = MAX_FSCC_BOARDS * 2;	/* number of fscc devices */
Fscc_Dev *fscc_devices;		/* allocated in init_module */
struct semaphore boardlock[MAX_FSCC_BOARDS];	//keeps accesses to both channels of a single board in line
int used_board_numbers[MAX_FSCC_BOARDS];	//used to determine which devices are part of the same board
int boardtype[MAX_FSCC_BOARDS];
int fscc_read_procmem(char *buf, char **start, off_t offset, int len, int *unused, void *data);

//if you need different ops on different minors, here is a good place to put them...
//we only have one set currently
struct file_operations *fscc_fop_array[] = {
#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)
  NULL,				//struct module *owner
#endif
  &fscc_fops			/* type 0 */
};

//MODULE_AUTHOR("Carl");
//MODULE_DESCRIPTION("Character Driver for the FSCC device family");
/*
 * The different file operations
 */
struct file_operations fscc_fops = {
  .owner = THIS_MODULE,
  .read = fscc_read,
  .write = fscc_write,
  .poll = fscc_poll,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 11))
  .unlocked_ioctl = fscc_ioctl,
#else
  .ioctl = fscc_ioctl,
#endif
  .open = fscc_open,
  .release = fscc_release,
  .fsync = fscc_fsync
};

struct file_operations proc_fops = {
#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)
  NULL,				//struct module *owner
#endif
  NULL,				//llseek
  NULL,				//fscc_read_procmem,     //read
  NULL,				//write
  NULL,				/* readdir */
  NULL,				/* select now poll */
  NULL,				//ioctl --the workhorse of all this
  NULL,				/*mmap */
  NULL,				//open
  NULL,				//flush
  NULL,				//release (close)
  NULL,				//fsync
  NULL,				//fasync
  NULL,				//check media change
  NULL,				//revalidate
  NULL				//lock
};

#ifdef FSCC_USE_PROC		/* don't waste space if unused */
/*
 * The proc filesystem: function to read an entry
 */

int
fscc_read_procmem(char *buf, char **start, off_t offset, int len, int *unused, void *data)
{
  int i;
  Fscc_Dev *d;
  PDEBUG("fscc_read_procmem entered");
#define LIMIT (PAGE_SIZE-80)	/* don't print any more after this size */
//note it isn't enforced....??
  len = 0;
  for (i = 0; i < fscc_nr_devs; i++)
    {
      //fill in usefull info here, displayed in /proc/fsccdrv via 'cat /proc/fsccdev'
      d = &fscc_devices[i];
      if (len < PAGE_SIZE - 512)
	{
	  len +=
	    sprintf(buf + len,
		    "Dev %i: Base:%x irq:%i ch:%i dma:%lu rb:%u rz:%u tb:%u tz:%u -- oc:%lu #t:%u\n",
		    i, d->fsccbase, d->irq, d->channel, d->usedma,
		    d->settings.n_rbufs, d->settings.n_rfsize_max,
		    d->settings.n_tbufs, d->settings.n_tfsize_max,
		    d->fscc_u_count, atomic_read(&d->transmit_frames_available));
	}
    }
  return len;
}

#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)
static struct proc_dir_entry *proc_fscc_entry;
#else
struct proc_dir_entry fscc_proc_entry = {
	0,				/* low_ino: the inode -- dynamic */
	8,				/* Name Length */
	"fsccpdrv",			/* Entry Name */
	S_IFREG | S_IRUGO,		/* mode_t Mode */
	1,				/* nlink_t nlink */
	0,				/* uid_t uid */
	0,				/* gid_t gid */
	20,				/* ulong Size */
	NULL,				/* struct inode_operations *PTR */
#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)
	&fscc_fops,			/* struct file_operations *PTR */
#endif
	NULL,				/* get_info_t * */
#if  LINUX_VERSION_CODE <= VERSION_CODE(2,4,0)
	NULL,
endif
#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)	/* Check Version Number..2.4.0 is not precise */
	NULL,				/* struct module *owner */
#endif
	NULL,				/* struct proc_dir_entry *next */
	NULL,				/* struct proc_dir_entry *parent */
	NULL,				/* struct proc_dir_entry *subdir */
	NULL,				/* void *data - Pointer to data */
	&fscc_read_procmem,		/* read_proc_t *read_proc */
	NULL,				/* write_proc_t *write_proc */
	/* atomic_t count - Counter */
	/* int delete - Delete Flags */
	/* kdev_t rdev */
};
#endif

#endif

#endif
/*
 * Finally, the module stuff
 */
#ifdef USE_2_6
static int __init
fscc_init(void)
#else
int
init_module(void)
#endif
{
  int result, i;
  int index;
  unsigned long fsccbase[MAX_FSCC_BOARDS];	//holds all fscc base addresses
  unsigned long uartbase[MAX_FSCC_BOARDS];	//holds all 16550 base addresses
  unsigned long bsrbase[MAX_FSCC_BOARDS];	//features registers
  unsigned irq[MAX_FSCC_BOARDS];	//holds all needed IRQ's
  struct pci_dev *pcidev[MAX_FSCC_BOARDS];
  struct pci_dev *pdev = NULL;	//to get base addresses from pci subsystem
  Fscc_Dev *dev;		//needed everywhere
  int err = 0;

  for (i = 0; i < MAX_FSCC_BOARDS; i++)
    {
//start blank
      uartbase[i] = 0;
      fsccbase[i] = 0;
      bsrbase[i] = 0;
      irq[i] = 0;
    }
  index = 0;
#ifdef CONFIG_PCI
#ifdef USE_2_6
  if (1)
#else
  if (pci_present())
#endif
    {
#ifdef USE_2_6
      while ((pdev = pci_get_device(COMMTECH_VENDOR_ID, PCI_ANY_ID, pdev)) != NULL)
#else
      while ((pdev = pci_find_device(COMMTECH_VENDOR_ID, PCI_ANY_ID, pdev)) != NULL)
#endif
	{
           if( (pdev->device == FSCC_DEVID) ||
               (pdev->device == SFSCC_DEVID) ||
               (pdev->device == FSCC232_DEVID) ||
               (pdev->device == SFSCC104_NOUA_DEVID) ||
               (pdev->device == SFSCC_4_DEVID) ||
               (pdev->device == SFSCC104_DEVID) ||
               (pdev->device == SFSCCLVDS_DEVID) ||
               (pdev->device == FSCC_4_DEVID) ||
               (pdev->device == SFSCC_4_LVDS_DEVID) ||
               (pdev->device == SFSCCe_4_DEVID) )
           {
//ok here we have a board at bus/function, so get its stats
	      if (pci_enable_device(pdev))
	      {
	         printk("fsccdrv:unable to enable device!\n");
	      }
	      else
	      {
	         pci_set_master(pdev);
	         irq[index] = pdev->irq;
#if  LINUX_VERSION_CODE >= VERSION_CODE(2,3,13)
	         fsccbase[index] = pci_resource_start(pdev, 0) & PCI_BASE_ADDRESS_IO_MASK;
	         uartbase[index] = pci_resource_start(pdev, 1) & PCI_BASE_ADDRESS_IO_MASK;
	         bsrbase[index] = pci_resource_start(pdev, 2) & PCI_BASE_ADDRESS_IO_MASK;
	         pcidev[index] = pdev;
#ifdef USE_2_6
	         pci_dev_get(pdev);
#endif
	         printk(KERN_DEBUG
		     "\nPCI BASE: fscc: %lx uart: %lx bsr:%lx irq: %d\n",
		     fsccbase[index], uartbase[index], bsrbase[index], irq[index]);

#else
	         fsccbase[index] = pdev->base_address[0] & PCI_BASE_ADDRESS_IO_MASK;
	         uartbase[index] = pdev->base_address[1] & PCI_BASE_ADDRESS_IO_MASK;
	         bsrbase[index] = pdev->base_address[2] & PCI_BASE_ADDRESS_IO_MASK;
#endif
	         PDEBUG("fscc:%lx, uart:%lx, bsr:%lx, irq:%u\n",
		     fsccbase[index], uartbase[index], bsrbase[index], irq[index]);

                 if( (pdev->device == FSCC_DEVID) || 
                     (pdev->device == FSCC232_DEVID) || 
                     (pdev->device ==FSCC_4_DEVID) )
	            boardtype[index] = 0;	//no dma
                 else if( (pdev->device == SFSCC_DEVID) || 
                          (pdev->device == SFSCC104_NOUA_DEVID) ||
                          (pdev->device == SFSCC_4_DEVID) || 
                          (pdev->device == SFSCC104_DEVID) ||
                          (pdev->device == SFSCCLVDS_DEVID) ||
                          (pdev->device == SFSCC_4_LVDS_DEVID) ||
                          (pdev->device == SFSCCe_4_DEVID) )
                    boardtype[index] = 1;	//dma (i.e. Super)
	         index++;
	      }
           }
	   else
           {
              pdev=NULL;
	   }
         }
/*
#ifdef USE_2_6
      while ((pdev = pci_get_device(FSCCP_VENDOR_ID, SFSCCP_DEVICE_ID, pdev)))
#else
      while ((pdev = pci_find_device(FSCCP_VENDOR_ID, SFSCCP_DEVICE_ID, pdev)))
#endif
	{
	  //ok here we have a board at bus/function, so get its stats

	  if (pci_enable_device(pdev))
	    {
	      printk("fsccdrv:unable to enable device!\n");
	    }
	  else
	    {
	      pci_set_master(pdev);
	      irq[index] = pdev->irq;
#if  LINUX_VERSION_CODE >= VERSION_CODE(2,3,13)
	      fsccbase[index] = pci_resource_start(pdev, 0) & PCI_BASE_ADDRESS_IO_MASK;
	      uartbase[index] = pci_resource_start(pdev, 1) & PCI_BASE_ADDRESS_IO_MASK;
	      bsrbase[index] = pci_resource_start(pdev, 2) & PCI_BASE_ADDRESS_IO_MASK;
	      pcidev[index] = pdev;
#ifdef USE_2_6
	      pci_dev_get(pdev);
#endif
	      printk(KERN_DEBUG
		     "\nPCI BASE: fscc: %lx uart: %lx bsr:%lx irq: %d\n",
		     fsccbase[index], uartbase[index], bsrbase[index], irq[index]);

#else
	      fsccbase[index] = pdev->base_address[0] & PCI_BASE_ADDRESS_IO_MASK;
	      uartbase[index] = pdev->base_address[1] & PCI_BASE_ADDRESS_IO_MASK;
	      bsrbase[index] = pdev->base_address[2] & PCI_BASE_ADDRESS_IO_MASK;
#endif
	      PDEBUG("fscc:%lx, uart:%lx, bsr:%lx, irq:%u\n",
		     fsccbase[index], uartbase[index], bsrbase[index], irq[index]);
	      boardtype[index] = 1;
	      index++;
	    }
	}
*/
    }
  else
    {
      return -ENODEV;
    }
  if (index == 0)
    {
      return -ENODEV;
    }
#else
  return -ENODEV;
#endif
//if we get here then we have index # of fsccp boards, and their particulars are
//stored in fsccbase[] uartbase[] bsrbase[] and irq[]    
  /*
   * Register your major, and accept a dynamic number
   */
  fscc_nr_devs = index * 2;
  result = register_chrdev(fscc_major, "fsccp", &fscc_fops);
  if (result < 0)
    {
      printk(KERN_WARNING "fsccp: can't get major %d\n", fscc_major);
      return result;
    }
  if (fscc_major == 0)
    {
      fscc_major = result;	/* dynamic */
    }

  /* 
   * allocate the devices  
   */
  fscc_devices = kmalloc(fscc_nr_devs * sizeof(Fscc_Dev), GFP_KERNEL);
  if (!fscc_devices)
    {
      result = -ENOMEM;
      goto fail_malloc;
    }
  memset(fscc_devices, 0, fscc_nr_devs * sizeof(Fscc_Dev));
  for (i = 0; i < fscc_nr_devs; i++)
    {
      fscc_devices[i].is_transmitting = 0;
#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)
      init_waitqueue_head(&fscc_devices[i].rq);
      init_waitqueue_head(&fscc_devices[i].wq);
      init_waitqueue_head(&fscc_devices[i].sq);
      init_waitqueue_head(&fscc_devices[i].fscc_fsync_wait);
#endif

#ifdef USE_2_6

#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,39)
      fscc_devices[i].irqlock = __SPIN_LOCK_UNLOCKED(Fscc_Dev.irqlock);
#else
      fscc_devices[i].irqlock = SPIN_LOCK_UNLOCKED;
#endif

#endif

      //set per instance data here
    }
  for (i = 0; i < MAX_FSCC_BOARDS; i++)
    {

      sema_init(&boardlock[i], 1);
      used_board_numbers[i] = 0;
    }
  /* At this point call the init function for any friend device */
  /* ... */
#ifdef USE_2_6
#else
  EXPORT_NO_SYMBOLS;
#endif

//here we setup each board/channel 
  for (i = 0; i < index; i++)	//do for all boards that we found in PCI space
    {
//add channel 0
      dev = &fscc_devices[i * 2];
      dev->pcidev = pcidev[i];
      dev->fsccbase = fsccbase[i];
      dev->uartbase = uartbase[i];
      dev->bsrbase = bsrbase[i];
      dev->channel = 0;
      dev->irq = irq[i];
      dev->rqtimeout = 500;
      dev->wqtimeout = 500;
      dev->rirqrate = 1;
      dev->tirqrate = 1;
      dev->rdtotal = 0;
      dev->allowcutoff = 0;
      dev->transmit_delay = 0;
      dev->rblockcnt = 0;
      dev->irq_hooked = 0;

#ifdef USE_DMA
      if (boardtype[i] == 1)
	{
	  enable_dma(dev);
	}
      else
	{
	  dev->usedma = 0;
	}
#else
      dev->usedma = 0;
#endif
      if (fsccbase[i] != 0)	//don't bother if base is 0 it means no device present
	{
	  if ((err = add_port(dev)) < 0)
	    {
	      return err;
	    }
	  dev->board = i;
	  dev->complement_dev = &fscc_devices[(i * 2) + 1];
	}
//add channel 1

      dev = &fscc_devices[(i * 2) + 1];
      dev->pcidev = pcidev[i];
      dev->fsccbase = fsccbase[i] + 0x80;
      dev->uartbase = uartbase[i] + 0x08;
      dev->bsrbase = bsrbase[i];
      dev->channel = 1;
      dev->irq = irq[i];
      dev->rqtimeout = 500;
      dev->wqtimeout = 500;
      dev->rirqrate = 1;
      dev->tirqrate = 1;
      dev->rdtotal = 0;
      dev->allowcutoff = 0;
      dev->transmit_delay = 0;
      dev->rblockcnt = 0;
      dev->irq_hooked = 0;
#ifdef USE_DMA
      if (boardtype[i] == 1)
	{
	  enable_dma(dev);
	}
      else
	{
	  dev->usedma = 0;
	}
#else
      dev->usedma = 0;
#endif

      if (fsccbase[i] != 0)	//don't bother if base is 0 it means no device present
	{
	  if ((err = add_port(dev)) < 0)
	    {
	      return err;
	    }
	  dev->board = i;
	  dev->complement_dev = &fscc_devices[(i * 2)];
	}
//here we look to see if the device needs an IRQ attached,
//if the base address is nonzero it is assumed that the device is "real" and
//we need to attach to the IRQ as long as our irq_hooked flag is 0.
      dev = &fscc_devices[i * 2];
      if ((dev->fsccbase != 0) && (dev->irq != 0) && (dev->irq_hooked == 0))
	{
	  PDEBUG("requesting IRQ in init\n");
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,20)
	  if ((err = request_irq(dev->irq, fscc_irq, IRQF_DISABLED | IRQF_SHARED, "fsccp", fscc_devices)) < 0)
#else
	  if ((err = request_irq(dev->irq, fscc_irq, SA_INTERRUPT | SA_SHIRQ, "fsccp", fscc_devices)) < 0)
#endif
	    {
	      PDEBUG("request_irq failed (in open):%i", err);
	      return err;
	    }
	  dev->irq_hooked = 1;

	}

    }
#ifdef FSCC_USE_PROC		/* only when available */
  /* this is the last line in init_module */
#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)
  proc_fscc_entry = create_proc_entry("fsccpdrv", S_IFREG | S_IRUGO, 0);
  if (!proc_fscc_entry)
    {
      PDEBUG("Cannot create proc entry!\n");
    }
  else
    {
      PDEBUG("CREATE PROC ENTRY");
      proc_fscc_entry->size = 20;
      proc_fscc_entry->namelen = 8;
      proc_fscc_entry->nlink = 1;
      proc_fscc_entry->name = "fsccpdrv";
      proc_fscc_entry->mode = S_IFREG | S_IRUGO;
      proc_fscc_entry->proc_fops = &fscc_fops;
      proc_fscc_entry->read_proc = &fscc_read_procmem;
    }
#else
  proc_register(&proc_root, &fscc_proc_entry);
#endif
#endif

  PDEBUG("MODULE-INIT\n");
  return 0;			/* succeed */

fail_malloc:unregister_chrdev(fscc_major, "fsccp");
  return result;
}

//called on module unload
//clean up everything allocated here
#ifdef USE_2_6
static void __exit
fscc_cleanup(void)
#else
void
cleanup_module(void)
#endif
{
  Fscc_Dev *dev;
  int i;
  //struct pci_dev *pdev = NULL;  //to get base addresses from pci subsystem

  PDEBUG("MODULE-CLEANUP\n");
  unregister_chrdev(fscc_major, "fsccp");

#ifdef FSCC_USE_PROC
#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)
  remove_proc_entry("fsccpdrv", 0);
#else
  proc_unregister(&proc_root, fscc_proc_entry.low_ino);
#endif
#endif

  for (i = fscc_nr_devs - 1; i >= 0; i--)
    {
      dev = &fscc_devices[i];

//mask IRQ's from card
      WRITE_INTERRUPT_MASK(dev->fsccbase, 0xffffffff);
//unhook IRQ
      if (dev->irq_hooked == 1)
	{
	  free_irq(dev->irq, fscc_devices);
	  dev->irq_hooked = 0;
	}
//give up io regions
#ifdef USE_2_6
#else
//give up io regions
//if(fscc_devices[i].fsccbase !=0) release_region(fscc_devices[i].fsccbase,0xff);
#endif

      if (dev->usedma == 1)
	{
	  stop_dma(dev);
	  cleanup_allocs(dev);
	  dev->usedma = 0;
	}
      else
	{
//free allocated memory (if any)
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
	}
//once per board!
      if ((i % 2) == 0)
	{
	  pci_disable_device(dev->pcidev);

#ifdef USE_2_6
	  pci_dev_put(dev->pcidev);
#endif
	}

    }
//any cleanup then
  kfree(fscc_devices);


  /* and call the cleanup functions for friend devices */

}

#ifdef USE_2_6
module_exit(fscc_cleanup);
module_init(fscc_init);
#endif

void
setup_tmr(Fscc_Dev * ptr)
{
  unsigned long tmp;
//      ptr->rblockcnt = 0;
  tmp = (ptr->board * 2) + ptr->channel;
  init_timer(&ptr->rqt);
  ptr->rqt.function = check_rq_timeout;
  ptr->rqt.data = tmp;
  ptr->rqt.expires = ((HZ * ptr->rqtimeout) / 1000) + jiffies;
  add_timer(&ptr->rqt);
}

void
wsetup_tmr(Fscc_Dev * ptr)
{
  unsigned long tmp;
  tmp = (ptr->board * 2) + ptr->channel;
  init_timer(&ptr->wqt);
  ptr->wqt.function = check_wq_timeout;
  ptr->wqt.data = tmp;
  ptr->wqt.expires = ((HZ * ptr->wqtimeout) / 1000) + jiffies;
  add_timer(&ptr->wqt);
}
