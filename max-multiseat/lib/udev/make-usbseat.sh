#!/bin/sh
#
# Read BUSNUM and DEVNUM from /sys/$1/../{busnum|devnum}
#  Search BUSNUM and DEVNUM in /tmp/seat.db and:
#   * if found, return SEAT_ID (column #3)
#   * not found, search for a free SEAT_ID, append line to /tmp/seat.db and return new SEAT_ID
#
# $2 is a optional parameter [display|mouse|keyboard|sound]

#logger() {
#  echo "$2: $3" >> /tmp/make-usbseat.log
#}

SEAT_DB=/dev/seat.db

# use /dev RAM (tmpfs) for SEATS text file
if [ ! -e $SEAT_DB ]; then
  touch $SEAT_DB
fi
# FIXME REMOVE when max-multiseat-storage change to new file
[ ! -e /tmp/seat.db ] && ln -s $SEAT_DB /tmp/seat.db



(
# lock in FD 9
flock -x -w 2 9

_BUSNUM=$(printf "%03g\n" $(cat "/sys/$1/../busnum" 2>/dev/null))
_DEVNUM=$(printf "%03g\n" $(cat "/sys/$1/../devnum" 2>/dev/null))
logger -t "/lib/udev/make-usbseat.sh [$2]" "/sys/$1/busnum BUSNUM=$_BUSNUM DEVNUM=$_DEVNUM"
REAL=$(readlink -f "/sys/$1/../")
logger -t "/lib/udev/make-usbseat.sh [$2]" "REAL=$REAL BUSNUM=$_BUSNUM DEVNUM=$_DEVNUM ID_VENDOR='$ID_VENDOR_ID' ID_MODEL='$ID_MODEL_ID'"


if grep -q "^$_BUSNUM $_DEVNUM" $SEAT_DB; then
  # return SEAT_ID (third column)
  SEAT_ID=$(grep "^$_BUSNUM $_DEVNUM" $SEAT_DB | awk '{print $3}' | tail -1)
  logger -t "/lib/udev/make-usbseat.sh [$2]" "found SEAT_ID in database: $SEAT_ID"
  echo $SEAT_ID
else
  # read biggest SEAT_ID
  LAST_SEAT=$(awk '{print $3}' $SEAT_DB 2>/dev/null| sort -n | tail -1)
  # if not found start in 1
  [ "$LAST_SEAT" = "" ] && LAST_SEAT=1
  # use LAST+1 for new SEAT_ID
  NEW_SEAT=$(($LAST_SEAT+1))
  echo "$_BUSNUM $_DEVNUM $NEW_SEAT" >> $SEAT_DB
  logger -t "/lib/udev/make-usbseat.sh [$2]" "created SEAT_ID '$NEW_SEAT'"
  echo $NEW_SEAT
fi

if [ "$ID_VENDOR_ID" = "0711" ] && [ "$ID_MODEL_ID" = "5100" ]; then
  logger -t "/lib/udev/make-usbseat.sh [$2]" "Calling /lib/udev/MWS300-init-tool $BUSNUM $DEVNUM"
  /lib/udev/MWS300-init-tool $BUSNUM $DEVNUM >> /tmp/usbseat.log 2>&1
fi

if [ "$ID_VENDOR_ID" = "0711" ] && [ "$ID_MODEL_ID" = "5500" ]; then
  logger -t "/lib/udev/make-usbseat.sh [$2]" "Calling /lib/udev/multiseat-init-tool.v2 $BUSNUM $DEVNUM"
  /lib/udev/multiseat-init-tool.v2 $BUSNUM $DEVNUM >> /tmp/usbseat.log 2>&1
fi

) 9>/tmp/make-usbseat.lock

rm -f /tmp/make-usbseat.lock
