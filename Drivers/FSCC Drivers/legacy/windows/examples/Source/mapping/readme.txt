Simulating the "Octet synchronous PPP" mode from the SuperFastcom using the
FSCC card.

To enable the control mapping that occurs in this mode you must use these
DeviceIoControl function calls:

IOCTL_FSCCDRV_SET_CHARACTER_MAP_ENABLE 
IOCTL_FSCCDRV_SET_CHARACTER_MAP        
IOCTL_FSCCDRV_SET_UCHARACTER_MAP       

Each of these functions takes a single DWORD as an input parameter, and
does not return anything.

Calling IOCTL_FSCCDRV_SET_CHARACTER_MAP_ENABLE  with a 0x00000001 as its input
parameter will enable the control character mapping feature.

Calling IOCTL_FSCCDRV_SET_CHARACTER_MAP_ENABLE  with a 0x00000000 as its input
parameter will return the FSCC to normal operation.

Calling IOCTL_FSCCDRV_SET_CHARACTER_MAP will take the given DWORD and enable/disable
mapping for the individual control characters in the range 0x00-0x1f depending
on the bits in the DWORD that is passed.  A '1' in any bit postion will enable
mapping for that control character.  (passing a value 0x00000001 will enable mapping
for the value 0x00 in the transmitted/received data, passing a value 0x00000002
will enable mapping for the value 0x01.  passing a value 0x00000003 will enable mapping
for both 0x00, and 0x01, etc. etc. on through bit position/control character 0x31
(this is the equivalent of the SuperFastcom ACCM register)


Calling IOCTL_FSCCDRV_SET_UCHARACTER_MAP will set the user defined character mapping
bytes.  These are four bytes that are user selectable, and if they show up in the data
they will be mapped in the same way that the control characters are (added 0x7d, and
the value with bit5 inverted).  (This is the equivalent of the SuperFastcom UDAC
register setting.)


For testing I connected a FSCC to a SuperFastcom using (rxd->RD,txd->SD,txclk->RT,rxclk->TT)
On the SuperFatcom I used the following settings:

on the FSCC I used the mapset configuration file:

fifot        = 0x08001000
cmdr         = 0x08020000
ccr0         = 0x00113005
ccr1         = 0x04000008
ccr2         = 0x00000000
bgr          = 0x00000000
ssr          = 0x0000007e
smr          = 0x00000000
tsr          = 0x00007e7e
tmr          = 0x00000000
rar          = 0x00000000
ramr         = 0x00000000
ppr          = 0x00000000
tcr          = 0x00000000
imr          = 0x00000000
n_rbufs      = 0xa
n_rfsize_max = 0x1000
n_tbufs      = 0xa
n_tfsize_max = 0x1000

This configures X-SYNC mode, idle=flags, one sync byte, one term byte (both set to 0x7e)
no address checking, no frame sync, crc=ccitt, clock mode 1, data is LSB first, NRZ line encoding
CTS control is manual, no zero insertion, no one insertion, Append TERM byte, append CRC
receive CRC checking enabled, do not return CRC bytes with data, CRC resetlevel=all 1's

I then executed fsccterm.exe and sfcmfc.exe and type back and forth.  Noting that the
mapped characters flow through as expected.


