#!/bin/sh
MODPATH=/lib/modules/`uname -r`/kernel/drivers/char
device=fsccp
devicenode=fscc
default_ports=12
#edit the fsccdrv.h variable MAX_FSCC_BOARDS to correspond with this number

if [ $1 ]; then
	ports=$1
else
	ports=$default_ports
fi

if [ -e $MODPATH ]; then
	rm -f $MODPATH/${device}.ko
else
	mkdir $MODPATH
fi

cp ${device}.ko $MODPATH
/sbin/insmod ${device}.ko 
rm -f /dev/${devicenode}[0-5]

major=`cat /proc/devices | awk "\\$2==\"${device}\" {print \\$1}"`

for i in $(seq 0 1 $ports)
do
	mknod /dev/${devicenode}$i c ${major} $i
done

chmod 666 /dev/${devicenode}*
