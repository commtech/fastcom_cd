import sys
import fscc
import msvcrt


def init(p):
    print('Restoring to default settings.')

    p.memory_cap.input = 1000000;
    p.memory_cap.output = 1000000;

    p.rx_multiple = False;
    p.append_status = False;
    p.append_timestamp = False;
    p.tx_modifiers = fscc.XF;
    p.ignore_timeout = False;

    p.registers.FIFOT = 0x08001000
    p.registers.CCR0 = 0x0011201c
    p.registers.CCR1 = 0x00000018
    p.registers.CCR2 = 0x00000000
    p.registers.BGR = 0x00000000
    p.registers.SSR = 0x0000007e
    p.registers.SMR = 0x00000000
    p.registers.TSR = 0x0000007e
    p.registers.TMR = 0x00000000
    p.registers.RAR = 0x00000000
    p.registers.RAMR = 0x00000000
    p.registers.PPR = 0x00000000
    p.registers.TCR = 0x00000000
    p.registers.IMR = 0x0f000000
    p.registers.DPLLR = 0x00000004
    p.registers.FCR = 0x00000000

    p.clock_frequency = 18432000

    p.purge(True, True)

def loop(p1, p2):
    odata = b'Hello world!';

    p1.write(odata);
    idata = p2.read()[0];
    
    if (len(idata) == 0 or odata != idata):
        return -1;

    return 0;

if __name__ == '__main__':
    port_num_1, port_num_2 = 0, 0
    iterations = 0
    mismatched = 0
    
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        print('loop.exe PORT_NUM PORT_NUM [RESET_REGISTER=1]')
        exit(1)

    port_num_1, port_num_2 = sys.argv[1], sys.argv[2]
    
    try:
        if len(sys.argv) == 3:
            reset = True if sys.argv[3] > 0 else False
        else:
            reset = True
    except:
        reset = True

    p1 = fscc.Port(port_num_1)
    p2 = fscc.Port(port_num_2)
    
    if reset:
        init(p1)
        init(p2)

    print('Data looping, press any key to stop...')

    while msvcrt.kbhit() is 0:
        e = loop(p1, p2)
        if e != 0:
            if e == -1:
                mismatched += 1
            else:
                exit(1)
                
        iterations += 1

    if mismatched == 0:
        print('Passed ({} iterations).'.format(iterations))
    else:
        print('Failed ({} out of {} iterations).'.format(mismatched, iterations))

    exit(0)