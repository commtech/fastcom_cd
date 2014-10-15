/*
Copyright(c) 2009, Commtech, Inc.
readfile.c -- user mode function to read to a file from the FSCC port using the fscctoolbox.dll

  To use the fscctoolbox.DLL, you must do three things:
  
	1. #include fscctoolbox.h in your source code
	2. Add the file fscctoolbox.lib to your linker's Output/Library modules.  To do this in
	Visual Studio, Click on Settings in the Project menu.  In the Project settings dialog
	click on the Link tab.  In the Category drop down, choose General.  In the 
	Output/library modules box, add the path to fscctoolbox.lib.  It is easiest to copy the 
	file to your working directory and just add "fscctoolbox.lib"
	3. The fscctoolbox.dll file must reside in the same directory as your compiled program .exe
	
	  usage:
	  read port filetowrite
			port - can be any valid fscc port (0,1,2...).
			filetowrite - The file to write to
		
		  
			
*/


#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include "..\fscctoolbox.h" /* user code header */
#define FRAMESIZE 4096


int main(int argc, char *argv[])
{
	int port,ret,numwritten;
	FILE *fout;
	ULONG frame,bytes,sz;
	DWORD status;
	unsigned char buf[FRAMESIZE];
	
	if(argc!=3)
	{
		printf("%s port file\n\n",argv[0]);
		printf("\tPort is appended onto \\\\.\\FSCC\n");
		exit(1);
	}
	
	port = atoi(argv[1]);
	if((ret = FSCCToolBox_Create(port))<0) // Open the port
	{
		printf("Created:%d\n",ret);
		exit(1);
	}
	
	fout = fopen(argv[2],"wb");  // Open the file
	if(fout==NULL)
	{
		printf("Cannot open %s\n",argv[2]);
		exit(1);
	}
	
	FSCCToolBox_Get_IStatus(port,&status); // Clear the status
	frame = 0;
	bytes = 0;
	memset( &buf, 0, sizeof( buf ) );
	printf("Press any key to stop reading.\n");
	while(!kbhit())
	{
		ret = FSCCToolBox_Read_Frame(port,&buf[0],FRAMESIZE,&sz,1000);//1 second timeout...adjust as necessary for bitrate!
		if(ret == READFILE_ERROR) // Check for errors
		{
			FSCCToolBox_GetLastError(port,&status);
			printf("Error while reading: 0x%8.8x\n",status);
		}
		frame++; // Increment the number of frames
		bytes += sz;
		numwritten = fwrite( buf, 1, sz, fout ); // Write to a file
		if(sz!=0) printf("Wrote %d bytes to %s.\n", numwritten, argv[2]); // If you write, print some info

		/*
		//Use this code to print to screen instead
		for(i=0;i<sz;i++)
			printf("%c", buf[i]);
		printf("\n");
		if(sz!=0) printf("Read %d bytes.\n", sz); // If you write, print some info
		*/

	}
	
	printf("Read %lu bytes in %lu frames\n",bytes,frame);
	FSCCToolBox_Destroy(port); // Close the port
	fclose( fout ); // Close the file
	return 0;
}
