#!/bin/sh
set -e

VERSION=$(dpkg-parsechangelog | awk '/^Version/ {print $2}')

[ "$1" != "nobuild" ] && debuild -us -uc -I

[ "$1" != "noupload" ] && rsync --bwlimit=70 -Pavz ../*${VERSION}* max.educa.madrid.org:/usr/local/max/logs/trac/incoming/branches/max-zentyal/
[ "$1" != "noupload" ] && ssh max.educa.madrid.org -t /usr/local/max/logs/root/bin/inject_incoming branches/max-zentyal

fakeroot debian/rules clean

rm -fv ../*${VERSION}*
