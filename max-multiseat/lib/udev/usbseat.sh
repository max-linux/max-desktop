#!/bin/bash
# takes the "seat number" as parameter $1
# the seat number is the kernel device id of the hub the seat's devices are sitting off of
# called once for every usb device that MIGHT be part of a seat, when they arrive or remove 

echo "--------------------------------------------" >> /tmp/usbseat.log
env >> /tmp/usbseat.log

if [[ !(-n `/bin/pidof gdm`) ]]; then
    exit 0
fi

if [ "$1" = "1"  ]; then
  echo "NOT make things in DISPLAY $1" >> /tmp/usbseat.log
  exit 0
fi

seat_running=`/usr/bin/gdmdynamic -l | /bin/sed -n -e "/:$1,/p"`

# $ACTION environment variable is set by udev subsystem
case "$ACTION" in
	'remove')
		if [[ -n "{$seat_running}" ]]; then
			/usr/bin/gdmdynamic -v -d $1
		fi
		;;
	*)
                # A device which might be part of a seat has been added

		# if we already have a running seat for this #, exit
		if [[ -n "${seat_running}" ]]; then
			exit 0
		fi

		if [[ -e /dev/usbseat/$1/keyboard && -e /dev/usbseat/$1/mouse && -e /dev/usbseat/$1/display ]]; then

			# We have a newly complete seat. Start it.
			TMPFILE=`/bin/mktemp` || exit 1
			if lsusb | grep -q 0711:5100; then
				echo "MWS300 complete, launching GDMdynamic in $1 with $TMPFILE" >> /tmp/usbseat.log
				/bin/sed "s/%ID_SEAT%/$1/g" < /lib/udev/usbseat-xf86.tusb.conf.sed > $TMPFILE
			else
				/bin/sed "s/%ID_SEAT%/$1/g" < /lib/udev/usbseat-xf86.conf.sed > $TMPFILE
			fi
			tree /dev/usbseat/$1 >> /tmp/usbseat.log

			/usr/bin/gdmdynamic -v -t 2 -s 1 -a "$1=/usr/bin/X -br :$1 vt07 -audit 0 -nolisten tcp -config $TMPFILE"
			#/usr/bin/gdmdynamic -v -t 2 -s 1 -a "$1=/usr/bin/X -br :$1 -audit 0 -nolisten tcp -novtswitch -sharevts -config $TMPFILE"

			/usr/bin/gdmdynamic -v -r $1
		else
			if lsusb | grep -q 0711:5100; then
				echo "MWS300 FIRST RUN ">> /tmp/usbseat.log
				tree /dev/usbseat/$1 >> /tmp/usbseat.log

				# MWS300 have only sound and display in /dev/usbseat/$1 before Xorg
				# launch Xorg and kill it
				if [ -e /dev/usbseat/$1/display ]; then
					TMPFILE=`/bin/mktemp` || exit 1
					/bin/sed "s/%ID_SEAT%/$1/g" < /lib/udev/usbseat-xf86.tusb.conf.sed > $TMPFILE
					/usr/bin/X -br :$1 vt07 -audit 0 -nolisten tcp -config $TMPFILE >> /tmp/usbseat.log 2>&1 &
					PID=$!
					echo "Running X -br :$1 vt07 -audit 0 -nolisten tcp -config $TMPFILE in background PID=$PID" >> /tmp/usbseat.log
					# kill this Xorg before 5secs and relaunch all multiseats
					(sleep 5 && kill $PID && start-multiseat && tree /dev/usbseat/$1 >> /tmp/usbseat.log) &
				fi
			fi
		fi
		#if [ "$1" = "1" ] && [ -e /dev/usbseat/$1/keyboard ] && [ /dev/usbseat/$1/mouse ]; then
		#	# start local dynamic
		#	PID=$(ps aux| grep "X :0" | grep -v grep| awk '{print $2}')
		#	[ -n $PID ] && kill -9 $PID || true
		#	/usr/bin/gdmdynamic -v -t 2 -s 1 -a "0=/usr/bin/X -br :0 vt07 -audit 0 -nolisten tcp -config /lib/udev/xorg.conf.display0"
		#fi
		;;
esac

exit 0


