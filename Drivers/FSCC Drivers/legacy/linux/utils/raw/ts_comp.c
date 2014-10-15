#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
FILE *fin;
FILE *fcmp;
unsigned char inbuf[25];
unsigned char cmpbuf[25];
unsigned long inoffset=0;
unsigned long cmpoffset=0;
unsigned char inbyte;
unsigned char cmpbyte;
unsigned long errors=0;
unsigned long bytes_compared=0;
int i;

memset(inbuf,0x00,sizeof(inbuf));
memset(cmpbuf,0x00,sizeof(cmpbuf));

if(argc<3)
{
printf("usage:\n");
printf("ts_comp readlogfile writelogfile\n");
exit(1);
}
fin=fopen(argv[1],"rb");
if(fin==NULL)
{
printf("cannot open input file %s \n",argv[1]);
exit(1);
}
fcmp = fopen(argv[2],"rb");
if(fcmp==NULL)
{
printf("cannot open compare file %s\n",argv[2]);
fclose(fin);
exit(1);
}
inoffset=0;
while(!feof(fin))
{
for(i=1;i<=11;i++) inbuf[i-1]=inbuf[i];
fread(&inbuf[11],1,1,fin);
inoffset++;
if((inbuf[0]==0xff)&&(inbuf[1]==0)&&(inbuf[2]==0x30)&&(inbuf[10]==0x00)&&(inbuf[11]==0xff))
{
printf("possible timestamp found at %ld:",inoffset);
for(i=0;i<=11;i++) printf("%2.2x",inbuf[i]&0xff);
printf("\n");
goto syncit;
}
}
printf("end of input file reached\n");
fclose(fin);
fclose(fcmp);
exit(1);

syncit:
cmpoffset=0;
while(!feof(fcmp))
{
for(i=1;i<=11;i++) cmpbuf[i-1]=cmpbuf[i];
fread(&cmpbuf[11],1,1,fcmp);
cmpoffset++;
if(memcmp(inbuf,cmpbuf,12)==0)
{
	printf("match found at offset %ld\n",cmpoffset);
	goto cmpit;
}
}
printf("end of compare file reached\n");
fclose(fin);
fclose(fcmp);
exit(1);

cmpit:
while((!feof(fin))&&(!feof(fcmp)))
{
fread(&inbyte,1,1,fin);
fread(&cmpbyte,1,1,fcmp);
bytes_compared++;
inoffset++;
if(inbyte!=cmpbyte)
	{
	errors++;
	printf("offset:%ld -- %2.2x : %2.2x\n",inoffset,inbyte,cmpbyte);
	}
}
printf("out of data to compare\n");
printf("%ld bytes compared\n",bytes_compared);
printf("%ld errors\n",errors);
fclose(fin);
fclose(fcmp);
return 0 ;

}
