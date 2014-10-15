/* $Id$ */
/*
Copyright(c) 2007, Commtech, Inc.
setumap.c -- user mode function to set the user character map


usage:
 mapping port map 

 The port can be any valid fscc port (0,1)

 map -character mapping value

  User defined map value, as 4 distinct bytes;
  (map)&0xff
  (map>>8)&0xff
  (map>>16)&0xff
  (map>>24)&0xff
  
  each byte of the DWORD becomes a character map value.
  When any of these bytes are seen in the data stream they are replaced
  with the sequence 0x7D, (value&0xDF)|((value&0x20)?0x00:0x20).
  This is the same sequence as for the control character mapping
  but extends the concept to an arbitrary set of byte values.
  
  If unused the value should be set to 0x7E7E7E7E
  As the 0x7E (flag sequence) must be escaped, (or not used) in this mode

  

*/


#include <windows.h>
#include <stdio.h>
#include <math.h> /* floor, pow */
#include "..\fscc.h"


int main(int argc, char * argv[])
{
	//clkset clock;
	char nbuf[80];
	int port;
	HANDLE hDevice; 
	unsigned long temp;
	ULONG map;


	if(argc<3) 
		{
		printf("usage:\r\n");
		printf("%s port mapvalue\r\n",argv[0]);
		printf("\r\n");
		printf("The port can be any valid fscc port (0,1)\r\n");
		printf("\r\n");
		printf("mapvalue is the character map as a hex value\r\n");
		printf("\r\n");
		exit(1);
		}

	port = atoi(argv[1]);
	sscanf(argv[2],"%x",&map);
	
    sprintf(nbuf,"\\\\.\\FSCC%u",port);

	printf("Opening: %s\n",nbuf);

	if((hDevice = CreateFile (
			nbuf, 
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
			NULL)) == INVALID_HANDLE_VALUE ) 
	{
                printf("Can't get a handle to fsccpdrv @ %s\n",nbuf);
		exit(1);
	}



if(DeviceIoControl(hDevice,IOCTL_FSCCDRV_SET_UCHARACTER_MAP,&map,sizeof(ULONG),NULL,0,&temp,NULL))
	{
	UCHAR value;
	value = (UCHAR)(map&0xff);
	printf("%x is mapped to 0x7D,%x\n",value,(value&0xDF)|((value&0x20)?0x00:0x20));
    value = (UCHAR)((map>>8)&0xff);
	printf("%x is mapped to 0x7D,%x\n",value,(value&0xDF)|((value&0x20)?0x00:0x20));
    value = (UCHAR)((map>>16)&0xff);
	printf("%x is mapped to 0x7D,%x\n",value,(value&0xDF)|((value&0x20)?0x00:0x20));
    value = (UCHAR)((map>>24)&0xff);
	printf("%x is mapped to 0x7D,%x\n",value,(value&0xDF)|((value&0x20)?0x00:0x20));	
	}
else printf("failed:%8.8x\r\n",GetLastError());

CloseHandle(hDevice);
	return 0;
}



/* $Id$ */