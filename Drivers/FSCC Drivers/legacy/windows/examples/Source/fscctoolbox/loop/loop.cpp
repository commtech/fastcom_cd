#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "..\fscctoolbox.h" /* user code header */


int main(int argc, char *argv[])
{
	DWORD nobytesread;
	char *tdata;
	char *rdata;
	int x,error,tosend;
	__int64 totalsent;
	__int64 totalread;
	__int64 totalerror;
	__int64 loop;
	ULONG status;
	ULONG size;
	int ret=0;
	unsigned tx_port=0;
	unsigned rx_port=0;
	DWORD returnstatus=0;
	
	if(argc!=4)
	{
		printf("usage:\n%s X Y size\n",argv[0]);
		printf("      X  tx port number\n");
		printf("      Y  rx port number\n");
		printf("      size is max packet size\n");
		
		exit(1);
	}
	size = atoi(argv[3]);
	if(size<1)
	{
		printf("size must be >1 :size=%d\n",size);
		exit(1);
	}
	srand( (unsigned)time( NULL ) );
	
	
	tx_port = atoi(argv[1]);
	rx_port = atoi(argv[2]);

	if((ret = FSCCToolBox_Create(tx_port))<0)
	{
		FSCCToolBox_GetLastError(tx_port,&returnstatus);
		printf("create tx port (%d)failed: 0x%8.8x\n",tx_port, returnstatus);
		exit(1);
	}
	if(tx_port != rx_port)
	{
		if((ret = FSCCToolBox_Create(rx_port))<0)
		{
			FSCCToolBox_GetLastError(rx_port,&returnstatus);
			printf("create rx port (%d) failed: 0x%8.8x\n",rx_port, returnstatus);
			exit(1);
		}
	}
	//printf("read overlapped event created\n");
	tdata = (char *)malloc(size+1);
	rdata = (char *)malloc(size+3);
	if((tdata==NULL)||(rdata==NULL))
	{
		printf("cannot allocate memory for buffers\n");
		if(tdata) free(tdata);
		if(rdata) free(rdata);
		FSCCToolBox_Destroy(tx_port);
		FSCCToolBox_Destroy(rx_port);
		exit(1);
		
	}
	//rDevice = wDevice;
	/* Flush the RX Descriptors so not as to have any complete descriptors in their
	* the first read in hdlc will get those left over frames and this test program
	* would not be of any use. */
	//printf("flush rx\n");
	FSCCToolBox_Flush_TX(tx_port);
	FSCCToolBox_Flush_RX(rx_port);
	
	totalerror=0;
	totalsent=0;
	totalread=0;
	loop=0;
	while(!kbhit())
	{
		error=0;
		tosend= (ULONG)(rand() % (size));
		//tosend = atoi(argv[3]);
		if(tosend==0) tosend=1;
		//printf("tosend               :%d\n",tosend);
		//generate a random string of our random length.
		
		for(x=0;x<tosend;x++) 
		{
			tdata[x]= (UCHAR)(rand());
		}
		
		//printf("pre-write\n");

		returnstatus = FSCCToolBox_Write_Frame(tx_port,&tdata[0],tosend,1000);//1 second timeout...adjust as necessary for bitrate!
		if(returnstatus != FSCCTOOLBOX_SUCCESS)
		{
			FSCCToolBox_GetLastError(tx_port,&returnstatus);
			printf("Error while writing: 0x%8.8x\n",returnstatus);
		}
		else
		{
			
			returnstatus = FSCCToolBox_Read_Frame(rx_port,&rdata[0],tosend+2,&nobytesread,1000);//1 second timeout...adjust as necessary for bitrate!
			if(returnstatus == READFILE_ERROR) // Check for errors
			{
				FSCCToolBox_GetLastError(rx_port,&returnstatus);
				printf("Error while reading: 0x%8.8x\n",returnstatus);
			}
			else
			{
				
				if(nobytesread!=(ULONG)tosend+2)
				{
					printf("Byte Count ERROR rec:%d != sent:%ld\n",nobytesread,tosend+2);
					error++;
					FSCCToolBox_Get_IStatus(rx_port,&status); // Clear the status
					if((status&0x00000200)!=0) printf("TDU!\n");
					FSCCToolBox_Flush_TX(tx_port);
					FSCCToolBox_Flush_RX(rx_port);	
					getch();
				}
				else if(nobytesread!=0)
				{
					if(nobytesread<32768)
						for(x=0;x<(int)nobytesread-2;x++)
							if((rdata[x]&0xff)!=(tdata[x]&0xff))	
							{
								//	printf("[%d] %x != %x\n",x,rdata[x]&0xff,tdata[x]&0xff);
								error++;
							}
				}
			}
		}
		//printf("Found: %d errors\n",error);
		totalerror +=error;
		loop++;
		totalsent+=tosend;
		totalread+=nobytesread;
		printf("loop:%15I64d :%d\n",loop,tosend);
		
}
getch();

printf("Found %I64d errors out of %I64d frames\n",totalerror,loop);
printf("Wrote %I64d bytes.\n",totalsent);
printf("Read %I64d bytes.\n",totalread-(loop*1));

if(tdata) free(tdata);
if(rdata) free(rdata);
FSCCToolBox_Destroy(tx_port); // Close the port
FSCCToolBox_Destroy(rx_port); // Close the port
return 0;
}
