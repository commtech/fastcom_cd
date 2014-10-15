#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
int _getch(void);
#endif

#include <fscc.h>

#define DATA_LENGTH 20
#define NUM_ITERATIONS 100

int init(fscc_handle h);
int loop(fscc_handle h);

int main(int argc, char *argv[])
{
    fscc_handle h;
    int e = 0;
    unsigned port_num;
    unsigned i = 0;

    if (argc != 2) {
        fprintf(stdout, "%s PORT_NUM\n", argv[0]);
        return EXIT_FAILURE;
    }

    port_num = atoi(argv[1]);

    e = fscc_connect(port_num, &h);
    if (e != 0) {
        switch (e) {
        case FSCC_PORT_NOT_FOUND:
            fprintf(stderr, "Port %i wasn't found.\n", port_num);
            break;

        default:
            fprintf(stderr, "fscc_connect failed with %d\n", e);
        }

        return EXIT_FAILURE;
    }

    fprintf(stdout, "This is a very simple test to verify your card is\n");
    fprintf(stdout, "communicating correctly.\n\n");

    fprintf(stdout, "NOTE: This will change your registers to defaults.\n\n");

    fprintf(stdout, "1) Connect your included loopback plug.\n");
    fprintf(stdout, "2) Press any key to start the test.\n\n");


    _getch();

    e = init(h);
    if (e != 0) {
        fscc_disconnect(h);
        return EXIT_FAILURE;
    }

    for (i = 0; i < NUM_ITERATIONS; i++) {
        e = loop(h);
        if (e != 0) {
            if (e == -1) {
                break;
            }
            else {
                fscc_disconnect(h);
                return EXIT_FAILURE;
            }
        }
    }

    if (e != -1)
        fprintf(stdout, "Passed, you can begin development.\n");
    else
        fprintf(stderr, "Failed, contact technical support.\n");

    fscc_disconnect(h);

    return EXIT_SUCCESS;
}

int init(fscc_handle h)
{
    struct fscc_registers r;
    struct fscc_memory_cap m;
    int e = 0;

    m.input = 1000000;
    m.output = 1000000;

    e = fscc_set_memory_cap(h, &m);
    if (e != 0) {
        fprintf(stderr, "fscc_set_memory_cap failed with %d\n", e);
        return EXIT_FAILURE;
    }

    e = fscc_disable_ignore_timeout(h);
    if (e != 0) {
        fprintf(stderr, "fscc_disable_ignore_timeout failed with %d\n", e);
        return EXIT_FAILURE;
    }

    e = fscc_disable_append_status(h);
    if (e != 0) {
        fprintf(stderr, "fscc_disable_append_status failed with %d\n", e);
        return EXIT_FAILURE;
    }

    e = fscc_disable_append_timestamp(h);
    if (e != 0) {
        fprintf(stderr, "fscc_disable_append_timestamp failed with %d\n", e);
        return EXIT_FAILURE;
    }

    e = fscc_set_tx_modifiers(h, XF);
    if (e != 0) {
        fprintf(stderr, "fscc_set_tx_modifiers failed with %d\n", e);
        return EXIT_FAILURE;
    }

    e = fscc_disable_ignore_timeout(h);
    if (e != 0) {
        fprintf(stderr, "fscc_disable_ignore_timeout failed with %d\n", e);
        return EXIT_FAILURE;
    }

    FSCC_REGISTERS_INIT(r);

    r.FIFOT = 0x08001000;
    r.CCR0 = 0x0011201c;
    r.CCR1 = 0x00000018;
    r.CCR2 = 0x00000000;
    r.BGR = 0x00000000;
    r.SSR = 0x0000007e;
    r.SMR = 0x00000000;
    r.TSR = 0x0000007e;
    r.TMR = 0x00000000;
    r.RAR = 0x00000000;
    r.RAMR = 0x00000000;
    r.PPR = 0x00000000;
    r.TCR = 0x00000000;
    r.IMR = 0x0f000000;
    r.DPLLR = 0x00000004;
    r.FCR = 0x00000000;

    e = fscc_set_registers(h, &r);
    if (e != 0) {
        fprintf(stderr, "fscc_set_registers failed with %d\n", e);
        return EXIT_FAILURE;
    }

    e = fscc_set_clock_frequency(h, 18432000);
    if (e != 0) {
        fprintf(stderr, "fscc_set_clock_frequency failed with %d\n", e);
        return EXIT_FAILURE;
    }

    e = fscc_purge(h, 1, 1);
    if (e != 0) {
        fprintf(stderr, "fscc_purge failed with %d\n", e);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int loop(fscc_handle h)
{
    unsigned bytes_written = 0, bytes_read = 0;
    char odata[DATA_LENGTH];
    char idata[100];
    int e = 0;

    memset(&odata, 0x01, sizeof(odata));
    memset(&idata, 0, sizeof(idata));

    e = fscc_write_with_blocking(h, odata, sizeof(odata), &bytes_written);
    if (e != 0) {
        fprintf(stderr, "fscc_write_with_blocking failed with %d\n", e);
        return EXIT_FAILURE;
    }

    e = fscc_read_with_timeout(h, idata, sizeof(idata), &bytes_read, 1000);
    if (e != 0) {
        fprintf(stderr, "fscc_read_with_timeout failed with %d\n", e);
        return EXIT_FAILURE;
    }

    if (bytes_read == 0 || memcmp(odata, idata, sizeof(odata)) != 0)
        return -1;

    return EXIT_SUCCESS;
}

#ifndef _WIN32
/* reads from keypress, doesn't echo */
int _getch(void)
{
    struct termios oldattr, newattr;
    int ch;

    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}
#endif
