/* $Id$ */
/*
Copyright(c) 2007, Commtech, Inc.
rxmult.c -- user mode example to show use of IOCTL_FSCCDRV_SET_RECEIVE_MULTIPLE

usage:
  rxmult PORT NUM_RX 

  PORT can be any valid FSCC port (0,1) 
  NUM_RX is the number of receive frames to return at a time.

  Note it only makes sense to use this if you have a constant sized receive frame.
  The frames will be returned stacked back to back (with their associated receive status word), so
  if you expect to receive a constant stream of 100 byte HDLC frames, and you set the receive multiple to 10, each
  readfile will return 1020 bytes (10 frames * 102 bytes) layed out like:
  
  char frame1data[100]
  uint16 receivestatus1
  char frame2data[100]
  uint16 receivestatus2
  char frame3data[100]
  uint16 receivestatus3
  char frame4data[100]
  uint16 receivestatus4
  char frame5data[100]
  uint16 receivestatus5
  char frame6data[100]
  uint16 receivestatus6
  char frame7data[100]
  uint16 receivestatus7
  char frame8data[100]
  uint16 receivestatus8
  char frame9data[100]
  uint16 receivestatus9
  char frame10data[100]
  uint16 receivestatus10

  If you have random sized received frames, there is not any useful way to use this function (as there is not a way to identify where the second/third/forth etc frames would be located)
  and you should set the receive multiple to 1 in this case.


*/


#include <windows.h>
#include <stdio.h>
#include <math.h> /* floor, pow */
#include "..\..\fscc.h"



int main(int argc, char * argv[])
{
	char nbuf[80];
	int port,t;
	HANDLE hDevice; 
    unsigned long temp;
	unsigned long ret;
	
	if(argc<2) 
	{
		printf("usage: %s port numrx\n",argv[0]);
		exit(1);
	}
	
	port = atoi(argv[1]);
	sprintf(nbuf,"\\\\.\\FSCC%u",port);
	
	
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
	printf("Setting RX Multiple to:%ld on %s\n",atol(argv[2]),nbuf);
	temp = atol(argv[2]);
	if(temp==0)temp=1;
	t = DeviceIoControl(hDevice,IOCTL_FSCCDRV_SET_RECEIVE_MULTIPLE,&temp,sizeof(ULONG),NULL,0,&ret,NULL);
	printf("ReadFile() will now return %ld frames per call\n",temp);
	CloseHandle(hDevice);
	return 0;
}
/* $Id$ */