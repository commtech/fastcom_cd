#include <conio.h>
#include <iostream>

#include <Windows.h>

#include <fscc.hpp>

#define DATA_LENGTH 20

int init(Fscc::Port &p);
int loop(Fscc::Port &p1, Fscc::Port &p2);

int main(int argc, char *argv[])
{
    int e = 0;
    unsigned port_num_1, port_num_2;
    BOOL reset;
    unsigned iterations = 0;
    unsigned mismatched = 0;

    if (argc < 3 || argc > 4) {
        fprintf(stdout, "%s PORT_NUM PORT_NUM [RESET_REGISTER=1]", argv[0]);
        return EXIT_FAILURE;
    }

    port_num_1 = atoi(argv[1]);
    port_num_2 = atoi(argv[2]);
    reset = (argc == 4) ? atoi(argv[3]) : 1;

    Fscc::Port p1((unsigned)port_num_1);
    Fscc::Port p2((unsigned)port_num_2);

    if (reset) {
        e = init(p1);
        if (e != 0)
            return EXIT_FAILURE;
        
        e = init(p2);
        if (e != 0)
            return EXIT_FAILURE;
    }

    std::cout << "Data looping, press any key to stop..." << std::endl;

    while (_kbhit() == 0) {
        e = loop(p1, p2);
        if (e != 0) {
            if (e == ERROR_INVALID_DATA)
                mismatched++;
            else
                return EXIT_FAILURE;
        }

        iterations++;
    }

    if (mismatched == 0)
        fprintf(stdout, "Passed (%d iterations).", iterations);
    else
        fprintf(stderr, "Failed (%d out of %d iterations).", 
                mismatched, iterations);

    return EXIT_SUCCESS;
}

int init(Fscc::Port &p)
{
    Fscc::Registers r;
    Fscc::MemoryCap m;

    m.input = 1000000;
    m.output = 1000000;

    p.SetMemoryCap(m);

    p.DisableRxMultiple();
    p.DisableAppendStatus();
    p.DisableAppendTimestamp();
    p.SetTxModifiers(Fscc::TxModifiers::XF);
    p.DisableIgnoreTimeout();

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

    p.SetRegisters(r);

    p.SetClockFrequency(18432000);
    
    p.Purge(true, true);

    return ERROR_SUCCESS;
}

int loop(Fscc::Port &p1, Fscc::Port &p2)
{
    unsigned bytes_written = 0, bytes_read = 0;
    char odata[DATA_LENGTH];
    char idata[100];

    memset(odata, 0x01, sizeof(odata));
    memset(&idata, 0, sizeof(idata));

    bytes_written = p1.Write(odata, sizeof(odata));
    bytes_read = p2.Read(idata, sizeof(idata), 1000);
    
    if (bytes_read == 0 || memcmp(odata, idata, sizeof(odata)) != 0)
        return ERROR_INVALID_DATA;

    return ERROR_SUCCESS;
}