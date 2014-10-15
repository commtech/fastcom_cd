Transparent mode operation: sending/receiving raw bits.

I had port 1 connected to port 2 via a crossover cable (txd->rxd, txclko->rxclk, fst->fsr).

if you are doing transmit only you can get by with just looking at the outputs (txd & txclko).  You will likely want to change
the clock mode away from mode 1 to mode 7 (to prevent issues with the lack of a receive clock signal), and disable the receiver.
(extset_t, uses mode 7 and disables the receiver)

If you are doing receive only, then you just need a clock and data source on rxclk and rxd. (and optionally a frame sync signal on fsr, to provide byte alignment).  The extset file assumes the frame syncs are in place (using mode 1/framing all of the data)
You could turn the framesyncs off and just "receive everything" at which point it becomes much like the superfastcom where byte alignment will be difficult.
(extset_r uses mode 1 and does not use framesyncs)
(extset uses clock mode 1 and framesync mode 1)

open a console window and execute:


setclock 0 10000000                
setclock 1 10000000
setfscc 0 extset
setfscc 1 extset
setfscc 0 extset

It is likely (unless you had the port(s) configured prior to running this example that there was not a clock on rxclk when the first 'setfscc 0 extset' line was executed, so it indicated that the port was busy.  After the configuration of port 1, the clock is present and the second pass at configuring port 0 completes successfully.
 
At this point it is good to go.
I opened two console windows and executed:

write_random 1 1024 10000000 wout

in one window and:

read 0 1024 out 10000000

in the other.
let it run until the receive program exits

note that the size requested is 1024 which is the same as the driver buffer selection in the settings file (n_rfsize_max)
the readcutoff program tells the driver to return data at the driver buffer boundaries instead of waiting for an end of frame.(since there is no end of frame indication in transparent mode, this is the only way to get data back).


To verify the files a couple of utilities were generated (ts_comp,disp_bin)
The out file may or may not be the same as wout, since more than likely the
two programs aren't started at the same time, the beginning points in the file
will be different.
If you started the read program first, then the write_random, you should be able to diff the wout and out files and have them be the same.  if not...
The ts_comp program can be used to verify the data, it scans the input file for
the framecounter (inserted by the write_random code), and then scans the wout file for the same counter value, then
compares the rest of the bytes in the file.

ts_comp out wout

should successfully compare the bulk of the saved files.

The read program will dump data read from a port using the given blocksize
for the reads, and save the data to the specified file.
For the FSCC series, keep in mind that you need to specify the block_size to be the same value as given as n_rfsize_max in the extset file. (in this case 1024).


The write_random program will generate a random bitstream with a 12 byte
framecounter placed every (blocksize) bytes in the file. (the 12 byte sequence
is 0xff,0x00,8 digit ascii counter,0x00,0xff).  It outputs this stream to the
given fscc port, and optionally also to a specified file.
(if you make the blocksize larger than the fifodepth (4096 byte) and the bitrate fast, and the system is
loaded too much you will not end up with a continuous output, which you
will see as long strings of 0xff's in the received file).  decreasing the blocksize
should take care of this. (if the blocksize is less than 4096, then the only
alternative you have is to reduce the bitrate, or move to the SuperFSCC (DMA controlled).

The disp_bin program dumps a binary file to the screen as hex digits,
25 bytes to a row.

The ts_comp program searches the input file for the 12 byte framecounter sequence
(as per write_random) and when it finds a likely suspect, it then looks for that
framecount in the compare file, and then compares the rest of the input file
to the data in the compare file.

The extset file is the settings file to setup transmitting in transparent mode,
it sets up clock mode 1 which is the desired mode to work with (external receive
clock, generated transmit clock) and expects to use the frame sync signals (using mode 1) 
framing all of the data. 

The write_file program can be used to dump any file out a port (for instance a
captured file generated via read...)

To capture raw data from the line (with an external clock connected to rxclk+/- and a data source connected to rxd+/-)
execute:

setfscc 0 extset_r
read 0 1024 outputfile 10000000

This will capture 10000384 bytes from the data lines and store them to the file 'outputfile'.

To transmit raw data from a file, use:

setclock 0 10000000
setfscc 0 extset_t
write_file 0 1024 outputfile



