#!/bin/sh

#ORIG_PATH=/home/mario/MaX/build/ubiquity/ubiquity-2.2.15/
#ORIG_PATH=/home/mario/MaX/build/ubiquity/ubiquity-2.2.16/
#ORIG_PATH=/home/mario/MaX/build/ubiquity/ubiquity-2.2.18/
#ORIG_PATH=/home/mario/MaX/build/ubiquity/ubiquity-2.2.23/
#ORIG_PATH=/home/mario/MaX/build/ubiquity/ubiquity-2.2.24/
#ORIG_PATH=/home/mario/MaX/build/ubiquity/ubiquity-2.2.25/
#ORIG_PATH=/home/madrid/MaX/build/ubiquity/ubiquity-2.2.27/
ORIG_PATH=/home/madrid/MaX/build/ubiquity/ubiquity-2.9.10/

#for f in $(find usr/ -type f -name "*py"); do
for f in $(find usr/ -type f | grep -v ".svn"); do

  e=$(basename $f)
  if [ "$e" = "do_option" ]; then
    orig=$ORIG_PATH/d-i/source/partman-auto/automatically_partition/resize_use_free/do_option
  elif [ "$e" = "ubiquity-maxui.desktop" ]; then
    orig=$ORIG_PATH/desktop/ubiquity-gtkui.desktop.in
  else
    orig=$(find $ORIG_PATH -type f -name "$e"| head -1)
  fi

  if [ "$orig" = "" ]; then
    #echo "WARNING: orig not found $f" >&2
    continue
  fi
  #echo diff -ur $orig $f >&2
  echo meld $orig $f >&2
  diff -ur $orig $f


done
