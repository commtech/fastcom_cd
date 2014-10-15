//$Id$
//Fastcom API Interface for Applications
#ifndef _FCAPI_H_
#define _FCAPI_H_

#define FC_XONXOFF		0xF0
#define FC_RTSCTS		0xF1
#define FC_DTRDSR		0xF2
#define FC_NOHANDSHAKE	0xF3

typedef struct _COM_PORT_ENHANCED {
	unsigned char	TxFifoTriggerLevel;
	unsigned char	RxFifoTriggerLevel;
	unsigned long	TxWriteSize;
	unsigned long	Clock;
	unsigned char	LoopBack;
	unsigned long	Auto485;
	unsigned long	Auto485TxEn;
	unsigned long	RxEchoCancel;
	unsigned long	Sampling;
	unsigned char	rw;
	unsigned char	offset;
	unsigned char	data;
	unsigned char	DigiIOData;
	unsigned long	ninebit;
	unsigned long	SpeedCust;
	
} COM_PORT_ENHANCED,*PCOM_PORT_ENHANCED;


#define FILE_DEVICE_FCPORT  0x00008005


#define FCPORT_IOCTL_INDEX  0x805


#define IOCTL_FCPORT_SET_RS485			0x80052014

#define IOCTL_FASTCOM_SET_CLOCK			0x80052018

#define IOCTL_FCPORT_SET_485_TXEN		0x8005201c

#define IOCTL_FASTCOM_SET_RXECHO		0x80052020

#define IOCTL_FCPORT_SET_SAMPLING		0x80052024

#define IOCTL_FCPORT_RW_REG				0x80052028

#define IOCTL_FCPORT_SET_RXTX_TRIG		0x8005202c

#define IOCTL_FCPORT_SET_DIGI_OUT		0x80052030

#define IOCTL_FCPORT_GET_DIGI_IN		0x80052034

#define IOCTL_FCPORT_CURRENT_DIGI_OUT	0x80052038

#define IOCTL_FCPORT_SET_SOFT_9_BIT		0x8005203c

#define IOCTL_FCPORT_GET_SOFT_9_BIT		0x80052040

#define IOCTL_FCPORT_GET_ENHANCED		0x80052044

#define IOCTL_FCPORT_SET_SPEED_CUST		0x80052048

#endif  //_FCAPI_H_

