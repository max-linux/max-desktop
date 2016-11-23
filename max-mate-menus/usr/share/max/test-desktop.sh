#!/bin/sh

if [ -d usr/share/mate/applications/ ]; then
  DIR="usr/share/mate/applications/"
else
  DIR="/usr/share/mate/applications/"
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
