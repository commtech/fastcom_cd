/*
	rcheck.c - a user app to check the output file generated using rx.exe
			 - reads the output file one line at a time (22 bytes) and verifies that
			 - the first two characters are the sync pattern (0x7e, 0x00)
			 - that the data[2],data[3] has the correct count (last frame count +1) (as inserted by the tx program)
			 - that the checksum (data[19] matches the checksummed data (sum(data[0]:data[18])

			basically checks that the received frames in the file are "in sequence" and not completely wrong

  usage:
   rcheck filename

*/

#include "windows.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"


int main(int argc, char *argv[])
{
FILE *fin;
UCHAR csum;
ULONG cnt;
ULONG lcnt = -1L;
UCHAR buf[256];
ULONG linecount=0;
ULONG errs=0;
int rcnt;
int i;

if(argc<2)
{
printf("usage:\n");
printf("%s file\n",argv[0]);
exit(1);
}

fin = fopen(argv[1],"rb");
if(fin==NULL)
{
printf("cannot open %s\n",argv[1]);
exit(1);
}
while(!feof(fin))
{
rcnt= fread(buf,sizeof(UCHAR),22,fin);
if(rcnt==22)
	{
	if(buf[0]!=0x7e)
		{
		printf("sync mismatch 0x%x!=0x7e\n",buf[0]);
		errs++;
		}
	if(buf[1]!=0x00)
		{
		printf("sync mismatch 0x%x!=0x00\n",buf[1]);
		errs++;
		}
	cnt = buf[4];
	cnt = cnt<<8;
	cnt = cnt | buf[5];
	if( cnt != ((lcnt+1) & 0xffff) )
		{
		printf("count mismatch, %d != %d\n",cnt,lcnt+1);
		errs++;
		}
	lcnt = cnt;
	csum=0;
	for(i=2;i<21;i++) csum = csum + buf[i];
	if(buf[21]!=csum)
		{
		printf("checksum mismatch 0x%2.2x != 0x%2.2x\n",csum,buf[21]);
		errs++;
		}
	printf("line:%d, cnt:%d, csum:0x%2.2x\n",linecount,cnt,buf[21]);
	}
linecount++;
}
printf("linecount :%d\n",linecount);
printf("errorcount:%d\n",errs);
fclose(fin);
}
