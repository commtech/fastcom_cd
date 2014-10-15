/******************************************************
 *
 * Copyright (C) 2000 Commtech, Inc. Wichita KS
 *
 * support.c -- misc internal support functions for the fscc-pci module
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

If you encounter problems or have suggestions and/or bug fixes please email them to
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
#include <asm/segment.h>	/* memcpy to/from fs */
#include <asm/system.h>		/* cli(), *_flags */
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/msr.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/div64.h>
#include <linux/delay.h>
#include <asm/i387.h>
#include "fsccdrv.h"		/* local definitions */



//function to add a port to the driver list
//(note it had more to do in the ISA driver...)
int
add_port(Fscc_Dev * dev)
{
//int err;
//do the region check , if OK then allocate it,
//if not check to see if we previously allocated it for the other channel
//on this card if so OK, otherwise dump
#ifdef USE_2_6
#else
//if((err=check_region(dev->fsccbase,0xff))<0)
//{
//someone allready has the region requested
//PDEBUG("region request failed:%i\n",err);
// return err; //busy
//}
//request_region(dev->fsccbase,0xff,"fsccp");//allways succeeds (was checked above)
#endif
//note also that we should probably also request the AMCC region in I/O space...

//if we have a previously allocated irq unhook it here
  if (dev->irq_hooked == 1)
    {
//this would actually be a hard error condition...as it should never happen
//release current irq mapping.
      free_irq(dev->irq, fscc_devices);
      dev->irq_hooked = 0;
    }
  WRITE_CCR0(dev->fsccbase, 0x0011201c);
  return 0;
}


uint64_t
get__tsc(void)
{
  uint64_t tsc;
  rdtscll(tsc);
//asm("rdtsc");
  return tsc;
}


static unsigned char xtab[256] = { 0x0,
  0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50,
  0xd0, 0x30, 0xb0, 0x70, 0xf0, 0x8, 0x88, 0x48, 0xc8, 0x28,
  0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78,
  0xf8, 0x4, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14,
  0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4, 0xc, 0x8c, 0x4c,
  0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c,
  0xbc, 0x7c, 0xfc, 0x2, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62,
  0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2, 0xa,
  0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a,
  0xda, 0x3a, 0xba, 0x7a, 0xfa, 0x6, 0x86, 0x46, 0xc6, 0x26,
  0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76,
  0xf6, 0xe, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e,
  0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe, 0x1, 0x81, 0x41,
  0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31,
  0xb1, 0x71, 0xf1, 0x9, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69,
  0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9, 0x5,
  0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55,
  0xd5, 0x35, 0xb5, 0x75, 0xf5, 0xd, 0x8d, 0x4d, 0xcd, 0x2d,
  0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d,
  0xfd, 0x3, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13,
  0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3, 0xb, 0x8b, 0x4b,
  0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b,
  0xbb, 0x7b, 0xfb, 0x7, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67,
  0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7, 0xf,
  0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f,
  0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

void
convertlsb2msb(unsigned char *data, unsigned size)
{
  unsigned i;
  for (i = 0; i < size; i++)
    data[i] = xtab[data[i]];
}


void
DELAY(void)
{
  udelay(10);
}







int
SetICS30703Bits(uint32_t port, unsigned char *progdata)
{
  unsigned long data = 0;
  unsigned long savedval;
  unsigned long i, j;
  savedval = READ_ANY(port);
  savedval = savedval & 0xfffffff0;
  WRITE_ANY(port, savedval & 0xfffffff0);

  //printk("ICS30703: Programming word is: \t0x" );
  for (i = 20; i > 0; i--)
    {
      //printk(":%2.2X", progdata[i-1]);
      for (j = 0; j < 8; j++)
	{
	  if (progdata[i - 1] & 0x80)
	    {
	      data = savedval | 0x01;
	      //printk("1");
	    }
	  else
	    {
	      data = savedval;
	      //printk("0");
	    }
	  WRITE_ANY(port, data);
	  //DELAY();

	  //clock high, data still there
	  data |= 0x02;
	  WRITE_ANY(port, data);
	  //DELAY();
	  //clock low, data still there
	  data &= 0xFFFFFFFd;
	  WRITE_ANY(port, data);
	  //DELAY();


	  progdata[i - 1] = progdata[i - 1] << 1;

	  //printk(" sclk ");
	}

    }
  //printk("\n");
  data = savedval | 0x8;	//strobe on
  WRITE_ANY(port, data);
  //DELAY();        
  data = savedval;		//all off
  WRITE_ANY(port, data);
  //DELAY();

  //printk("cs\n");


  return 0;


}				//end of SetICS30703Bits


int
SetICS30703Bits2(uint32_t port, unsigned char *progdata)
{
  unsigned long data = 0;
  unsigned long savedval;
  unsigned long i, j;
  savedval = READ_ANY(port);
  savedval = savedval & 0xfffff0ff;
  WRITE_ANY(port, savedval & 0xfffff0ff);

  //printk("ICS30703: Programming word is: \t0x" );
  for (i = 20; i > 0; i--)
    {

  //printk(":%2.2X", progdata[i-1]);
      for (j = 0; j < 8; j++)
	{
	  if (progdata[i - 1] & 0x80)
	    {
	      data = savedval | 0x0100;
	      //printk("1");
	    }
	  else
	    {
	      data = savedval;
	      //printk("0");
	    }
	  WRITE_ANY(port, data);
	  //DELAY();

	  //clock high, data still there
	  data |= 0x0200;
	  WRITE_ANY(port, data);
	  //DELAY();
	  //clock low, data still there
	  data &= 0xFFFFFdFF;
	  WRITE_ANY(port, data);
	  //DELAY();


	  progdata[i - 1] = progdata[i - 1] << 1;

	  //printf(" sclk ");
	}

    }
  //printk("\n");
  data = savedval | 0x800;	//strobe on
  WRITE_ANY(port, data);
  //DELAY();        
  data = savedval;		//all off
  WRITE_ANY(port, data);
  //DELAY();

  //printf("cs\n");


  return 0;


}				//end of SetICS30703Bits



// I/O mapped registers
void
READ_FIFO(uint32_t port, uint32_t * buf, uint32_t count)
{
  //pull data from the rx fifo
  READ_PORT_BUFFER_ULONG(port + FIFO, buf, count);
}

void
WRITE_FIFO(uint32_t port, uint32_t * buf, uint32_t count)
{
  //push data to the tx fifo
  WRITE_PORT_BUFFER_ULONG(port + FIFO, buf, count);
}

void
WRITE_TX_COUNT(uint32_t port, uint32_t count)
{
  //sets the number of bytes in the current (tx) frame
  WRITE_PORT_ULONG(port + TXBCR, count);
}

uint32_t
READ_RX_COUNT(uint32_t port)
{
  //returns the number of bytes in the current (rx) frame
  return (READ_PORT_ULONG(port + RXBCR));
}

uint32_t
READ_RX_TRIGGER(uint32_t port)
{
  return (READ_PORT_ULONG(port + FTRIG) & RXTRIG);
}

void
WRITE_RX_TRIGGER(uint32_t port, uint32_t trig)
{
  uint32_t temp;
  temp = READ_PORT_ULONG(port + FTRIG);
  temp = (temp & (~RXTRIG)) | trig;
  WRITE_PORT_ULONG(port + FTRIG, temp);
}

uint32_t
READ_TX_TRIGGER(uint32_t port)
{
  return ((READ_PORT_ULONG(port + FTRIG) & TXTRIG) >> 16);
}

void
WRITE_TX_TRIGGER(uint32_t port, uint32_t trig)
{
  uint32_t temp;
  temp = READ_PORT_ULONG(port + FTRIG);
  temp = (temp & (~TXTRIG)) | (trig << 16);
  WRITE_PORT_ULONG(port + FTRIG, temp);
}

uint32_t
READ_RX_FIFO_BYTE_COUNT(uint32_t port)
{
  //returns the number of bytes in the receive fifo (total)
  return (READ_PORT_ULONG(port + FBCR) & RXCNT);
}

uint32_t
READ_TX_FIFO_BYTE_COUNT(uint32_t port)
{
  //returns the number of bytes in the transmit fifo (total)
  return ((READ_PORT_ULONG(port + FBCR) >> 16) & 0x00001fff);
}

uint32_t
READ_RX_FRAME_COUNT(uint32_t port)
{
  //returns number of frames in the receive fifo
  return (READ_PORT_ULONG(port + FFCR) & RFCNT);
}

uint32_t
READ_TX_FRAME_COUNT(uint32_t port)
{
  //returns the number of frames in the transmit fifo
  return ((READ_PORT_ULONG(port + FFCR) >> 16) & 0x000001ff);
}

uint32_t
WRITE_COMMAND(uint32_t port, uint32_t cmd)
{
  uint32_t status;
  unsigned long timeout;
  timeout = 0;
  do
    {
      status = READ_PORT_ULONG(port + STAR);
      timeout++;
    }
  while (((status & CE) != 0) && (timeout < 100000));
  if (timeout >= 100000)
    {
      return -1;
    }
  else
    {
      WRITE_PORT_ULONG(port + CMDR, cmd);
    }
  return 0;
}

uint32_t
READ_STATUS(uint32_t port)
{
  return (READ_PORT_ULONG(port + STAR));
}

uint32_t
READ_CCR0(uint32_t port)
{
  return (READ_PORT_ULONG(port + CCR0));
}

void
WRITE_CCR0(uint32_t port, uint32_t ccr0)
{
  WRITE_PORT_ULONG(port + CCR0, ccr0);
}

uint32_t
READ_CCR1(uint32_t port)
{
  return (READ_PORT_ULONG(port + CCR1));
}

void
WRITE_CCR1(uint32_t port, uint32_t ccr1)
{
  WRITE_PORT_ULONG(port + CCR1, ccr1);
}

uint32_t
READ_CCR2(uint32_t port)
{
  return (READ_PORT_ULONG(port + CCR2));
}

void
WRITE_CCR2(uint32_t port, uint32_t ccr2)
{
  WRITE_PORT_ULONG(port + CCR2, ccr2);
}

uint32_t
READ_BGR(uint32_t port)
{
  return (READ_PORT_ULONG(port + BGR));
}

void
WRITE_BGR(uint32_t port, uint32_t bgr)
{
  WRITE_PORT_ULONG(port + BGR, bgr);
}

uint32_t
READ_SYNC_SEQUENCE(uint32_t port)
{
  return (READ_PORT_ULONG(port + SSR));
}

void
WRITE_SYNC_SEQUENCE(uint32_t port, uint32_t ssr)
{
  WRITE_PORT_ULONG(port + SSR, ssr);
}

uint32_t
READ_SYNC_SEQUENCE_MASK(uint32_t port)
{
  return (READ_PORT_ULONG(port + SMR));
}

void
WRITE_SYNC_SEQUENCE_MASK(uint32_t port, uint32_t smr)
{
  WRITE_PORT_ULONG(port + SMR, smr);
}

uint32_t
READ_TERMINATION_SEQUENCE(uint32_t port)
{
  return (READ_PORT_ULONG(port + TSR));
}

void
WRITE_TERMINATION_SEQUENCE(uint32_t port, uint32_t tsr)
{
  WRITE_PORT_ULONG(port + TSR, tsr);
}

uint32_t
READ_TERMINATION_SEQUENCE_MASK(uint32_t port)
{
  return (READ_PORT_ULONG(port + TMR));
}

void
WRITE_TERMINATION_SEQUENCE_MASK(uint32_t port, uint32_t tmr)
{
  WRITE_PORT_ULONG(port + TMR, tmr);
}

uint32_t
READ_RECEIVE_ADDRESS(uint32_t port)
{
  return (READ_PORT_ULONG(port + RAR));
}

void
WRITE_RECEIVE_ADDRESS(uint32_t port, uint32_t rar)
{
  WRITE_PORT_ULONG(port + RAR, rar);
}

uint32_t
READ_RECEIVE_ADDRESS_MASK(uint32_t port)
{
  return (READ_PORT_ULONG(port + RAMR));
}

void
WRITE_RECEIVE_ADDRESS_MASK(uint32_t port, uint32_t ramr)
{
  WRITE_PORT_ULONG(port + RAMR, ramr);
}

uint32_t
READ_PREAMBLE(uint32_t port)
{
  return (READ_PORT_ULONG(port + PPR));
}

void
WRITE_PREAMBLE(uint32_t port, uint32_t ppr)
{
  WRITE_PORT_ULONG(port + PPR, ppr);
}

uint32_t
READ_TIMER_CONTROL(uint32_t port)
{
  return (READ_PORT_ULONG(port + TCR));
}

void
WRITE_TIMER_CONTROL(uint32_t port, uint32_t tcr)
{
  WRITE_PORT_ULONG(port + TCR, tcr);
}

uint32_t
READ_VERSION(uint32_t port)
{
  return (READ_PORT_ULONG(port + VSTR));
}

uint32_t
READ_INTERRUPT_STATUS(uint32_t port)
{
  return (READ_PORT_ULONG(port + ISR));
}

void
WRITE_INTERRUPT_MASK(uint32_t port, uint32_t imr)
{
  WRITE_PORT_ULONG(port + IMR, imr);
}

uint32_t
READ_GLOBAL_INTERRUPT_STATUS(uint32_t port)
{
  return (READ_PORT_ULONG(port + GIS));
}

uint32_t
GET_CD_STATE(uint32_t port)
{
  uint32_t status;
  status = READ_STATUS(port);
  if ((status & CD) == CD)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

uint32_t
GET_CTS_STATE(uint32_t port)
{
  uint32_t status;
  status = READ_STATUS(port);
  if ((status & CTS) == CTS)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

uint32_t
GET_DSR_STATE(uint32_t port)
{
  uint32_t status;
  status = READ_STATUS(port);
  if ((status & DSR) == DSR)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

uint32_t
GET_RI_STATE(uint32_t port)
{
  uint32_t status;
  status = READ_STATUS(port);
  if ((status & RI) == RI)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

uint32_t
READ_ANY(uint32_t port)
{
  return (READ_PORT_ULONG(port));
}

void
WRITE_ANY(uint32_t port, uint32_t val)
{
  WRITE_PORT_ULONG(port, val);
}



int
fscc_tx_start(Fscc_Dev * dev)
{
  //this routine is used from the dispatch when there is an immediate
  //need to transmit a frame, it starts the interrupt chain flowing
  uint32_t port;
  //unsigned long flags;
  port = dev->fsccbase;
  if ((READ_STATUS(port) & TBC) != 0)
    {
      return 0;			//can't write to fifo/frame
    }
    //if (dev->is_transmitting == 0)
      //{
      fill_tx_fifo(dev);
      //}
  return 0;
}

int
fscc_start_tx_dma(Fscc_Dev * dev)
{
  uint32_t status;
  uint32_t tempul;
//check to see if we are in fullstop:
  status = inl(dev->bsrbase + 0x30);

  if ((dev->channel == 0) && ((status & 0x02) == 0x02))
    {
      //printk("tx startup!\n");

      //is fullstopped, so take starting index and poke its address into the base tx register and issue a gotxa
      WRITE_PORT_ULONG(dev->bsrbase + 0x10, dev->phys_txbdalist[dev->current_txbuf]);	//poke address into base descriptor register
      //printk("base:%8.8x\n",READ_PORT_ULONG(dev->bsrbase+0x0c));
      //printk("next:%8.8x\n",dev->Txbdalist[dev->current_txbuf]->next_desc);
      tempul = READ_PORT_ULONG(dev->bsrbase + 0x04) & 0x03000000;	//save dma enables
	if(dev->transmit_repeat==0) tempul=tempul&0xbfffffff;
	else tempul |= 0x40000000;//enable transmit repeat

	if(dev->external_transmit==0) tempul=tempul&0xdfffffff;
	else tempul |= 0x20000000;//enable external transmit

	if(dev->timed_transmit==0) tempul=tempul&0xefffffff;     
	else tempul |= 0x10000000;


 WRITE_PORT_ULONG(dev->bsrbase + 0x04, tempul | 0x00000002);	//gota request
    }
  else if ((dev->channel == 0) && ((status & 0x02) == 0x00))
    {
      //printk("ch0 already running!\n");
      //already running so we shouldn't have to do anything...shouldn't being the key word here 
//no damn good, currently we are only doing loopback, so it is not continuous write, it should have completed
//since it didn't it means that either we never posted/completed the last descripter, or ???
//stop and restart the tx statemachine
/*	tempul=READ_PORT_ULONG(dev->bsrbase+0x04)&0x03000000;//save dma enables
	WRITE_PORT_ULONG(dev->bsrbase+0x04,tempul|0x00000200);//stopt request
while((dev->channel==0)&&((status&0x02)==0x00))
	{
	status = inl(dev->bsrbase+0x30);
	}
*/
      //
      //WRITE_PORT_ULONG(dev->bsrbase+0x10,dev->phys_txbdalist[dev->current_txbuf]);//poke address into base descriptor register
      //tempul=READ_PORT_ULONG(dev->bsrbase+0x04)&0x03000000;//save dma enables
      //WRITE_PORT_ULONG(dev->bsrbase+0x04,tempul|0x00000002);//gota request
      return -1;
    }
  else if ((dev->channel == 1) && ((status & 0x08) == 0x08))
    {
      //is fullstopped, so take starting index and poke its address into the base tx register and issue a gotxb
      WRITE_PORT_ULONG(dev->bsrbase + 0x018, dev->phys_txbdalist[dev->current_txbuf]);	//poke address into base descriptor register
      tempul = READ_PORT_ULONG(dev->bsrbase + 0x08) & 0x03000000;	//save dma enables
	if(dev->transmit_repeat==0) tempul=tempul&0xbfffffff;
	else tempul |= 0x40000000;//enable transmit repeat

	if(dev->external_transmit==0) tempul=tempul&0xdfffffff;
	else tempul |= 0x20000000;//enable external transmit

	if(dev->timed_transmit==0) tempul=tempul&0xefffffff;     
	else tempul |= 0x10000000;


      WRITE_PORT_ULONG(dev->bsrbase + 0x08, tempul | 0x00000002);	//issue gotb request
    }
  else if ((dev->channel == 1) && ((status & 0x08) == 0x00))
    {
      //printk("ch1 already running!\n");
      //already running so we shouldn't have to do anything...shouldn't being the key word here 
      //WRITE_PORT_ULONG(dev->bsrbase+0x018,dev->phys_txbdalist[dev->current_txbuf]);//poke address into base descriptor register
      //tempul=READ_PORT_ULONG(dev->bsrbase+0x08)&0x03000000;//save dma enables
      //WRITE_PORT_ULONG(dev->bsrbase+0x08,tempul|0x00000002);//issue gotb request
      return -1;
    }
  return 0;
}


int
allocate_desc(Fscc_Dev * dev)
{
  int i;
  //printk("cleanup allocs\n");
  cleanup_allocs(dev);
  //printk("allcocating\n");

  dev->Txbdalist = (DESC **) kmalloc(dev->settings.n_tbufs * sizeof(DESC *), GFP_KERNEL);
  dev->Rxbdalist = (DESC **) kmalloc(dev->settings.n_rbufs * sizeof(DESC *), GFP_KERNEL);
  dev->rxdata = (unsigned char **) kmalloc(dev->settings.n_rbufs * sizeof(char *), GFP_KERNEL);
  dev->txdata = (unsigned char **) kmalloc(dev->settings.n_tbufs * sizeof(char *), GFP_KERNEL);
  dev->phys_rxdata = (dma_addr_t *) kmalloc(dev->settings.n_rbufs * sizeof(dma_addr_t), GFP_KERNEL);
  dev->phys_txdata = (dma_addr_t *) kmalloc(dev->settings.n_tbufs * sizeof(dma_addr_t), GFP_KERNEL);
  dev->phys_txbdalist = (dma_addr_t *) kmalloc(dev->settings.n_tbufs * sizeof(dma_addr_t), GFP_KERNEL);
  dev->phys_rxbdalist = (dma_addr_t *) kmalloc(dev->settings.n_rbufs * sizeof(dma_addr_t), GFP_KERNEL);

  if ((dev->Txbdalist == NULL) || (dev->Rxbdalist == NULL) || (dev->rxdata == NULL) || (dev->txdata == NULL)
      || (dev->phys_rxdata == NULL) || (dev->phys_txdata == NULL) || (dev->phys_txbdalist == NULL)
      || (dev->phys_rxbdalist == NULL))
    {
      return -ENOMEM;
    }

  for (i = 0; i < dev->settings.n_rbufs; i++)
    {
      dev->Rxbdalist[i] = (DESC *) pci_alloc_consistent(dev->pcidev, sizeof(DESC), &dev->phys_rxbdalist[i]);
      if (dev->Rxbdalist[i] == NULL)
	{
	  cleanup_allocs(dev);
	  return -ENOMEM;
	}
      dev->rxdata[i] =
	(uint8_t *) pci_alloc_consistent(dev->pcidev, dev->settings.n_rfsize_max * sizeof(uint8_t),
					 &dev->phys_rxdata[i]);
      if (dev->rxdata[i] == NULL)
	{
	  cleanup_allocs(dev);
	  return -ENOMEM;
	}
    }
  for (i = 0; i < dev->settings.n_tbufs; i++)
    {
      dev->Txbdalist[i] = (DESC *) pci_alloc_consistent(dev->pcidev, sizeof(DESC), &dev->phys_txbdalist[i]);
      if (dev->Txbdalist[i] == NULL)
	{
	  cleanup_allocs(dev);
	  return -ENOMEM;
	}
      dev->txdata[i] =
	(uint8_t *) pci_alloc_consistent(dev->pcidev, dev->settings.n_tfsize_max * sizeof(uint8_t),
					 &dev->phys_txdata[i]);
      if (dev->txdata[i] == NULL)
	{
	  cleanup_allocs(dev);
	  return -ENOMEM;
	}
    }
  return 0;
}


void
cleanup_allocs(Fscc_Dev * dev)
{
  int i;

  if (dev->Txbdalist != NULL)
    {
      for (i = 0; i < dev->settings.n_tbufs; i++)
	{
	  if (dev->Txbdalist[i] != NULL)
	    {
	      pci_free_consistent(dev->pcidev, sizeof(DESC), dev->Txbdalist[i], dev->phys_txbdalist[i]);
	    }
	}
      kfree(dev->Txbdalist);
    }
  if (dev->Rxbdalist != NULL)
    {
      for (i = 0; i < dev->settings.n_rbufs; i++)
	{
	  if (dev->Rxbdalist[i] != NULL)
	    {
	      pci_free_consistent(dev->pcidev, sizeof(DESC), dev->Rxbdalist[i], dev->phys_rxbdalist[i]);
	    }
	}
      kfree(dev->Rxbdalist);
    }
  if (dev->txdata != NULL)
    {
      for (i = 0; i < dev->settings.n_tbufs; i++)
	{
	  if (dev->txdata[i] != NULL)
	    {
	      pci_free_consistent(dev->pcidev, dev->settings.n_tfsize_max * sizeof(uint8_t), dev->txdata[i],
				  dev->phys_txdata[i]);
	    }
	}
      kfree(dev->txdata);
    }
  if (dev->rxdata != NULL)
    {
      for (i = 0; i < dev->settings.n_rbufs; i++)
	{
	  if (dev->rxdata[i] != NULL)
	    {
	      pci_free_consistent(dev->pcidev, dev->settings.n_rfsize_max * sizeof(uint8_t), dev->rxdata[i],
				  dev->phys_rxdata[i]);
	    }
	}
      kfree(dev->rxdata);
    }
  if (dev->phys_rxdata != NULL)
    {
      kfree(dev->phys_rxdata);
    }
  if (dev->phys_txdata != NULL)
    {
      kfree(dev->phys_txdata);
    }
  if (dev->phys_txbdalist != NULL)
    {
      kfree(dev->phys_txbdalist);
    }
  if (dev->phys_rxbdalist != NULL)
    {
      kfree(dev->phys_rxbdalist);
    }

  dev->phys_rxdata = NULL;
  dev->phys_txdata = NULL;
  dev->phys_txbdalist = NULL;
  dev->phys_rxbdalist = NULL;
  dev->rxdata = NULL;
  dev->txdata = NULL;
  dev->Txbdalist = NULL;
  dev->Rxbdalist = NULL;
}


void
init_rx_desc(Fscc_Dev * dev)
{
  int i;
  for (i = 0; i < dev->settings.n_rbufs - 1; i++)
    {
      if ((i % dev->rirqrate) == 0)
	{
	  dev->Rxbdalist[i]->control = 0x20000000;
	}
      else
	{
	  dev->Rxbdalist[i]->control = 0x00000000;
	}
      dev->Rxbdalist[i]->data_address = dev->phys_rxdata[i];
      dev->Rxbdalist[i]->data_count = dev->settings.n_rfsize_max;
      dev->Rxbdalist[i]->next_desc = dev->phys_rxbdalist[i + 1];
      //printk("rx [%d]@%8.8x\n",i,dev->phys_rxbdalist[i]);
      //printk("rxd[%d]@%8.8x\n",i,dev->phys_rxdata[i]);
    }
  if ((i % dev->rirqrate) == 0)
    {
      dev->Rxbdalist[i]->control = 0x20000000;
    }
  else
    {
      dev->Rxbdalist[i]->control = 0x00000000;
    }
  dev->Rxbdalist[i]->data_address = dev->phys_rxdata[i];
  dev->Rxbdalist[i]->data_count = dev->settings.n_rfsize_max;
  dev->Rxbdalist[i]->next_desc = dev->phys_rxbdalist[0];
  dev->current_rxbuf = 0;

}

void
init_tx_desc(Fscc_Dev * dev)
{
  int i;
  for (i = 0; i < dev->settings.n_tbufs - 1; i++)
    {
      if ((i % dev->tirqrate) == 0)
	{
	  dev->Txbdalist[i]->control = 0x60000000;
	}
      else
	{
	  dev->Txbdalist[i]->control = 0x40000000;
	}
      dev->Txbdalist[i]->data_address = dev->phys_txdata[i];
      dev->Txbdalist[i]->data_count = 0;
      dev->Txbdalist[i]->next_desc = dev->phys_txbdalist[i + 1];
    }
  if ((i % dev->tirqrate) == 0)
    {
      dev->Txbdalist[i]->control = 0x60000000;
    }
  else
    {
      dev->Txbdalist[i]->control = 0x40000000;
    }
  dev->Txbdalist[i]->data_address = dev->phys_txdata[i];
  dev->Txbdalist[i]->data_count = 0;
  dev->Txbdalist[i]->next_desc = dev->phys_txbdalist[0];
  dev->current_txbuf = 0;
}

void
enable_dma(Fscc_Dev * dev)
{
  uint32_t tempul;
  if (dev->channel == 0)
    {
      tempul = READ_PORT_ULONG(dev->bsrbase + 0x04);
      tempul = tempul & 0x03000000;
      WRITE_PORT_ULONG(dev->bsrbase + 0x04, tempul | 0x03000000);
    }
  if (dev->channel == 1)
    {
      tempul = READ_PORT_ULONG(dev->bsrbase + 0x08);
      tempul = tempul & 0x03000000;
      WRITE_PORT_ULONG(dev->bsrbase + 0x08, tempul | 0x03000000);
    }
  dev->usedma = 1;
}

void
stop_dma(Fscc_Dev * dev)
{
  uint32_t tempul;
  if (dev->channel == 0)
    {
      tempul = READ_PORT_ULONG(dev->bsrbase + 0x04);
      tempul = tempul & 0x03000000;
      WRITE_PORT_ULONG(dev->bsrbase + 0x04, tempul | 0x00000300);	//stop transmit and receive dma
      WRITE_PORT_ULONG(dev->bsrbase + 0x04, tempul | 0x00000030);	//reset transmit and receive dma
      WRITE_PORT_ULONG(dev->bsrbase + 0x04, 0x00000000);	//disable dma cha
    }
  if (dev->channel == 1)
    {
      tempul = READ_PORT_ULONG(dev->bsrbase + 0x08);
      tempul = tempul & 0x03000000;
      WRITE_PORT_ULONG(dev->bsrbase + 0x08, tempul | 0x00000300);	//stop transmit and receive dma
      WRITE_PORT_ULONG(dev->bsrbase + 0x08, tempul | 0x00000030);	//reset transmit and receive dma
      WRITE_PORT_ULONG(dev->bsrbase + 0x08, 0x00000000);	//disalbe dma chb
    }
  dev->usedma = 0;
}

void
init_rx_dma(Fscc_Dev * dev)
{
  uint32_t tempul;
  uint32_t status;
  unsigned long timeout;

  status = READ_PORT_ULONG(dev->bsrbase + 0x30);
  WRITE_CCR0(dev->fsccbase, dev->settings.ccr0 | 0x02000000);	//turn off receiver

  if (dev->channel == 0)
    {
      tempul = READ_PORT_ULONG(dev->bsrbase + 0x04);
      tempul = tempul & 0x03000000;
      if ((status & 0x01) == 0)
	{
	  WRITE_PORT_ULONG(dev->bsrbase + 0x04, (tempul & 0x02000000) | 0x00000100);	//stopra
	}

      WRITE_PORT_ULONG(dev->bsrbase + 0x04, (tempul & 0x02000000) | 0x00000010);	//reset ra
      WRITE_COMMAND(dev->fsccbase, RRES);
      timeout = 0;
      while (((READ_STATUS(dev->fsccbase) & CE) == CE) && (timeout < 100000))
	{
	  timeout++;
	}
      if (timeout == 100000)
	{
	  printk("fsccdrv:uh oh, timeout on CEC after RRES! in flushrx\n");
	}

      init_rx_desc(dev);
      WRITE_PORT_ULONG(dev->bsrbase + 0x0c, dev->phys_rxbdalist[0]);
      dev->current_rxbuf = 0;
      WRITE_PORT_ULONG(dev->bsrbase + 0x04, tempul | 0x00000001);	//gora
    }
  else if (dev->channel == 1)
    {
      tempul = READ_PORT_ULONG(dev->bsrbase + 0x08);
      tempul = tempul & 0x03000000;

      if ((status & 0x04) == 0)
	{
	  WRITE_PORT_ULONG(dev->bsrbase + 0x08, (tempul & 0x02000000) | 0x00000100);	//stoprb
	}

      WRITE_PORT_ULONG(dev->bsrbase + 0x08, (tempul & 0x02000000) | 0x00000010);	//reset rb
      WRITE_COMMAND(dev->fsccbase, RRES);
      timeout = 0;
      while (((READ_STATUS(dev->fsccbase) & CE) == CE) && (timeout < 100000))
	{
	  timeout++;
	}
      if (timeout == 100000)
	{
	  printk("fsccdrv:uh oh, timeout on CEC after RRES! in flushrx\n");
	}


      WRITE_PORT_ULONG(dev->bsrbase + 0x014, dev->phys_rxbdalist[0]);
      dev->current_rxbuf = 0;
      WRITE_PORT_ULONG(dev->bsrbase + 0x08, tempul | 0x00000001);	//gorb

    }
  WRITE_CCR0(dev->fsccbase, dev->settings.ccr0);	//turn on receiver

}

int
get_numt_empty(Fscc_Dev * dev)
{
  int i;
  int numempty;
  DESC *tdesc;
  struct buf *tbuf;
  numempty = 0;
  for (i = 0; i < dev->settings.n_tbufs; i++)
    {
      if (dev->usedma == 1)
	{
	  tdesc = dev->Txbdalist[i];
	  if ((tdesc->control == 0x40000000) || (tdesc->control == 0x60000000))
	    {
	      numempty++;
	    }
	}
      else
	{
	  tbuf = &dev->fscc_tbuf[i];
	  if (tbuf->valid == 0)
	    {
	      numempty++;
	    }
	}
    }

  return numempty;
}


int
get_t_empty(Fscc_Dev * dev, int count)
{
  int i, k;
  int numempty;
  DESC *tdesc;
  struct buf *tbuf;
  numempty = 0;
  if (dev->usedma == 1)
    {
//for DMA mode dev->current_txbuf is where we are placing data into buffers
//the dma controller holds the "current" buffer/descriptor and increments 
//it as necessary.  As a result we always need to look forward in the 
//buffer list to see if there are any empty spots
  i = dev->current_txbuf;
  i++;
  if (i >= dev->settings.n_tbufs) 
    {
      i = 0;
    }

      for (k = 0; k < count; k++)
	{
	  tdesc = dev->Txbdalist[i];
	  if ((tdesc->control == 0x40000000) || (tdesc->control == 0x60000000))
	    {
	      numempty++;
	    }
	  i++;
	  if (i >=dev->settings.n_tbufs)
	    {
	      i = 0;
	    }
	}
    }
  else
    {
//int interrupt mode dev->current_txbuf holds the actual current buffer/descriptor
//that is being used by the ISR/driver to fill the fifo.
//as a result we look at slots behind this one to determine if there are enough
//to satisfy the request, as the slots in front of current could well hold queued frames
//
//The long and short of it is that in DMA mode we increment current_txbuf as 
//we place frames in the queue, in IRQ mode the ISR increments current_txbuf
//as it finishes using the data in the buffer.
    i = dev->current_txbuf;
    i--;
    if (i < 0 ) 
      {
       i = dev->settings.n_tbufs-1;
      }

      for (k = 0; k < count; k++)
	{
	  tbuf = &dev->fscc_tbuf[i];
	  if (tbuf->valid == 0)
	    {
	      numempty++;
	    }
	  i--;
	  if (i <0 )
	    {
	      i = dev->settings.n_tbufs-1;
	    }
	}
    }
  if (numempty >= count)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

int
fill_tx_fifo(Fscc_Dev * dev)
{
  unsigned long sz;
  unsigned long fbc;
  struct buf *tbuf;
  uint32_t port;
  int bytespushed;
  int sendcmd;


  port = dev->fsccbase;
  bytespushed = 0;
  sendcmd = 0;

  if (dev->usedma == 1)
    {
    }
  else
    {
      sz = TXFIFOSIZE;
      fbc = READ_TX_FIFO_BYTE_COUNT(port);
      //printk("fbc:%d\n",fbc);
      if (fbc > 0x1000)
	{
	  //here is our problem!
	  printk("Yikes the fbc is >4k fbc=%ld\n", fbc);
	  fbc = fbc & 0x0fff;
	}
      sz = sz - (fbc & 0x01fff);
      //szorig=sz;

      sz = sz / 4;
      sz = sz * 4;

      if (sz > 4096)
	{
	  unsigned long i;
	  printk("Yikes we are supposed to write more than 4096 bytes on a TFT! sz=:%ld\n", sz);

	  dev->current_txbuf = 0;
	  for (i = 0; i < dev->settings.n_tbufs; i++)
	    {
	      dev->fscc_tbuf[i].valid = 0;
	      dev->fscc_tbuf[i].no_bytes = 0;
	      dev->fscc_tbuf[i].fe = 0;
	      dev->fscc_tbuf[i].ftot = 0;
	    }
	  atomic_set(&dev->transmit_frames_available, dev->settings.n_tbufs);
	  dev->is_transmitting = 0;
	  WRITE_COMMAND(dev->fsccbase, TRES);

	  dev->status |= ST_TX_DONE;
	}
      else
	{
	  //printk("fillfifo-current:%d\n",dev->current_txbuf);
	  tbuf = &dev->fscc_tbuf[dev->current_txbuf];

	  while ((sz > 0) && (tbuf->valid == 1) && ((READ_STATUS(port) & TBC) == 0))
	    {
	      if (sz >= (tbuf->max - tbuf->no_bytes))
		{
		  unsigned long writesz;
		  writesz = tbuf->max - tbuf->no_bytes;
		  writesz = writesz / 4;
		  if (((tbuf->max - tbuf->no_bytes) % 4) != 0)
		    {
		      writesz += 1;
		    }
		  //printk("iwritesz:%d\n",writesz);
		  //printk("bytesin:%d\n",READ_TX_FIFO_BYTE_COUNT(port));
		  WRITE_FIFO(port, (uint32_t *) & tbuf->frame[tbuf->no_bytes], writesz);
		  bytespushed++;
		  if ((tbuf->fe == 1) && (tbuf->no_bytes == 0))
		    {
		      //printk("IWRITETXCOUNT:%d\n",tbuf->ftot);
		      WRITE_TX_COUNT(port, tbuf->ftot);
		      sendcmd = 1;
		    }

		  tbuf->valid = 0;
		  dev->status |= ST_TX_DONE;
		  atomic_inc(&dev->transmit_frames_available);
		  sz -= (writesz * 4);	//(tbuf->max - tbuf->no_bytes);
		  tbuf->no_bytes = 0;
		  tbuf->max = 0;
		  tbuf->fe = 0;
		  tbuf->ftot = 0;
		  dev->current_txbuf++;
		  if (dev->current_txbuf == dev->settings.n_tbufs)
		    {
		      dev->current_txbuf = 0;
		    }
		  if (sz < 4)
		    {
		      sz = 0;
		    }
		}
	      else
		{
		  unsigned long writesz;
		  writesz = sz;
		  writesz = sz / 4;
		  //if(((sz)%4)!=0) writesz+=1;
		  //printk("iwritesz:%d\n",writesz);
		  WRITE_FIFO(port, (uint32_t *) & tbuf->frame[tbuf->no_bytes], writesz);
		  bytespushed++;
		  if ((tbuf->fe == 1) && (tbuf->no_bytes == 0))
		    {
		      //printk("IWRITETXCOUNT:%d\n",tbuf->ftot);
		      WRITE_TX_COUNT(port, tbuf->ftot);
		      sendcmd = 1;
		    }
		  tbuf->no_bytes += (writesz * 4);
		  sz = 0;	//force us to drop out
		}
	      tbuf = &dev->fscc_tbuf[dev->current_txbuf];
	      //WRITE_COMMAND(port,XF);
	    }
	  /* I don't believe this does what I think I wanted      
	     if((sz>0)&&(tbuf->valid==0))
	     {
	     //printk("B\n");
	     dev->is_transmitting=0;
	     }
	   */
	}
      if (sendcmd == 1)
	{
	  uint32_t outboundcmd;
	  outboundcmd = XF;
	  if (dev->timed_transmit == 1)
	    {
	      outboundcmd |= TXT;
	    }
	  if (dev->external_transmit == 1)
	    {
	      outboundcmd |= TXEXT;
	    }
	  if (dev->transmit_repeat == 1)
	    {
	      outboundcmd |= XREP;
	    }
	  WRITE_COMMAND(port, outboundcmd);
	  dev->is_transmitting = 1;
	  dev->bits_going_out = 1;
	}
    }
  if (bytespushed != 0)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

int
empty_rx_fifo(Fscc_Dev * dev)
{
  unsigned long sz;
  int rxframesize;
  int num_reads;
  struct buf *rbuf;
  uint32_t port;


  port = dev->fsccbase;
  if (dev->usedma == 1)
    {
    }
  else
    {
      rbuf = &dev->fscc_rbuf[dev->current_rxbuf];
      //receive frame end
      if (READ_RX_FIFO_BYTE_COUNT(port) == 0)
	{
	}
      else
	{
	getanotherx:
	  if (READ_RX_FRAME_COUNT(port) > 0)
	    {
	      rxframesize = READ_RX_COUNT(port);	//to get the frame size
	      if (rxframesize != 0)
		{
		  //printk("rxframesize:%d\n",rxframesize);
		  sz = rxframesize;
		  sz = sz - dev->current_frame_rbytes;
		  num_reads = sz / 4;
		  if ((sz % 4) != 0)
		    {
		      num_reads++;
		    }
		  //printk("rx:%d, sz:%ld, nm:%d\n",rxframesize,sz,num_reads);
		  if (num_reads > 2048)
		    {
		      //Houston we have a problem...we are supposed to read more than there is fifo...
		      //just finish the frame and flush?
		      printk("Yikes, we are supposed to read more than 2048 times on a RFE! sz=%d\n", num_reads);
		      printk("current_frame_rbytes:%ld\n", dev->current_frame_rbytes);
		      printk("rxframesize: %d\n", rxframesize);
		      printk("bytesinrxfifo:%d\n", READ_RX_FIFO_BYTE_COUNT(port));
		      rbuf->valid = 1;
		      rbuf->fe = 1;
		      dev->current_frame_rbytes = 0;
		      atomic_inc(&dev->received_frames_pending);
		      atomic_inc(&dev->received_frames);
		      dev->status |= ST_RX_DONE;
		      dev->current_rxbuf++;
		      if (dev->current_rxbuf == dev->settings.n_rbufs)
			{
			  dev->current_rxbuf = 0;
			}
		      WRITE_COMMAND(port, RRES);
		    }
		  else
		    {
		      //printk("RFE:rdcount=%d, sz=%d numreads:%d\n",rxframesize,sz,num_reads);
		      dev->current_frame_rbytes = 0;

		    ragain2:
		      rbuf = &dev->fscc_rbuf[dev->current_rxbuf];
		      if (rbuf->valid == 1)
			{
			  //oops we have wrapped the receive buffers
			  //signal it and reuse;
			  rbuf->valid = 0;
			  rbuf->no_bytes = 0;
			  if (rbuf->fe == 1)
			    {
			      atomic_dec(&dev->received_frames_pending);
			      atomic_dec(&dev->received_frames);
			    }
			  rbuf->fe = 0;
			  dev->status |= ST_RBUF_OVERFLOW;
			}
		      //check that sz will fit in to the buffer location
		      if (sz + rbuf->no_bytes > dev->settings.n_rfsize_max)
			{
			  //fill up to n_rfsize_max, then go to the next one
			  READ_FIFO(port, (uint32_t *) (&rbuf->frame[rbuf->no_bytes]),
				    (dev->settings.n_rfsize_max - rbuf->no_bytes) / 4);
			  sz -= (((dev->settings.n_rfsize_max - rbuf->no_bytes) / 4) * 4);
			  num_reads -= ((dev->settings.n_rfsize_max - rbuf->no_bytes) / 4);
			  rbuf->valid = 1;
			  //this is a buffer rollover condition, but
			  //if we are here then by definition we have an end condition
			  //so there should be more bytes and a complete/end buffer indicated below
			  rbuf->no_bytes += (((dev->settings.n_rfsize_max - rbuf->no_bytes) / 4) * 4);
			  dev->current_rxbuf++;
			  if (dev->current_rxbuf == dev->settings.n_rbufs)
			    {
			      dev->current_rxbuf = 0;
			    }
			  //printk("RFE:filled descriptor\n");
			  goto ragain2;
			  //error condition, software overflow.  Received frame has more bytes
			  //than our frame buffer can hold...I'm affraid that some bits are going
			  //to get dropped on the floor here
			}
		      else
			{
			  //get the data from the hardware
			  READ_FIFO(port, (uint32_t *) & rbuf->frame[rbuf->no_bytes], num_reads);	//sz is in bytes, and we are reading dwords here (the count value is in DWORDS)
			  //increment the numbytes in current frame received
			  rbuf->no_bytes += sz;
			  if (dev->allowcutoff == 1)
			    {
			      rbuf->no_bytes -= 2;	//adjust for transparent mode operation--strip the status word
			    }

			  rbuf->valid = 1;
			  rbuf->fe = 1;
			  atomic_inc(&dev->received_frames_pending);
			  atomic_inc(&dev->received_frames);
			  dev->status |= ST_RX_DONE;
			  dev->current_rxbuf++;
			  if (dev->current_rxbuf == dev->settings.n_rbufs)
			    {
			      dev->current_rxbuf = 0;
			    }
			  //printk("RFE:filled descriptor+complete\n");
			}
		      fill_tx_fifo(dev);
		      if (READ_RX_FRAME_COUNT(port) > 0)
			{
			  goto getanotherx;
			}

		    }		//end framesize is valid
		}		//end framesize not zero
	    }			//end rx framecount >0 (RFE)
	  else
	    {
	      sz = READ_RX_FIFO_BYTE_COUNT(port);	//get whats actually in the fifo...
	      sz = (sz / 4) * 4;	//we only do dword reads
	      //printk("RFT:%u\n",sz);
	      if (sz > 8192)
		{
		  printk("Yikes, we are supposed to read more than 8192 bytes on a RFT! sz=%ld\n", sz);
		  WRITE_COMMAND(port, RRES);
		}
	      else
		{
		ragain:

		  rbuf = &dev->fscc_rbuf[dev->current_rxbuf];
		  if (rbuf->valid == 1)
		    {
		      //oops we haave wrapped the receive buffers
		      //signal it and reuse;
		      rbuf->valid = 0;
		      rbuf->no_bytes = 0;
		      if (rbuf->fe == 1)
			{
			  atomic_dec(&dev->received_frames_pending);
			  atomic_dec(&dev->received_frames);
			}
		      rbuf->fe = 0;
		      dev->status |= ST_RBUF_OVERFLOW;
		    }

		  //check that sz will fit in to the buffer location
		  //printk("[%d]RFT:sz:%d, no_bytes:%d, max:%d\n",dev->current_rxbuf,sz,rbuf->no_bytes,dev->settings.n_rfsize_max);
		  if (sz + rbuf->no_bytes >= dev->settings.n_rfsize_max)
		    {
		      unsigned read_x_bytes;
		      read_x_bytes = (((dev->settings.n_rfsize_max - rbuf->no_bytes) / 4) * 4);
		      //fill up to n_rfsize_max, then go to the next one
		      READ_FIFO(port, (uint32_t *) (&rbuf->frame[rbuf->no_bytes]), read_x_bytes / 4);
		      sz -= read_x_bytes;
		      rbuf->valid = 1;
		      //buffer rollover here
		      //check to see if we are in a mode that has no "end"
		      //if so, then trigger an end of frame here to allow read to complete
		      //with every buffer completed.
		      //if not HDLC mode and no framesync and no termination bytes then..
		      //complete it here as if it had a frame end indication
		      if (((dev->settings.ccr0 & 0x3) != 0) && ((dev->settings.ccr0 & 0x70700) == 0))
			{
			  atomic_inc(&dev->received_frames_pending);
			  dev->status |= ST_RX_DONE;
			  rbuf->fe = 1;
			}

		      dev->current_frame_rbytes += read_x_bytes;
		      rbuf->no_bytes += read_x_bytes;
		      dev->current_rxbuf++;
		      if (dev->current_rxbuf == dev->settings.n_rbufs)
			{
			  dev->current_rxbuf = 0;
			}
		      if (sz >= 4)
			{
			  goto ragain;
			}
		    }
		  else
		    {
		      //get the data from the hardware
		      if (sz != 0)
			{
			  READ_FIFO(port, (uint32_t *) & rbuf->frame[rbuf->no_bytes], sz / 4);	//sz is in bytes, and we are reading dwords here (the count value is in DWORDS)
			}
		      //increment the numbytes in current frame received
		      rbuf->no_bytes += (sz / 4) * 4;
		      dev->current_frame_rbytes += (sz / 4) * 4;
		    }
		}

	    }			//end no full frame in fifo (RFT)
	}			//end bytes in fifo !=0
    }				//end use_dma==0 (irq mode)
  return 0;
}				//end empty_rx_fifo
