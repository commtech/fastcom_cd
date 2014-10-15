/*
	header file for Fastcom:FSCC
	fscc.h
*/
#define  IOCTL_FSCCDRV_TX_ACTIVE			0x830020C0
#define  IOCTL_FSCCDRV_RX_READY				0x830020C4
#define  IOCTL_FSCCDRV_SETUP				0x830020C8
#define  IOCTL_FSCCDRV_STATUS				0x830020CC

#define  IOCTL_FSCCDRV_FLUSH_RX				0x830020E0
#define  IOCTL_FSCCDRV_FLUSH_TX				0x830020E4

#define  IOCTL_FSCCDRV_READ_REGISTER		0x830020F8
#define  IOCTL_FSCCDRV_WRITE_REGISTER		0x830020FC
#define  IOCTL_FSCCDRV_IMMEDIATE_STATUS		0x83002100

#define  IOCTL_FSCCDRV_WRITE_REGISTER2		0x83002104
#define  IOCTL_FSCCDRV_READ_REGISTER3		0x83002108
#define  IOCTL_FSCCDRV_WRITE_REGISTER3		0x8300210c
#define  IOCTL_FSCCDRV_READ_REGISTER2		0x83002110  


#define  IOCTL_FSCCDRV_CANCEL_RX			0x83002134
#define  IOCTL_FSCCDRV_CANCEL_TX			0x83002138
#define  IOCTL_FSCCDRV_CANCEL_STATUS		0x8300213C

#define IOCTL_FSCCDRV_SET_FREQ				0x83002178
#define IOCTL_FSCCDRV_SET_FEATURES			0x8300217C
#define IOCTL_FSCCDRV_GET_FEATURES			0x83002180
#define IOCTL_FSCCDRV_GET_FREQ				0x83002184

#define  IOCTL_FSCCDRV_BLOCK_MULTIPLE_IO	0x83002188
#define  IOCTL_FSCCDRV_GET_SETUP			0x83002194


#define  IOCTL_FSCCDRV_SET_FREQ2			0x83002198
//#define  IOCTL_FSCCDRV_GET_FREQ2			0x8300219C// Do not use

#define  IOCTL_FSCCDRV_TIMED_TRANSMIT		0x830021a0
#define  IOCTL_FSCCDRV_TRANSMIT_REPEAT		0x830021a4
#define  IOCTL_FSCCDRV_DELAY_WRITE_START	0x830021a8
#define IOCTL_FSCCDRV_ALLOW_READ_CUTOFF     0x830021ac
#define IOCTL_FSCCDRV_SET_RX_IRQ_RATE       0x830021b0
#define IOCTL_FSCCDRV_SET_TX_IRQ_RATE       0x830021b4
#define IOCTL_FSCCDRV_SET_DMA               0x830021b8
#define IOCTL_FSCCDRV_SET_RECEIVE_MULTIPLE	0x830021bc
#define IOCTL_FSCCDRV_GET_RECEIVE_MULTIPLE	0x830021c0
#define IOCTL_FSCCDRV_SET_CHARACTER_MAP_ENABLE	0x830021c4
#define IOCTL_FSCCDRV_GET_CHARACTER_MAP_ENABLE	0x830021c8
#define IOCTL_FSCCDRV_SET_CHARACTER_MAP		0x830021cc
#define IOCTL_FSCCDRV_GET_CHARACTER_MAP		0x830021d0
#define IOCTL_FSCCDRV_SET_UCHARACTER_MAP	0x830021d4
#define IOCTL_FSCCDRV_GET_UCHARACTER_MAP	0x830021d8
#define IOCTL_FSCCDRV_EXTERNAL_TRANSMIT		0x830021dc
#define  IOCTL_FSCCDRV_GET_TIMED_TRANSMIT   0x830021e0
#define  IOCTL_FSCCDRV_GET_TRANSMIT_REPEAT  0x830021e4
#define  IOCTL_FSCCDRV_GET_EXTERNAL_TRANSMIT 0x830021e8  




#define ULONG DWORD

typedef struct setup{
	ULONG fifot;
	ULONG cmdr;
	ULONG ccr0;
	ULONG ccr1;
	ULONG ccr2;
	ULONG bgr;
	ULONG ssr;
	ULONG smr;
	ULONG tsr;
	ULONG tmr;
	ULONG rar;
	ULONG ramr;
	ULONG ppr;
	ULONG tcr;
	ULONG imr;
	ULONG n_rbufs;
	ULONG n_tbufs;
	ULONG n_rfsize_max;
	ULONG n_tfsize_max;

} SETUP;

//These status message defines are mapped from the ISR.  Unless noted, each status 
//message correlates to a bit found in the Interrupt Status Register.

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
//reserved						0x00000400
#define ST_RX_DONE				0x00000800
//This occurs when the driver flags a read buffer as complete.
//This is the trigger that will unblock a pending read.

#define ST_ALLS					0x00001000
#define ST_TX_DONE				0x00002000
//When the driver finishes pushing a Writefile to the card's TxFIFO.
//This is the trigger that will unblock a pending write.

#define ST_RFT					0x00004000
#define ST_TFT					0x00008000
#define ST_RFE					0x00010000
#define ST_RBUF_OVERFLOW		0x00020000
//When all rbufs in the driver are full and you receive another frame.




//FSCC register offsets

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



//Register bit defines

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
#define VST    0xffffffff

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


#define DMA_TSTOP 0x00008000
#define DMA_RSTOP 0x00004000
#define DMA_TFE   0x00002000
#define DMA_RFE   0x00001000
#define DMA_THI   0x00000800
#define DMA_RHI   0x00000400

