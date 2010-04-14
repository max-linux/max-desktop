#!/bin/sh

#ORIG_PATH=/home/mario/MaX/build/ubiquity/ubiquity-slideshow-ubuntu-19/
ORIG_PATH=/home/mario/MaX/build/ubiquity/ubiquity-slideshow-ubuntu-21/

for f in $(find -type f | grep -v ".svn"); do

  e=$(basename $f)
  if [ "$e" = "do_option" ]; then
    orig=$ORIG_PATH/d-i/source/partman-auto/automatically_partition/resize_use_free/do_option
  elif [ "$e" = "ubiquity-maxui.desktop" ]; then
    orig=$ORIG_PATH/desktop/ubiquity-gtkui.desktop.in
  else
    orig=$(find $ORIG_PATH -type f -name "$e"| head -1)
  fi

  if [ "$orig" = "" ]; then
    echo "WARNING: orig not found $f" >&2
    continue
  fi
  #echo diff -ur $orig $f >&2
  echo meld $orig $f >&2
  diff -ur $orig $f


done
