#!/bin/sh

if [ -d usr/share/dpsyco/skel/usr/share/applications/ ]; then
  DIR="usr/share/dpsyco/skel/usr/share/applications/"
else
  DIR="/usr/share/dpsyco/skel/usr/share/applications/"
fi

for f in $DIR/*desktop; do

  fname=$(basename $f)
  tryexec=$(awk -F"=" '/^TryExec/ {print $2}' $f)
  if [ "$tryexec" = "" ]; then
    echo " * $fname: no TryExec line."
  elif [ ! -e "$tryexec" ]; then
    echo " * $fname: TryExec '$tryexec' not found."
  fi

done
