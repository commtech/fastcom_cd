/*
 disp_bin.c -- a program to dump a binary file as hex digits
			-- dumps 25 bytes per line to the display

  usage:
  
  disp_bin inputfile

*/
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

int main(int argc, char *argv[])
{
FILE *fin;
unsigned char inbyte;
int i;

if(argc<2)
{
printf("usage:\n");
printf("disp_bin inputfile\n");
exit(1);
}
fin = fopen(argv[1],"rb");
if(fin==NULL)
{
printf("cannot open input file %s\n",argv[1]);
exit(1);
}
while(!feof(fin))
{
for(i=0;i<25;i++)
	{
	if(!feof(fin))
		{
		fread(&inbyte,1,1,fin);
		printf("%2.2x",inbyte&0xff);
		}
	}
printf("\n");
}
fclose(fin);
return 0;
}
