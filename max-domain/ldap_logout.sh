#!/bin/bash
# force kill apps to umount clean

# only run if user is in domain
id | grep -q "domain^users" || exit 0


pid=`ps ax | grep gdm-session-worker | grep -v grep | awk '{print $1}'`
kill -9 $pid >/dev/null 2>&1

killall dbus-daemon >/dev/null 2>&1


( sleep 1 & killall -9 pulseaudio  >/dev/null 2>&1 ) &

umount ~/.gvfs >/dev/null 2>&1

PIDS=$(lsof $HOME/ | grep -v "bash" | awk '{print $2}' | grep [0-9] | sort | uniq )

(sleep 1 && for pid in $PIDS; do kill -9 $pid >/dev/null 2>&1; done ) &



