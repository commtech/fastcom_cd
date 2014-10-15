/******************************************************
 *
 * Copyright (C) 2000 Commtech, Inc. Wichita KS
 *
 * fsccdrv.h -- header file for fscc-pci module
 *
 * Tested with Linux version 2.2 12
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
#include <linux/poll.h>
#include <linux/ioctl.h>

/* version dependencies have been confined to a separate file */

#define VERSION_CODE(vers,rel,seq) ( ((vers)<<16) | ((rel)<<8) | (seq) )
#include "sysdep.h"

//define if using 2.6 kernel series.--possibly rework with VERSION_CODE, is version code still there in 2.6?
#if  LINUX_VERSION_CODE >= VERSION_CODE(2,6,0)
#define USE_2_6
#include <linux/interrupt.h>
#endif

//for debug messages
//#define FSCC_DEBUG

//for fsccpdrv entry in /proc
#define FSCC_USE_PROC
// if enabled it will use V3 busmaster DMA to do data transfers
#define USE_DMA

//if defined will make read return 0 after 2 timeout cycles (rqtimeout) 
//not defined read will block until data comes in or signaled
#define ALLOW_READ_TIMEOUT

/*
 * Macros to help debugging
 */

#undef PDEBUG			/* undef it, just in case */
#ifdef FSCC_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_INFO "\nfsccp: " fmt,## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...)	/* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...)	/* nothing: it's a placeholder */

#ifndef FSCC_MAJOR		//dynamic is broken on 2.6? giving 0 doesn't work anymore
#ifdef USE_2_6
#define FSCC_MAJOR 0		//you will likely have to change this for your system
#else
#define FSCC_MAJOR 0		/* dynamic major by default */
#endif
#endif



#define COMMTECH_VENDOR_ID 0x18f7
#define FSCC_DEVID 0x000f
#define SFSCC_DEVID 0x0014
#define FSCC232_DEVID 0x0016
#define SFSCC104_NOUA_DEVID 0x0017
#define SFSCC_4_DEVID 0x0018
#define SFSCC104_DEVID 0x0019
#define SFSCCLVDS_DEVID 0x001a
#define FSCC_4_DEVID 0x001b
#define SFSCC_4_LVDS_DEVID 0x001c
#define SFSCCe_4_DEVID 0x001e

#ifndef MAX_FSCC_BOARDS
#define MAX_FSCC_BOARDS 6	//edit the mkdev.sh script variable default_ports to correspond with this number
#endif
#define MAX_TIMEOUT_VALUE 1000000	//max value for WAIT_WITH_TIMEOUT

#define TXFIFOSIZE 4096L
#define RXFIFOSIZE 8192L

#define READ_PORT_ULONG(port) inl(port)
#define READ_PORT_BUFFER_ULONG(port,buf,count) insl(port,buf,count)
#define WRITE_PORT_ULONG(port,val) outl(val,port)
#define WRITE_PORT_BUFFER_ULONG(port,buf,count) outsl(port,buf,count)
/*
 */

typedef struct fsccsetup
{
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
//structure for buffered frames
struct buf
{
  unsigned valid;		//indicator 1 = frame[] has data, 0 = frame[] has ???
  unsigned no_bytes;		//number of bytes in frame[]
  unsigned max;			//maximum number of bytes to send/receive
  unsigned fe;			//frame end
  unsigned ftot;
  unsigned char *frame;		//data array for received/transmitted data
};




typedef struct fscc_regsingle
{
  uint32_t port;		//offset from base address of register to access
  uint32_t data;		//data to write
} regsingle;

typedef struct desc
{
  volatile uint32_t control;	//0
  volatile uint32_t data_address;	//4
  volatile uint32_t data_count;	//8
  volatile uint32_t next_desc;	//12
  volatile uint32_t pad;	//16 - this is here to keep descriptors from spanning physical pages in memory
} DESC;

typedef struct Fscc_Dev
{
  struct pci_dev *pcidev;
  setup settings;
  uint32_t fsccbase;
  uint32_t uartbase;
  uint32_t bsrbase;
  unsigned irq;
  unsigned channel;
  unsigned board;		//holds board number (ch0 and  ch1 have same board number)
  //this is used to lock access to the board if 
  //both channels access at the same time
  //in conjunction with the global atomic_t boardlock[] and 
  //the per device wboardlock wait queue 
  unsigned dev_no;		//only needed to check against previous in addboard
//add other per port info here
  struct Fscc_Dev *complement_dev;	//pointer to dev of other channel on this board

  unsigned tx_type;
//state flags
  unsigned irq_hooked;
  unsigned port_initialized;

//interrupt status flags
  unsigned long status;

//wait queues
#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)
  wait_queue_head_t rq;
  wait_queue_head_t wq;
  wait_queue_head_t sq;
#else
  struct wait_queue *rq;
  struct wait_queue *wq;
  struct wait_queue *sq;
#endif
#if LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)
  wait_queue_head_t fscc_fsync_wait;
#else
  struct wait_queue *fscc_fsync_wait;
#endif


//buffering params

  struct buf *fscc_rbuf;
  struct buf *fscc_tbuf;
  unsigned current_rxbuf;
  unsigned current_rxbuf_out;
  unsigned current_txbuf;
  unsigned is_transmitting;
  atomic_t received_frames_pending;
  atomic_t transmit_frames_available;
//dma driven buffering
  DESC **Txbdalist;
  DESC **Rxbdalist;
  dma_addr_t *phys_txbdalist;
  dma_addr_t *phys_rxbdalist;
  unsigned char **rxdata;
  unsigned char **txdata;
  dma_addr_t *phys_rxdata;
  dma_addr_t *phys_txdata;

  unsigned long rirqrate;
  unsigned long tirqrate;
  unsigned long fscc_u_count;
  uid_t fscc_u_owner;
  unsigned long freq;
  unsigned long ppm;
  unsigned long features;
  unsigned long timed_transmit;
  unsigned long transmit_repeat;
  unsigned long external_transmit;
  unsigned long current_frame_rbytes;
  atomic_t received_frames;
  unsigned bits_going_out;
  spinlock_t irqlock;
  unsigned long usedma;

  struct timer_list rqt;
  struct timer_list wqt;
  unsigned long rqtimeout;
  unsigned long wqtimeout;

  unsigned long transmit_delay;
  int rblockcnt;
  unsigned long allowcutoff;
  unsigned long rdtotal;
} Fscc_Dev;

//new ics307-03 stuff
struct clockstruct
{
  unsigned long freq;
  char progword[20];
};


/*
 * Split minors in two parts
 */
#define TYPE(dev)   (MINOR(dev) >> 4)	/* high nibble */
#define NUM(dev)    (MINOR(dev) & 0xf)	/* low  nibble */

/*
 * Different minors behave differently, so let's use multiple fops
 */

extern struct file_operations fscc_fops;	/* simplest: global */

/*
 * The different configurable parameters
 */
extern int fscc_major;		/* main.c */
extern int fscc_nr_devs;
extern unsigned long status;
extern Fscc_Dev *fscc_devices;
extern int used_board_numbers[MAX_FSCC_BOARDS];
extern struct semaphore boardlock[MAX_FSCC_BOARDS];
extern int boardtype[MAX_FSCC_BOARDS];

/*
 * Prototypes for shared functions
 */



ssize_t fscc_read(struct file *filp, char *buf, size_t count, loff_t * off);
ssize_t fscc_write(struct file *filp, const char *buf, size_t count, loff_t * off);
loff_t fscc_lseek(struct file *filp, loff_t off, int whence);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 11))
long fscc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#else
int fscc_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#endif
int fscc_open(struct inode *inode, struct file *filp);
int fscc_release(struct inode *inode, struct file *filp);

#if LINUX_VERSION_CODE >= VERSION_CODE(2,6,35)
int fscc_fsync(struct file *filp, int datasync);
#elif LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)
int fscc_fsync(struct file *filp, struct dentry *de, int datasync);
#else
int fscc_fsync(struct file *filp, struct dentry *de);
#endif

unsigned int fscc_poll(struct file *filp, poll_table * pt);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
irqreturn_t fscc_irq(int irq, void *dev_id);
#else
irqreturn_t fscc_irq(int irq, void *dev_id, struct pt_regs *regs);
#endif
int add_port(Fscc_Dev * dev);
void convertlsb2msb(unsigned char *data, unsigned size);
uint64_t get__tsc(void);
int SetICS30703Bits(uint32_t port, unsigned char *progdata);
int SetICS30703Bits2(uint32_t port, unsigned char *progdata);
int fscc_tx_start(Fscc_Dev * dev);
int fill_tx_fifo(Fscc_Dev * dev);
int empty_rx_fifo(Fscc_Dev * dev);

//dma support routines
//

void enable_dma(Fscc_Dev * dev);
void stop_dma(Fscc_Dev * dev);
void init_rx_dma(Fscc_Dev * dev);
void init_tx_desc(Fscc_Dev * dev);
void init_rx_desc(Fscc_Dev * dev);
void cleanup_allocs(Fscc_Dev * dev);
int allocate_desc(Fscc_Dev * dev);
int fscc_start_tx_dma(Fscc_Dev * dev);


void setup_tmr(Fscc_Dev * ptr);
void wsetup_tmr(Fscc_Dev * ptr);

void check_rq_timeout(unsigned long ptr);
void check_wq_timeout(unsigned long ptr);

int get_t_empty(Fscc_Dev * dev, int count);
int get_numt_empty(Fscc_Dev * dev);


#ifndef min
#  define min(a,b) ((a)<(b) ? (a) : (b))
#endif
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
#define FSCC_TRANSMIT_DELAY _IOW(FSCC_IOC_MAGIC,34,status_t)
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
#define ST_RBUF_OVERFLOW			0x00020000
#define ST_DMAT_S				0x00040000
#define ST_DMAR_S				0x00080000
#define ST_TX_HI                                0x00100000
#define ST_RX_HI                                0x00200000



uint32_t GET_RI_STATE(uint32_t port);
uint32_t GET_DSR_STATE(uint32_t port);
uint32_t GET_CTS_STATE(uint32_t port);
uint32_t GET_CD_STATE(uint32_t port);
uint32_t READ_GLOBAL_INTERRUPT_STATUS(uint32_t port);
void WRITE_INTERRUPT_MASK(uint32_t port, uint32_t imr);
uint32_t READ_INTERRUPT_STATUS(uint32_t port);
uint32_t READ_VERSION(uint32_t port);
void WRITE_TIMER_CONTROL(uint32_t port, uint32_t tcr);
uint32_t READ_TIMER_CONTROL(uint32_t port);
void WRITE_PREAMBLE(uint32_t port, uint32_t ppr);
uint32_t READ_PREAMBLE(uint32_t port);
void WRITE_RECEIVE_ADDRESS_MASK(uint32_t port, uint32_t ramr);
uint32_t READ_RECEIVE_ADDRESS_MASK(uint32_t port);
void WRITE_RECEIVE_ADDRESS(uint32_t port, uint32_t rar);
uint32_t READ_RECEIVE_ADDRESS(uint32_t port);
void WRITE_TERMINATION_SEQUENCE_MASK(uint32_t port, uint32_t tmr);
uint32_t READ_TERMINATION_SEQUENCE_MASK(uint32_t port);
void WRITE_TERMINATION_SEQUENCE(uint32_t port, uint32_t tsr);
uint32_t READ_TERMINATION_SEQUENCE(uint32_t port);
void WRITE_SYNC_SEQUENCE_MASK(uint32_t port, uint32_t smr);
uint32_t READ_SYNC_SEQUENCE_MASK(uint32_t port);
void WRITE_SYNC_SEQUENCE(uint32_t port, uint32_t ssr);
uint32_t READ_SYNC_SEQUENCE(uint32_t port);
void WRITE_BGR(uint32_t port, uint32_t bgr);
uint32_t READ_BGR(uint32_t port);
void WRITE_CCR2(uint32_t port, uint32_t ccr2);
uint32_t READ_CCR2(uint32_t port);
void WRITE_CCR1(uint32_t port, uint32_t ccr1);
uint32_t READ_CCR1(uint32_t port);
void WRITE_CCR0(uint32_t port, uint32_t ccr0);
uint32_t READ_CCR0(uint32_t port);
uint32_t READ_STATUS(uint32_t port);
uint32_t WRITE_COMMAND(uint32_t port, uint32_t cmd);
uint32_t READ_TX_FRAME_COUNT(uint32_t port);
uint32_t READ_RX_FRAME_COUNT(uint32_t port);
uint32_t READ_TX_FIFO_BYTE_COUNT(uint32_t port);
uint32_t READ_RX_FIFO_BYTE_COUNT(uint32_t port);
void WRITE_TX_TRIGGER(uint32_t port, uint32_t trig);
uint32_t READ_TX_TRIGGER(uint32_t port);
void WRITE_RX_TRIGGER(uint32_t port, uint32_t trig);
uint32_t READ_RX_TRIGGER(uint32_t port);
uint32_t READ_RX_COUNT(uint32_t port);
void WRITE_TX_COUNT(uint32_t port, uint32_t count);
void WRITE_FIFO(uint32_t port, uint32_t * buf, uint32_t count);
void READ_FIFO(uint32_t port, uint32_t * buf, uint32_t count);
uint32_t READ_ANY(uint32_t port);
void WRITE_ANY(uint32_t port, uint32_t val);




#define FIFO   0x00
#define TXBCR  0x04
#define RXBCR  0x04
#define FTRIG  0x08
#define FBCR   0x0c
#define FFCR   0x10
#define CMDR   0x14
#define STAR   0x18
#define CCR0   0x1c
#define CCR1   0x20
#define CCR2   0x24
#define BGR    0x28
#define SSR    0x2c
#define SMR    0x30
#define TSR    0x34
#define TMR    0x38
#define RAR    0x3c
#define RAMR   0x40
#define PPR    0x44
#define TCR    0x48
#define VSTR   0x4c
#define ISR    0x50
#define IMR    0x54
#define GIS    0x58

//RXTRIG is bits 12:0 of FTRIG
#define RXTRIG  0x00001fff
#define TXTRIG  0x0fff0000

//#define RXTRIG(a) a & 0x00001fff
//TXTRIG is bits 27:16 of FTRIG
//#define TXTRIG(a) ( a >> 16 ) & 0x00000fff

//RXCNT is bits 13:0 of FBCR
#define RXCNT   0x00003fff
#define TXCNT   0x1fff0000

//#define RXCNT(a)  a & 0x00003fff
//TXCNT is bits 28:16 of FBCR
//#define TXCNT(a) (a >> 16) & 0x00001fff

//RFCNT is bits 9:0 of FFCR
#define RFCNT   0x000003ff
#define TFCNT   0x01ff0000

//#define RFCNT(a) a & 0x000003ff
//TFCNT is bits 24:16 of FFCR
//#define TFCNT(a) (a >> 16) &0x000001ff





//CMDR defines
#define TIMR   0x00000001
#define STIMR  0x00000002
#define HUNT   0x00010000
#define RRES   0x00020000
#define XF     0x01000000
#define XREP   0x02000000
#define SXREP  0x04000000
#define TRES   0x08000000
#define TXT    0x10000000
#define TXEXT  0x20000000
//STAR defines
#define CTS    0x00000001
#define DSR    0x00000002
#define CD     0x00000004
#define RI     0x00000008
#define ABF    0x00000100
#define CRCS   0x00000200
#define RLEX   0x00000400
#define VFR    0x00000800
#define RFLS   0x00001000
#define RDOS   0x00002000
#define TFE    0x00010000
#define TBC    0x00020000
#define CE     0x00040000
#define DPLLA  0x00080000
#define XREPA  0x00100000
#define TXTA   0x00200000
#define TDO    0x01000000

//CCR0 defines
#define MODE   0x00000003
#define CM     0x0000001c
#define LE     0x000000e0
#define FSC    0x00000700
#define SFLAG  0x00000800
#define ITF    0x00001000
#define NSB    0x0000e000
#define NTB    0x00070000
#define VIS    0x00080000
#define CRC    0x00300000
#define OBT    0x00400000
#define ADM    0x01800000
#define RECD   0x02000000

//CCR1 defines
#define RTS    0x00000001
#define DTR    0x00000002
#define RTSC   0x00000004
#define CTSC   0x00000008
#define ZINS   0x00000010
#define OINS   0x00000020
#define DPS    0x00000040
#define SYNC2F 0x00000080
#define TERM2F 0x00000100
#define ADD2F  0x00000200
#define CRC2F  0x00000400
#define CRCR   0x00000800
#define DRCRC  0x00001000
#define DTCRC  0x00002000
#define DTERM  0x00004000
#define RIP    0x00010000
#define CDP    0x00020000
#define RTSP   0x00040000
#define CTSP   0x00080000
#define DTRP   0x00100000
#define DSRP   0x00200000
#define FSRP   0x00400000
#define FSTP   0x00800000
#define TCOP   0x01000000
#define TCIP   0x02000000
#define RCP    0x04000000
#define TDP    0x08000000
#define RDP    0x10000000

//CCR2 defines
#define FSRO   0x0000000f
#define FSTO   0x000000f0
#define RLC    0xffff0000

//SSR
#define SYNC1  0x000000ff
#define SYNC2  0x0000ff00
#define SYNC3  0x00ff0000
#define SYNC4  0xff000000

//SMR
#define SMASK1 0x000000ff
#define SMASK2 0x0000ff00
#define SMASK3 0x00ff0000
#define SMASK4 0xff000000

//TSR 
#define TERM1  0x000000ff
#define TERM2  0x0000ff00
#define TERM3  0x00ff0000
#define TERM4  0xff000000

//TMR
#define TMASK1 0x000000ff
#define TMASK2 0x0000ff00
#define TMASK3 0x00ff0000
#define TMASK4 0xff000000

//RAR
#define GADDRL 0x000000ff
#define GADDRH 0x0000ff00
#define SADDRL 0x00ff0000
#define SADDRH 0xff000000

//RAMR
#define GMASKL 0x000000ff
#define GMASKH 0x0000ff00
#define SMASKL 0x00ff0000
#define SMASKH 0xff000000

//PPR
#define POST   0x000000ff
#define NPOST  0x0000ff00
#define PRE    0x00ff0000
#define NPRE   0xff000000

//TCR
#define TSRC   0x00000003
#define TTRIG  0x00000004
#define TCNT   0xfffffff8

//VSTR
#define VST    0x0000ffff

//ISR
#define RFS    0x00000001
#define RFT    0x00000002
#define RFE    0x00000004
#define RFO    0x00000008
#define RDO    0x00000010
#define RFL    0x00000020
#define TIN    0x00000100
#define TFT    0x00010000
#define ALLS   0x00020000
#define TDU    0x00040000
#define CTSS   0x01000000
#define DSRC   0x02000000
#define CDC    0x04000000
#define CTSA   0x08000000
