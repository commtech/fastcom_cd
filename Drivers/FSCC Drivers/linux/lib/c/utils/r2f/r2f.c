#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
int _kbhit(void);
#endif

#include <fscc.h>

#define DATA_LENGTH 3000

int main(int argc, char *argv[])
{
    fscc_handle h;
    struct fscc_registers r;
    char idata[DATA_LENGTH];
    unsigned bytes_stored = 0;
    unsigned bytes_read = 0;
    unsigned total_bytes_read = 0;
    int e = 0;
    unsigned port_num = 0;
    FILE *file;
    unsigned i = 0;
#ifdef _WIN32
    clock_t tstart, tend;
    float tdiff;
    double bps = 0;
#endif

    if (argc == 1) {
        fprintf(stdout, "%s PORT_NUM\n", argv[0]);
        return EXIT_FAILURE;
    }

    port_num = atoi(argv[1]);
    e = fscc_connect(port_num, &h);
    if (e != 0) {
        fprintf(stderr, "fscc_connect failed with %d\n", e);
        return EXIT_FAILURE;
    }

    file = fopen("out.hex", "wb");
    if (file == NULL) {
        fscc_disconnect(h);
        fprintf(stderr, "cannot open output file %s\n", "out.hex");
        return EXIT_FAILURE;
    }

    FSCC_REGISTERS_INIT(r);
    r.BGR = 0;
    r.CCR0 = 0x00500006;
    r.CCR1 = 0x10007088;
    r.CCR2 = 0x0a7f0000;

    e = fscc_set_registers(h, &r);
    if (e != 0) {
        fscc_disconnect(h);
        fprintf(stderr, "fscc_set_registers failed with %d\n", e);
        return EXIT_FAILURE;
    }

    /* 12 Mhz seems to be the cutoff */
    e = fscc_set_clock_frequency(h, 1000000);
    if (e != 0) {
        fscc_disconnect(h);
        fprintf(stderr, "fscc_set_clock_frequency failed with %d\n", e);
        return EXIT_FAILURE;
    }

    e = fscc_enable_append_status(h);
    if (e != 0) {
        fscc_disconnect(h);
        fprintf(stderr, "fscc_enable_append_status failed with %d\n", e);
        return EXIT_FAILURE;
    }

    e = fscc_purge(h, 0, 1);
    if (e != 0) {
        fscc_disconnect(h);
        fprintf(stderr, "fscc_purge failed with %d\n", e);
        return EXIT_FAILURE;
    }

#ifdef _WIN32
    tstart = clock();
#endif

    fprintf(stdout, "Press any key to stop collecting data...\n");

    while (_kbhit() == 0) {
        bytes_read = 0;
        bytes_stored = 0;

        memset(&idata, 0, sizeof(idata));

        e = fscc_read_with_timeout(h, idata, sizeof(idata), &bytes_read, 1000);
        if (e != 0) {
            fscc_disconnect(h);
            fprintf(stderr, "fscc_read_with_timeout failed with %d\n", e);
            return EXIT_FAILURE;
        }
        if (bytes_read) {
            total_bytes_read += bytes_read;
            bytes_stored = fwrite(idata, 1, bytes_read, file);

            if (bytes_stored != bytes_read)
                fprintf(stderr, "error writing to file\n");
        }

        i++;
    }

#ifdef _WIN32
    tend = clock();
    tdiff = (float)(tend - tstart) / CLOCKS_PER_SEC;
    bps = (double)(total_bytes_read * 8) / tdiff;

    fprintf(stdout, "Read %i bytes of data in %.2f seconds ", total_bytes_read, tdiff);

    if (bps > 1000000)
        fprintf(stdout, "(~%.2f MHz)\n", bps / 1000000);
    else if (bps > 1000)
        fprintf(stdout, "(~%.2f KHz)\n", bps / 1000);
    else
        fprintf(stdout, "(~%.2f Hz)\n", bps);
#else
    fprintf(stdout, "Read %i bytes of data\n", total_bytes_read);
#endif

    FSCC_REGISTERS_INIT(r);
    r.BGR = 0;
    r.CCR0 = 0x00100000;
    r.CCR1 = 0x04007008;
    r.CCR2 = 0x00000000;

    e = fscc_set_registers(h, &r);
    if (e != 0) {
        fscc_disconnect(h);
        fprintf(stderr, "fscc_set_registers failed with %d\n", e);
        return EXIT_FAILURE;
    }

    e = fscc_purge(h, 0, 1);
    if (e != 0) {
        fscc_disconnect(h);
        fprintf(stderr, "fscc_purge failed with %d\n", e);
        return EXIT_FAILURE;
    }

    fclose(file);
    fscc_disconnect(h);

    return EXIT_SUCCESS;
}

#ifndef _WIN32
int _kbhit(void) {
    static const int STDIN = 0;
    static unsigned initialized = 0;
    int bytes_waiting;

    if (!initialized) {
        /* Use termios to turn off line buffering */
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = 1;
    }

    ioctl(STDIN, FIONREAD, &bytes_waiting);
    return bytes_waiting;
}
#endif
