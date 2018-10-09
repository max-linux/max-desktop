#!/bin/sh

#ORIG_PATH=/home/madrid/MaX/build/ubiquity/ubiquity-2.10.20/
# for precise
#ORIG_PATH=/data/max/build/desktop/ubiquity/ubiquity-2.10.28/
# for trusty
#ORIG_PATH=/data/max/build/desktop/ubiquity/ubiquity-2.18.8.7/
#ORIG_PATH=/data/max/build/desktop/ubiquity/ubiquity-2.18.8.8/
#ORIG_PATH=/data/max/build/desktop/ubiquity/ubiquity-2.21.63.2max1/
ORIG_PATH=/data/max/build/desktop/ubiquity/ubiquity-18.04.14.8/

#for f in $(find usr/ -type f -name "*py"); do
for f in $(find usr/ -type f | grep -v ".svn"); do

  e=$(basename $f)
  if [ "$e" = "do_option" ]; then
    orig=$ORIG_PATH/d-i/source/partman-auto/automatically_partition/resize_use_free/do_option
  elif [ "$e" = "ubiquity-maxui.desktop" ]; then
    orig=$ORIG_PATH/desktop/ubiquity-gtkui.desktop.in
  elif [ "$e" = "ubiquity" ]; then
    orig=$ORIG_PATH/bin/ubiquity
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
