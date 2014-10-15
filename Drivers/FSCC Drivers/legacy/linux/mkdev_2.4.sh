#!/bin/sh
MODPATH=/lib/modules/`uname -r`/char
device=fsccp
devicenode=fscc

if [ -e $MODPATH ]
then
rm -f $MODPATH/${device}.o
else
mkdir $MODPATH
fi
cp ${device}.o $MODPATH

/sbin/insmod -f ${device}.o 
rm -f /dev/${devicenode}[0-5]

major=`cat /proc/devices | awk "\\$2==\"${device}\" {print \\$1}"`

mknod /dev/${devicenode}0 c ${major} 0 
mknod /dev/${devicenode}1 c ${major} 1 
mknod /dev/${devicenode}2 c ${major} 2 
mknod /dev/${devicenode}3 c ${major} 3 
mknod /dev/${devicenode}4 c ${major} 4
mknod /dev/${devicenode}5 c ${major} 5 

chmod 666 /dev/${devicenode}?
