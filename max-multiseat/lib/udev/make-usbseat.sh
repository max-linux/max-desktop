#!/bin/sh

#cat /sys/devices/pci0000\:00/0000\:00\:04.1/usb2/2-1/2-1.3/../busnum

logger -t "/lib/udev/make-usbseat.sh" "cat $1/../busnum"
A=$(cat "/sys/$1/../busnum" 2>/dev/null)
if [ "$A" = "" ]; then
  logger -t "/lib/udev/make-usbseat.sh" "no busnum"
  exit
fi
echo $(($A+1))
logger -t "/lib/udev/make-usbseat.sh" "A=$A"
