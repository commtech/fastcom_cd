/* $Id$ */
/*
Copyright(c) 2007, Commtech, Inc.
setmap.c -- user mode function to set the character map


usage:
 mapping port map 

 The port can be any valid fscc port (0,1)

 map -character mapping value

  each bit in the map (32 bits) represents a control character to map.
  a '1' in a bit position will indicate that the driver should map that 
  control character, adding a 0x7d and inverting bit 5.
  This allows for the mapping of the values 0x00 through 0x1F 
  which if enabled will map to the values 0x20-0x3f (with the extra 0x7D)


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
	ULONG  desreg;
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



if(DeviceIoControl(hDevice,IOCTL_FSCCDRV_SET_CHARACTER_MAP,&map,sizeof(ULONG),NULL,0,&temp,NULL))
	{
	ULONG i;
	int j;
	printf("map:%x\n",map);
	i=1;
	for(j=0;j<32;j++)
	{
	if((map&i)==i) printf("%x mapped to 0x7d,0x%x\n",j,j|0x20);
	i=i<<1;
	}
	}
else printf("failed:%8.8x\r\n",GetLastError());

CloseHandle(hDevice);
	return 0;
}



/* $Id$ */