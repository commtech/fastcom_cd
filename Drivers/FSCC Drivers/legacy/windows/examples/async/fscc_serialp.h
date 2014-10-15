#define IOCTL_SERIALP_GET_CONFIG_REG		0x001b00a0

#define IOCTL_SERIALP_GET_SAMPLE_RATE		0x001b00a4

#define IOCTL_SERIALP_GET_RX_TRIGGER		0x001b00a8

#define IOCTL_SERIALP_GET_TX_TRIGGER		0x001b00ac

#define IOCTL_SERIALP_GET_AUTO485			0x001b00b0

#define IOCTL_SERIALP_GET_4X				0x001b00b4

#define IOCTL_SERIALP_GET_CLOCK				0x001b00b8

#define IOCTL_SERIALP_SET_CONFIG_REG		0x001b00bc

#define IOCTL_SERIALP_SET_SAMPLE_RATE		0x001b00c0

#define IOCTL_SERIALP_SET_RX_TRIGGER		0x001b00c4

#define IOCTL_SERIALP_SET_TX_TRIGGER		0x001b00c8

#define IOCTL_SERIALP_SET_AUTO485			0x001b00cc

#define IOCTL_SERIALP_SET_4X				0x001b00d0

#define IOCTL_SERIALP_SET_CLOCK				0x001b00d4

#define IOCTL_SERIALP_SET_TX_WRITE_SIZE		0x001b00d8

#define IOCTL_SERIALP_GET_TX_WRITE_SIZE		0x001b00dc

#define IOCTL_SERIALP_GET_SOFT_9_BIT		0x001b00e0

#define IOCTL_SERIALP_SET_SOFT_9_BIT		0x001b00e4

#define IOCTL_SERIALP_SET_ISOSYNC		    0x001b00ec

#define IOCTL_SERIALP_GET_ISOSYNC  		    0x001b00f0

#define IOCTL_SERIALP_SET_CLK1XDTR			0x001b00f4

#define IOCTL_SERIALP_GET_CLK1XDTR			0x001b00f8

#define IOCTL_SERIALP_SET_EXT_COUNT			0x001b00fc

#define IOCTL_SERIALP_GET_EXT_COUNT			0x001b0100

#define IOCTL_SERIALP_SET_HARDWARE_RTSCTS	0x001b0104

#define IOCTL_SERIALP_GET_HARDWARE_RTSCTS		0x001b0108

/*
DeviceIoControl(cporth,IOCTL_SERIALP_GET_CLOCK,NULL,0,&Params->PortSettings.ClockRate,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_GET_4X,NULL,0,&Params->PortSettings.Clock4x,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_GET_AUTO485,NULL,0,&Params->PortSettings.Auto485,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_GET_TX_TRIGGER,NULL,0,&Params->PortSettings.TxTrigger,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_GET_RX_TRIGGER,NULL,0,&Params->PortSettings.RxTrigger,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_GET_SAMPLE_RATE,NULL,0,&Params->PortSettings.SampleRate,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_GET_CONFIG_REG,NULL,0,&configreg,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_GET_TX_WRITE_SIZE,NULL,0,&Params->PortSettings.TxWriteSize,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_GET_SOFT_9_BIT,NULL,0,&Params->PortSettings.Soft9Bit,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_GET_ISOSYNC,NULL,0,&Params->PortSettings.Isosync,sizeof(ULONG),&retsz,NULL);

DeviceIoControl(cporth,IOCTL_SERIALP_SET_4X,&Params->PortSettings.Clock4x,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_SET_ISOSYNC,&Params->PortSettings.Isosync,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_SET_AUTO485,&Params->PortSettings.Auto485,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_SET_TX_TRIGGER,&Params->PortSettings.TxTrigger,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_SET_RX_TRIGGER,&Params->PortSettings.RxTrigger,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_SET_SAMPLE_RATE,&Params->PortSettings.SampleRate,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_SET_TX_WRITE_SIZE,&Params->PortSettings.TxWriteSize,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);

configreg=0;
if(Params->PortSettings.Enable485==1) configreg|=0x2;
if(Params->PortSettings.RxEchoCancel==1) configreg|=0x4;
if(Params->PortSettings.enasync==1) configreg|=0x80000000;
DeviceIoControl(cporth,IOCTL_SERIALP_SET_CONFIG_REG,&configreg,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);

DeviceIoControl(cporth,IOCTL_SERIALP_SET_CLOCK,&Params->PortSettings.ClockRate,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);
DeviceIoControl(cporth,IOCTL_SERIALP_SET_SOFT_9_BIT,&Params->PortSettings.Soft9Bit,sizeof(ULONG),&ret,sizeof(ULONG),&retsz,NULL);

*/