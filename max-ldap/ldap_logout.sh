#!/bin/bash
# force kill apps to umount clean

# only run if user is in domain
( id | grep -q "__USERS__" ) || exit 0


umount ~/.gvfs >/dev/null 2>&1

PIDS=$(lsof ~/ | grep -v "bash" | awk '{print $2}' | grep [0-9] | sort | uniq )

(sleep 1 && for pid in $PIDS; do kill -9 $pid; done ) &

( sleep 1 & killall -9 pulseaudio  >/dev/null 2>&1 ) &


