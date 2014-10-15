/* $Id$ */
/*
Copyright(c) 2007, Commtech, Inc.
convert2async2.c -- user mode function convert a file to async formatted data
				 -- suitable for transmission out a FSCC sync port configured
				 -- as Transparent, with MSB first transmission.

usage:
 convert2async infile format outfile

 The infile is the (data)file to convert
 format can be any combination of:
 PARITY|BITS|STOPBITS
 where parity can be N,E,O,M,S

 N==None (no parity bit)
 E==Even (even parity, parity bit is 1 if #1bits is even)
 O==Odd  (odd parity,  parity bit is 1 if #1bits is odd)
 M==Mark (mark parity, parity bit is 1)
 S==Space (space parity, parity bit is 0)

 bits can be 9,8,7,6,5
 
 stobits can be 1 or 2

 outfile is the file to store the resultant data.

example:

  convert2async testfile.txt N81 outfile.bin

  This will convert the testfile.txt file into an async formatted data stream
  with no parity, 8 data bits, and one stop bit and place the results in outfile.bin.

  You could then transmit the outfile using the  sendfile.exe or similar.

*/

#include "windows.h"
#include "stdio.h"
#include "string.h"
#include "conio.h"

void setbit(int bitnum,unsigned char *byte);
void clearbit(int bitnum,unsigned char *byte);
void inc_curout(int *curoutp,unsigned char *outbyte,FILE *fout);
unsigned char getbit(int bitnum,unsigned char *byte);

#define NONE	0
#define EVEN	1
#define ODD		2
#define MARK	3
#define SPACE	4

#define ONE     1
#define TWO		2
#define FIVE	5
#define SIX		6
#define SEVEN	7
#define EIGHT	8
#define NINE	9

unsigned long wrsz;

int main(int argc, char *argv[])
{
	FILE *fin;
	FILE *fout;
	long finsz;
	unsigned char inbyte;
	unsigned char outbyte;
	long rdsz;
	int i;
	int stop;
	int bits;
	int parity;
	
	int curoutp;
	int numonebits;
	
	wrsz=0;
	if(argc != 4)
	{
		printf("usage:\n%s infile format outfile\n",argv[0]);
		exit(1);
	}
	
	fin = fopen(argv[1],"rb");
	if(fin==NULL)
	{
		printf("cannot open input file %s\n",argv[1]);
		exit(1);
	}
	
	fout = fopen(argv[3],"wb");
	if(fout==NULL)
	{
		printf("cannot open output file %s\n",argv[3]);
		fclose(fin);
		exit(1);
	}
	
	if((argv[2][0]=='n')||(argv[2][0]=='N'))
	{
		parity=NONE;
	}
	if((argv[2][0]=='e')||(argv[2][0]=='E'))
	{
		parity=EVEN;
	}
	if((argv[2][0]=='o')||(argv[2][0]=='O'))
	{
		parity=ODD;
	}
	if((argv[2][0]=='m')||(argv[2][0]=='M'))
	{
		parity=MARK;
	}
	if((argv[2][0]=='s')||(argv[2][0]=='S'))
	{
		parity=SPACE;
	}
	
	if((argv[2][1]=='9')||(argv[2][1]=='9'))
	{
		bits=NINE;
	}
	if((argv[2][1]=='8')||(argv[2][1]=='8'))
	{
		bits=EIGHT;
	}
	if((argv[2][1]=='7')||(argv[2][1]=='7'))
	{
		bits=SEVEN;
	}
	if((argv[2][1]=='6')||(argv[2][1]=='6'))
	{
		bits=SIX;
	}
	if((argv[2][1]=='5')||(argv[2][1]=='5'))
	{
		bits=FIVE;
	}
	
	if((argv[2][2]=='1')||(argv[2][1]=='1'))
	{
		stop=ONE;
	}
	if((argv[2][2]=='2')||(argv[2][1]=='2'))
	{
		stop=TWO;
	}
	
	
	fseek(fin,0,SEEK_END);
	finsz = ftell(fin);
	fseek(fin,0,SEEK_SET);
	
	printf("infile:%s,\tsize:%ld bytes\n",argv[1],finsz);
	//printf("outfile:%s\n",argv[3]);
	printf("format:");
	if(parity==NONE) printf("NONE,");
	if(parity==EVEN) printf("EVEN,");
	if(parity==ODD)  printf("ODD,");
	if(parity==MARK) printf("MARK,");
	if(parity==SPACE) printf("SPACE,");
	
	if(bits==NINE)   printf("9,");
	if(bits==EIGHT)  printf("8,");
	if(bits==SEVEN)  printf("7,");
	if(bits==SIX)    printf("6,");
	if(bits==FIVE)   printf("5,");
	
	if(stop==ONE)    printf("1");
	if(stop==TWO)    printf("2");
	
	printf("\n");
	
	curoutp=7;
	numonebits=0;
	outbyte=0;
	while (!feof(fin))
	{
		
		rdsz = fread(&inbyte,1,1,fin);
		if(rdsz==1)
		{
			numonebits=0;
			
			//start bit generation
			clearbit(curoutp,&outbyte);//set start bit
			inc_curout(&curoutp,&outbyte,fout);
			
			
			//data bit generation
			if(bits==9)
			{
			/*
				this assumes the input file is packed as:
				[data1][data2]
				[data3][data4]
				..
				where data1 bit0 is the 9th bit of data for the first byte (data2 holds the other 8 bits)
				data3 bit0 is the 9th bit of data for the second byte (data4 hold the other 8 bits)
			etc.
			*/
				if(getbit(0,&inbyte)==0) clearbit(curoutp,&outbyte);
				else 
				{
					setbit(curoutp,&outbyte);
					numonebits++;
				}
				inc_curout(&curoutp,&outbyte,fout);
				rdsz = fread(&inbyte,1,1,fin);//read in the other 8bits of data here
				if(rdsz!=1) 
				{
					printf("error! not enough bytes for 9bit\n");
				}
			}
			//this is LSB first transmission (assumes the lsb follows the start bit on the line)
			//if you want MSB first (unusual) then run this backwards (use for(i=bits-1;i>=0;i--)
			for(i=0;i<bits;i++)
			{
				if(i==8) continue;//if 9bit bits, i will =8, but we are done here
				if(getbit(i,&inbyte)==0) clearbit(curoutp,&outbyte);
				else 
				{
					setbit(curoutp,&outbyte);
					numonebits++;
				}
				inc_curout(&curoutp,&outbyte,fout);
			}
			
			//parity bit generation
			//(note parity==NONE skips all of these and thus does not insert a parity bit)
			if(parity==EVEN)
			{
				//the idea is to force the number of 1 bits to be even (including the parity bit)
				//so if it is already, then set a 0
				//if not set a 1 to make it so.
				if((numonebits%2)==0) clearbit(curoutp,&outbyte);
				else setbit(curoutp,&outbyte);
				inc_curout(&curoutp,&outbyte,fout);
			}
			if(parity==ODD)
			{
				//the idea is to force the number of 1 bits to be odd (including the parity bit)
				//so if it is even already, then set a 1 to make it odd
				//else set a 0 to leave it odd.
				if((numonebits%2)==0) setbit(curoutp,&outbyte);
				else clearbit(curoutp,&outbyte);
				inc_curout(&curoutp,&outbyte,fout);
			}
			if(parity==MARK)
			{
				//force a 1 parity bit
				setbit(curoutp,&outbyte);
				inc_curout(&curoutp,&outbyte,fout);
			}
			if(parity==SPACE)
			{
				//force a 0 parity bit
				clearbit(curoutp,&outbyte);
				inc_curout(&curoutp,&outbyte,fout);
			}
			
			//stop bit generation
			setbit(curoutp,&outbyte);//set stop bit
			inc_curout(&curoutp,&outbyte,fout);
			if(stop==TWO) 
			{
				setbit(curoutp,&outbyte);//set stop bit
				inc_curout(&curoutp,&outbyte,fout);
			}
		}
	}
	//force all of last byte to be idle (1's)
	while(curoutp!=7)
	{
		setbit(curoutp,&outbyte);//set stop bit
		inc_curout(&curoutp,&outbyte,fout);
	}
	
	//done
	fclose(fin);
	fclose(fout);
	
	printf("output:%s,\tsize:%ld bytes\n",argv[3],wrsz);
	
}

void setbit(int bitnum,unsigned char *byte)
{
	unsigned char temp;
	temp=0x01;
	temp=temp<<bitnum;
	byte[0] |= temp;
}
void clearbit(int bitnum,unsigned char *byte)
{
	unsigned char temp;
	temp = 0x01;
	temp=temp<<bitnum;
	temp = ~temp;
	byte[0] &= temp;
}

unsigned char getbit(int bitnum,unsigned char *byte)
{
	unsigned char temp;
	temp=0x01;
	temp=temp<<bitnum;
	if((byte[0]&temp)==0) return 0;
	else return 1;
	
}

void inc_curout(int *curoutp,unsigned char *outbyte,FILE *fout)
{
	curoutp[0]--;
	if(curoutp[0]==-1)
	{
		curoutp[0]=7;
		fwrite(&outbyte[0],1,1,fout);
		outbyte[0]=0;
		wrsz++;
	}
}