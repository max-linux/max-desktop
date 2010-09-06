#!/bin/bash

VERSION=$(dpkg-parsechangelog | awk '/^Version/ {print $2}')

[ "$1" != "nobuild" ] && debuild -us -uc -I

if [ "$1" != "noupload" ]; then

  rsync --bwlimit=70 -Pavz ../*${VERSION}* max.educa.madrid.org:/usr/local/max/logs/trac/incoming/branches/max-ebox/
  ssh max.educa.madrid.org -t /usr/local/max/logs/root/bin/inject_incoming branches/max-ebox

fi


fakeroot debian/rules clean
