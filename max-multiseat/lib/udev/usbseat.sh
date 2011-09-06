#!/bin/sh
# takes the "seat number" as parameter $1
# the seat number is the kernel device id of the hub the seat's devices are sitting off of
# called once for every usb device that MIGHT be part of a seat, when they arrive or remove 

echo "-----------DISPLAY='$1'---ACTION='$ACTION'----DEVICE='$2'-------------" >> /tmp/usbseat.log

if [ "$1" = "0" ] || [ "$1" = "1"  ]; then
  echo "NOT make things in DISPLAY $1" >> /tmp/usbseat.log
  exit 0
fi

env | grep -e ^ID -e ^DEV -e ^SEAT_ID >> /tmp/usbseat.log

#if [ "$ID_VENDOR_ID" = "0711" ]; then
#	# $1 could be 'display'
#	if [ "$1" = "display" ]; then
#		echo "UPSSS , error: \$1 is display, use SEAT_ID=$SEAT_ID" >> /tmp/usbseat.log
#	else
#		SEAT_ID=$1
#	fi
#
#	if [ -e /dev/usbseat/$SEAT_ID/keyboard ] && [ -e /dev/usbseat/$SEAT_ID/mouse ] && [ -e /dev/usbseat/$SEAT_ID/display ]; then
#		echo "Device $BUSNUM $DEVNUM complete, no MWS300-init-tool" >> /tmp/usbseat.log
#	else
#		tree /dev/usbseat/$SEAT_ID >> /tmp/usbseat.log
#		echo "Call MWS300-init-tool $BUSNUM $DEVNUM ...." >> /tmp/usbseat.log
#		[ "$ID_MODEL_ID" = "5100" ] && /lib/udev/MWS300-init-tool $BUSNUM $DEVNUM >> /tmp/usbseat.log 2>&1
#		[ "$ID_MODEL_ID" = "5500" ] && /lib/udev/multiseat-init-tool.v2 >> /tmp/usbseat.log 2>&1
#		# exit now
#		exit 0
#	fi
#fi

if ! pidof gdm >/dev/null; then
    exit 0
fi

#GDMDYNAMIC="/usr/bin/gdmdynamic -v"
GDMDYNAMIC="/usr/bin/gdmdynamic "

(
# lock in FD 8 for 4 seconds
flock -x -w 4 8

seat_running=`/usr/bin/gdmdynamic -l | /bin/sed -n -e "/:$1,/p"`
DISPLAY_NUMBER=$(echo $1| sed -e 's/://g')

# $ACTION environment variable is set by udev subsystem
case "$ACTION" in
	'remove')
		exit 0
		;;
	*)
		# if we already have a running seat for this #, exit
		if [ -n "${seat_running}" ]; then
			if [ ! -f "/tmp/.X${1}-lock" ]; then
				echo "SEAT_ID $1 running, but no /tmp/.X${1}-lock, last lines of Xorg.log" >> /tmp/usbseat.log
				# seat running but Xorg no running
				tail -50 /var/log/Xorg.${1}.log >> /tmp/usbseat.log 2>/dev/null
				# no exit now, exec rest of script
			else
				echo "SEAT_ID $1 running, exit now" >> /tmp/usbseat.log
				exit 0
			fi
		fi
		#PID=$(ps aux| grep "usbseat-gdm-remover /dev/usbseat/$1/sound $1"| grep -v grep)
		#if [ "$PID" != "0" ]; then
		#	echo "SEAT_ID $1 running (PID=$PID), exit now" >> /tmp/usbseat.log
		#	exit 0
		#fi

		if [ -e /dev/usbseat/$1/keyboard ] && \
		   [ -e /dev/usbseat/$1/mouse ] &&  \
		   [ -e /dev/usbseat/$1/display ]; then

			# We have a newly complete seat. Start it.
			TMPFILE=`/bin/mktemp -t usbseat.XXXXXXXXXX` || exit 1
			
			# search for a MWS300 device
			MWS=$(udevadm info --query=env --name=/dev/usbseat/$1/display | grep -e ID_VENDOR_ID=0711 -e ID_MODEL_ID=5)
			if [ "$MWS" != "" ]; then
			    export $MWS
			fi
			
			if [ "$ID_VENDOR_ID" = "0711" ]; then
				echo "MWS300 complete, launching GDMdynamic in $1 with $TMPFILE" >> /tmp/usbseat.log

				BUSNUM=$(readlink -f /dev/usbseat/$1/display | awk -F"/" '{print $5}')
				DEVNUM=$(readlink -f /dev/usbseat/$1/display | awk -F"/" '{print $6}')
				if [ "$ID_MODEL_ID" = "5500" ]; then
					/bin/sed -e "s/%ID_SEAT%/$1/g" -e "s|%BUSNUM%|$BUSNUM|g" -e "s|%DEVNUM%|$DEVNUM|g"  /lib/udev/usbseat-xf86.tusb5500.conf.sed > $TMPFILE
				else # MODEL=5100
					/bin/sed -e "s/%ID_SEAT%/$1/g" -e "s|%BUSNUM%|$BUSNUM|g" -e "s|%DEVNUM%|$DEVNUM|g"  /lib/udev/usbseat-xf86.tusb.conf.sed > $TMPFILE
				fi
			else
				/bin/sed "s/%ID_SEAT%/$1/g" < /lib/udev/usbseat-xf86.conf.sed > $TMPFILE
			fi
			
			tree /dev/usbseat/$1 >> /tmp/usbseat.log
			if [ ! -e /dev/localseat ]; then
				$GDMDYNAMIC -t 2 -s 1 -a "$1=/usr/bin/X -br :$1 vt07 -audit 0 -nolisten tcp -config $TMPFILE"
			else
				# -novtswitch -sharevts when LocalSeat is enabled
				$GDMDYNAMIC -t 2 -s 1 -a "$1=/usr/bin/X -br :$1 vt07 -audit 0 -nolisten tcp -novtswitch -sharevts -config $TMPFILE"
			fi

			$GDMDYNAMIC -r $1
			
			# kill OLD removers
			PIDS=$(pgrep -f "/usr/sbin/usbseat-gdm-remover /dev/usbseat/$1/sound $1")
			if [ "$PIDS" != "" ]; then
				echo "killing OLD usbseat-gdm-remover in $1" >> /tmp/usbseat.log 2>&1
				pgrep -f -l "/usr/sbin/usbseat-gdm-remover /dev/usbseat/$1/sound $1" >> /tmp/usbseat.log 2>&1
				kill $PIDS >> /tmp/usbseat.log 2>&1
			fi
			# call $GDMDYNAMIC -d $1 when sound device disappear (fork)
			/usr/sbin/usbseat-gdm-remover /dev/usbseat/$1/sound $1 >> /tmp/usbseat.log 2>&1 &
		else
			if [ "$1" = "1" ]; then
				$GDMDYNAMIC -t 2 -s 1 -a "$1=/usr/bin/X -br :$1 vt07 -audit 0 -nolisten tcp -config /lib/udev/xorg.conf.display0"
				$GDMDYNAMIC -r $1
				exit
			fi
			echo "Some devices not found" >> /tmp/usbseat.log
			tree /dev/usbseat/$1 >> /tmp/usbseat.log
		fi
		;;
esac

) 8>/tmp/usbseat.lock

rm -f /tmp/usbseat.lock



