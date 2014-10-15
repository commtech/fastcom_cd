/*
 * loop.c: User mode program that illustrates how to use some of the termios 
 * structure to configure a serial port.  See the man pages for termios for 
 * more details about the complex termios structure.
 *
 * The program will transmit random bytes and receive those bytes on the same
 * port.  It will check that the bytes transmitted match those received.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[])
{
    struct termios tios;
    int fd = -1;
    int size = 255;
    int j = 0;
    int i = 0;
    int repetitions;
    int errors = 0;
    long bits;
    long parity;
    long stopbits;
    long txsize;
    long rxsize;
    unsigned long frequency;
    char txbuf[1024];
    char rxbuf[1024];
    unsigned int usecs;

    if (argc < 2) {
	printf("\tusage ./loop device repetitions\n");
	printf("\t\tdevice: /dev/ttySx\n");
	printf("\t\trepetitions: integer\n");
	exit(1);
    }
    fd = open(argv[1], O_RDWR);
    if (fd < 0) {
	printf("\tcannot open port %s\r\n", argv[1]);
	exit(1);
    }
    repetitions = atoi(argv[2]);
    parity = 0;
    bits = CS8;
    stopbits = 1;

    printf("\tsetting %s to ", argv[1]);

    if (parity == 0)
	printf("N,");
    else if (parity == (PARENB | PARODD))
	printf("O,");
    else if (parity == PARENB)
	printf("E,");

    if (bits == CS8)
	printf("8,");
    else if (bits == CS7)
	printf("7,");
    else if (bits == CS6)
	printf("6,");
    else if (bits == CS5)
	printf("5,");

    if (stopbits == 1)
	printf("1\n");
    else if (stopbits == CSTOPB)
	printf("2\n");
    else if (stopbits == 0)
	printf("0!!!!!!!!!!!!!\n");

    tcgetattr(fd, &tios);
    printf("\told iflag=0x%08x\n", tios.c_iflag);
    printf("\told oflag=0x%08x\n", tios.c_oflag);
    printf("\told cflag=0x%08x\n", tios.c_cflag);
    printf("\told lflag=0x%08x\n", tios.c_lflag);
    printf("\told line=%02x\n", tios.c_line);

    tios.c_iflag = IGNBRK;
    tios.c_oflag = 0;
    tios.c_cflag = bits | stopbits | parity | CREAD | CLOCAL;
    //note CLOCAL ignores modem lines
    tios.c_lflag &= ~(ICANON | ISIG | ECHO);

    tios.c_cc[VTIME] = 5;	//TIME=500 miliseconds
    tios.c_cc[VMIN] = size;
    //vmin=0,vtim>0 will return a read every vtime*.1 seconds with or 
    //without data as the case may be

    cfsetospeed(&tios, B9600);	//set ospeed
    cfsetispeed(&tios, 0);	//set ispeed = ospeed 
    //Then we will get a baud of 9600 bit/s

    printf("\tnew iflag=0x%08x\n", tios.c_iflag);
    printf("\tnew oflag=0x%08x\n", tios.c_oflag);
    printf("\tnew cflag=0x%08x\n", tios.c_cflag);
    printf("\tnew lflag=0x%08x\n", tios.c_lflag);
    printf("\tnew line=%02x\n", tios.c_line);
    tcsetattr(fd, TCSANOW, &tios);

    for (i = 0; i < repetitions; i++) {
	for (j = 0; j < size; j++) {
	    txbuf[j] = rand();
//              txbuf[j] = j;
	}
	txsize = write(fd, txbuf, size);
	rxsize = 0;
	rxsize = read(fd, rxbuf, size);
	if (txsize != rxsize) {
	    errors++;
	    printf("\tsize mismatch sent:%d received:%d\n", txsize,
		   rxsize);
	} else {
	    for (j = 0; j < rxsize; j++) {
		if ((rxbuf[j] & 0xff) != (txbuf[j] & 0xff)) {
		    errors++;
		    printf("\tData mismatch at %d\n", j);
		    printf("\t0x%x != 0x%x\n", rxbuf[j] & 0xff,
			   txbuf[j] & 0xff);
		} else
		    printf("\tloop[%d] match: sent == received\r", i + 1);
	    }
	}
    }
    printf("\n\tErrors:%d\n", errors);
    close(fd);

    return 0;
}

/* $Id$ */
