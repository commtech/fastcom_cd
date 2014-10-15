/******************************************************
 *
 * Copyright (C) 2000 Commtech, Inc. Wichita KS
 *
 * fsccdrv.h -- header file for fscc-pci module
 *
 * Tested with Linux version 2.2 12
 ******************************************************/

/* $Id$ */

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
#include <linux/types.h>
typedef __u32 uint32_t;
typedef struct fsccsetup{
	uint32_t fifot;
	uint32_t cmdr;
	uint32_t ccr0;
	uint32_t ccr1;
	uint32_t ccr2;
	uint32_t bgr;
	uint32_t ssr;
	uint32_t smr;
	uint32_t tsr;
	uint32_t tmr;
	uint32_t rar;
	uint32_t ramr;
	uint32_t ppr;
	uint32_t tcr;
	uint32_t imr;
	uint32_t n_rbufs;
	uint32_t n_tbufs;
	uint32_t n_rfsize_max;
	uint32_t n_tfsize_max;

} setup;


typedef struct fscc_regsingle{
	uint32_t port;		//offset from base address of register to access
	uint32_t data;		//data to write
}regsingle;



//new ics307-03 stuff
struct clockstruct{
unsigned long freq;
char progword[20];
};

typedef unsigned long status_t;
typedef struct clockstruct myclk;
 /* Ioctl definitions
 */

/* Use 'k' as magic number */
#define FSCC_IOC_MAGIC  'k'
#define FSCC_INIT           _IOW(FSCC_IOC_MAGIC,  1, setup)
#define FSCC_READ_REGISTER  _IOWR(FSCC_IOC_MAGIC,   2, regsingle)
#define FSCC_WRITE_REGISTER _IOWR(FSCC_IOC_MAGIC,   3, regsingle)
#define FSCC_STATUS         _IOR(FSCC_IOC_MAGIC,  4, status_t)
#define FSCC_ADD_BOARD      _IOR(FSCC_IOC_MAGIC,  5, board_settings)
#define FSCC_TX_ACTIVE      _IOR(FSCC_IOC_MAGIC, 6, status_t)
#define FSCC_RX_READY       _IOR(FSCC_IOC_MAGIC,7, status_t)
#define FSCC_SETUP  ESCC_INIT
#define FSCC_FLUSH_RX       _IO(FSCC_IOC_MAGIC,12)
#define FSCC_FLUSH_TX       _IO(FSCC_IOC_MAGIC,13)
#define FSCC_IMMEDIATE_STATUS _IOR(FSCC_IOC_MAGIC,18,status_t)

#define FSCC_SET_FREQ   _IOW(FSCC_IOC_MAGIC,22,myclk)
#define FSCC_GET_FREQ   _IOR(FSCC_IOC_MAGIC,23,status_t)
#define FSCC_SET_FEATURES   _IOW(FSCC_IOC_MAGIC,24,status_t)
#define FSCC_GET_FEATURES   _IOR(FSCC_IOC_MAGIC,25,status_t)

#define FSCC_TIMED_TRANSMIT _IOW(FSCC_IOC_MAGIC,26,status_t)
#define FSCC_TRANSMIT_REPEAT _IOW(FSCC_IOC_MAGIC,27,status_t)
#define FSCC_WRITE_REGISTER2 _IOW(FSCC_IOC_MAGIC,28,regsingle)
#define FSCC_READ_REGISTER2  _IOR(FSCC_IOC_MAGIC,29,regsingle)
#define FSCC_WRITE_REGISTER3 _IOW(FSCC_IOC_MAGIC,30,regsingle)
#define FSCC_READ_REGISTER3  _IOR(FSCC_IOC_MAGIC,31,regsingle)
#define FSCC_DUMP_RDESC    _IO(FSCC_IOC_MAGIC,32)
#define FSCC_DUMP_TDESC    _IO(FSCC_IOC_MAGIC,33)
#define FSCC_TRANSMIT_DELAY  _IOW(FSCC_IOC_MAGIC,34,status_t)
#define FSCC_TRANSMIT_IRQ_RATE _IOW(FSCC_IOC_MAGIC,35,status_t)
#define FSCC_RECEIVE_IRQ_RATE _IOW(FSCC_IOC_MAGIC,36,status_t)
#define FSCC_ALLOW_READ_CUTOFF _IOW(FSCC_IOC_MAGIC,37,status_t)
#define FSCC_EXTERNAL_TRANSMIT _IOW(FSCC_IOC_MAGIC,38,status_t)

#define FSCC_IOC_MAXNR 38


//status messages
#define ST_CTSA					0x00000001
#define ST_CDC					0x00000002
#define ST_DSRC					0x00000004
#define ST_CTSS					0x00000008
#define ST_TIN					0x00000010
#define ST_RFS					0x00000020
#define ST_RFL					0x00000040
#define ST_RDO					0x00000080
#define ST_RFO					0x00000100
#define ST_TDU					0x00000200
#define ST_OVF					0x00000400
#define ST_RX_DONE				0x00000800
#define ST_ALLS					0x00001000
#define ST_TX_DONE				0x00002000
#define ST_RFT					0x00004000
#define ST_TFT					0x00008000
#define ST_RFE					0x00010000
#define ST_RBUF_OVERFLOW		0x00020000
#define ST_DMAT_S				0x00040000
#define ST_DMAR_S				0x00080000
#define ST_TX_HI                                0x00100000
#define ST_RX_HI                                0x00200000




