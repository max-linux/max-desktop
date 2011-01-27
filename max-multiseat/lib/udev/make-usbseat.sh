#!/bin/sh
#
# Read BUSNUM and DEVNUM from /sys/$1/../{busnum|devnum}
#  Search BUSNUM and DEVNUM in /tmp/seat.db and:
#   * if found, return SEAT_ID (column #3)
#   * not found, search for a free SEAT_ID, append line to /tmp/seat.db and return new SEAT_ID
#
# $2 is a optional parameter [display|mouse|keyboard|sound]


SEAT_DB=/tmp/seat.db

_BUSNUM=$(printf "%03g\n" $(cat "/sys/$1/../busnum" 2>/dev/null))
_DEVNUM=$(printf "%03g\n" $(cat "/sys/$1/../devnum" 2>/dev/null))
logger -t "/lib/udev/make-usbseat.sh [$2]" "/sys/$1/busnum BUSNUM=$_BUSNUM DEVNUM=$_DEVNUM"

if [ -e "$SEAT_DB" ] && grep -q "$_BUSNUM $_DEVNUM" $SEAT_DB; then
  # return SEAT_ID (third column)
  SEAT_ID=$(grep "$_BUSNUM $_DEVNUM" $SEAT_DB | awk '{print $3}')
  logger -t "/lib/udev/make-usbseat.sh [$2]" "found SEAT_ID in database: $SEAT_ID"
  echo $SEAT_ID
  exit
else
  # create new line with a unused SEAT_ID
  for i in $(seq 2 200); do
    if [ ! -d "/dev/usbseat/${i}" ]; then
      # create SEAT_ID in SEAT_DB
      echo "$_BUSNUM $_DEVNUM $i" >> $SEAT_DB
      logger -t "/lib/udev/make-usbseat.sh [$2]" "created SEAT_ID '$i'"
      echo $i
      exit
    fi
  done
  logger -t "/lib/udev/make-usbseat.sh [$2]" "CRITICAL ERROR, i=200 and no free SEAT_ID"
fi
