Installation Instructions for Commtech's Fastcom:FSCC Series adapters using the
serial architecture.

1.0 Modifying the serial.c file

1.1 You will have to obtain the source code for your kernel to perform this
    installation.  We are modifying the serial kernel code.

1.2 Once you have your kernel source installed, you will have to modify the
    yourpath/linux-2.4.x/drivers/char/serial.c file:

    The patch file serial_2.4.32.patch allows you to add support for the 
    Fastcom:FSCC to the 2.4.32 (or later) kernel.  This patch should 
    also work on later kernel builds, if it does not please contact us.

    $ cd /yourpath/linux-2.4.x/drivers/char
    $ patch -p0 <serial_2.4.32.patch

    Alternatively you can copy the serial.c file to:
    /yourpath/linux-2.4.x/drivers/char 
    directory (backing up the original) This is less preferable unless you are 
    using the 2.4.32 kernel.  If your build is newer, you probably want to 
    go for the patch.

    I can confirm that this patch works on 2.4.32.

1.3 Once modified appropriately, you must either recompile the kernel, or just 
    the module.  If you decide to recompile only the module, you must be sure
    to use the .config file for your running kernel, or it won't work.  If you
    need help rebuilding your kernel, check the HOWTO: 
    http://www.digitalhermit.com/linux/Kernel-Build-HOWTO.html 

    If your efforts were successful, you can boot into your new kernel and 
    check your boot message log.  You will see something like:

       ttyS04 at I/O 0xd000 (irq = 4) is a 16C950/954

    for each of your newly discovered Fastcom True-Async ports.

2.0 Board specific functions: The fsccp module 

    This module will allow you to change the base frequency of the port or 
    the RS485/RS422 control and Receive echo cancel features.

2.1 Building the fsccp module and utilities

    $ cd fsccp
    $ make -f Makefile_2.4
    $ ./mkdev_2.4.sh (note you only have to run this once; on subsequent boots
                      you can simply run 'insmod fsccp' to load the driver)

    $ cd utils
    $ make
    $ make install

    To determine which board is associated with which minor # you can look at 
    the proc entry for fsccpdrv:

       $ cat /proc/fsccpdrv

    You can also determine which /dev/ttyS#'s are associated with which 
    card/ports by looking in the system message log:

       $ cat /var/log/messages | grep ttyS
   
    The I/O address of the port will be the second BAR range for the card 
    (as seen in lspci -v) matching that value to the first BAR address (which 
    is used by the FSCC sync module) is currently the only way to make this 
    correlation.

2.2 Clock frequency relationship

    The FSCC ports each feature a programmable clock generator that is fed into 
    the UART, allowing for custom baud rates.  The clock generator can output
    a minimum frequency of 20 kHz and the UART can accept a maximum frequency
    of 50 MHz, so the frequencies that you set must lie within this range.

    Inside the UART, the input clock is initially divided by 16 to get the 
    maximum attainable baud rate, or baud_base as described by setserial.

    By default the driver expects the clock frequency to be 14.7456 MHz.  
    Divide this frequency by 16 and you get a baud_base of 921600 bps.  With 
    this baud_base a divisor of 1 will yield 921600 bps, 16 yields 57600 bps, 
    96 yields 9600 bps, etc.

    Before you proceed, you must set the ports' clocks to their expected
    frequencies:

      ./bin/setclock 0 14745600
      ./bin/setclock 1 14745600

2.2.1 Set a custom baud rate

    All standard baud rates can be set directly using stty without changing
    anything else.  You do not need any of the following.

    There are two ways that you can set custom baud rates, you can choose
    the one that makes sense to you.

2.2.2 Change clock and use standard baud rates as divisors only

      This is probably the easiest method and can be used in your own programs
      that utilize the termio structure (see loop.c).

      Change the clock frequency and the set whatever standard baud rate 
      corresponds to the divisor that you are looking for.  You can create
      a divisor look-up table like this:

        921600: divisor = 1	(baud_base)
        460800: divisor = 2
        230400: divisor = 4
        115200: divisor = 8
        57600: divisor = 16
        38400: divisor = 24
        28800: divisor = 32
        19200: divisor = 48
        14400: divisor = 64
        9600: divisor = 96
        4800: divisor = 192
        2400: divisor = 384

      Example 1:

        $examples/bin/setclock /dev/ttySx 16000000

	$stty -F /dev/ttySx 921600		(16M/16/1=1Mbit/s)
        or
        $stty -F /dev/ttySx 230400		(16M/16/4 = 250k bit/s)

      Example 2:

        $examples/bin/setclock /dev/ttySx 20000000
        $stty -F /dev/ttySx 460800		(20M/16/2 = 625k bit/s)
        or
        $stty -F /dev/ttySx 57600		(20M/16/16 = 78125 bit/s)

2.2.3 Use the spd_cust flag and 38400 baud

      $examples/bin/setclock /dev/ttySx F
      $setserial /dev/ttySx baud_base M spd_cust divisor N
      $stty -F /dev/ttySx 38400 [other options]
      where:
        F = desired frequency
        B = baud = M / N
        M = baud_base = F / 16
        N = divisor = M / B

      The baud rate will appear to be 38400, but the driver is using a special 
      case where 38400 is actually your custom baud rate.

      Example 1: Want to set 1Mbit/sec baud rate

        set F = 16,000,000
        M = F / 16 = 1,000,000
        N = M / B = 1
        $setserial /dev/ttyM0 baud_base 1000000 spd_cust divisor 1
        $stty -F /dev/ttyM0 38400 -echo icrnl ocrnl

      Example 2: Want to set 75kbit/sec baud rate

        set F = 6,000,000
        M = F / 16 = 375,000
        N = M / B = 5
        $setserial /dev/ttySx baud_base 375000 spd_cust divisor 5
        $stty -F /dev/ttySx 38400 -echo -crtscts parenb -parodd

2.3 Set Features

    By default, the ports on a Fastcom FSCC card are configured as:

       RS485 driver enable(SD) = turned off (i.e. RS422 mode)
       Rx Echo Cancel = turned off
       RS485 driver enable(TT) = turned off (i.e. RS422 mode)

    Rs485 driver enable - setting this bit allows the transmit driver to
    tri-state when not actively transmitting to avoid bus collisions.

    Rx Echo Cancel - when enabled, this bit turns off the receiver during
    transmits.  This is useful in 2-wire 485 situations.

    If you need to switch between ASYNC operation and SYNC operation use 
    setfeatures.

    To enable port 0 to be a SYNC port (accessed through /dev/fscc0)

      bin/setfeatures 0 0 0 0 0

    To enable port 0 to be an ASYNC port (accessed through /dev/ttySx)

      bin/setfeatures 0 0 0 0 1

    Note that to use these functions you must have the fsccp module loaded and 
    the /dev/fsccX devnodes created.

    By default the serial.c driver will force the ports into ASYNC mode, so on 
    power up both Fastcom:FSCC ports should be accessable via /dev/ttySx.

    Use setclock to program the clock generator to a known value before you 
    try to use the port!
