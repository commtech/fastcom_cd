These examples were generated to demonstrate the usage of the Fastcom:FSCC
card to generate/simulate and recover/receive frames of the form:

0x7e,0x00,data[0],data[19]

or frames of 11 x 16 bit words (22 bytes), with the first word being 0x7e00
It is assumed that data[19] is a checksum, for the generator code
it is created as a simple 8 bit sum of data[0] to data[18].

The line bitrate is assumed to be 2500000 bps.
The outbound frame rate is 120Hz. (one frame every 8.3333ms)

To run the example code you need to construct a crossover cable of the form:

txd    -> rxd
txclko -> rxclk
rxd    <- txd
rxclk  <- txclko
gnd    -> gnd

To run the transparent examples you need a crossover of the form:

txd    -> rxd
txclko -> rxclk
rxd    <- txd
rxclk  <- txclko
gnd    -> gnd
fst    -> fsr
fsr    <- fst

The examples consist of 5 programs:
rx.exe - a program that configures the port to be in bisync mode and capture
       - frames with a 0x7e00 sync pattern, with a cutoff size of 22 bytes.

tx.exe - a program the configures the port to be in bisync mode and sends random 22byte
       - data packets with the proper starting sync pattern, a checksum byte,
       - and a 16 bit frame counter.

rcheck.exe - a program that examines the output file from rx.exe and determines
           - if the frame counter and checksum are correct for each packet.

ttx.exe - a program that configures the port in Transparent mode and sends the
        - same data as tx.exe.  This program can be used to generate an external
        - frame sync signal (mode 1 or mode 2) to be used by trx for frame recovery.

trx.exe - a program that configures the port in transparent mode and expects to
        - use a frame sync signal (and cutoff size) to enable reception of the frames.

Connect the crossover between the two ports of the FSCC card and execute:
(in one console window)
rx 0 rout

(in a second console window)
tx 1

The 'tx 1' windows should display a running count of the number of frames transmitted
let it run for awhile and press a key on the 'tx 1' window to stop it.
The 'rx 0 rout' window will (after a second or so) start printing out "receive timeout"
when it does press a key in that window to stop it.
Note that the transmitted frame count is inflated by the number of frames buffered
(there are ~250 frames buffered in the FSCC hardware, and 240 in the driver)
when the transmit program is stopped it doesn't wait for all of those frames to go out
so there will be a 4-500 frame difference between what the transmit program says
the frame count is on exit, and what the receive program says on exit.
If you want these two numbers to line up you would need to put a 5 second or so delay
in before closing the FSCC port on the transmitter.

You can then run rcheck.exe on the output file (rout) as:
rcheck rout

and the program will read each line of the rout file and check it for the proper frame
sequence number and checksum. At the end of its execution it should say "errorcount:0" (if all went well).

If you have the cable with the framesyncs (FSR/FST) you can run the transparent
examples in a similar maner:

trx 0 1 rout
ttx 1 1

(the first # is the FSCC port to use, the second # is the Frame sync mode (use either a 1 or a 2))
Framesync mode 1 is a signal that frames the data completely.
Framesync mode 2 is a signal that is active for the first data bit.

The code is currently not setup to run both the transmitter and receiver on one port,
It could be done, but you would need to modify the code so that the configuration
is only done by one of the two programs.
(ie don't try executing 'rx 0 rout' and 'tx 0' with a single port loopback, it won't work)


cg
10/30/2006

