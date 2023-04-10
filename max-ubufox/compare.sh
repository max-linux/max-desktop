#!/bin/sh

#ORIG_PATH=/home/mario/MaX/build/ubufox/ubufox-0.9~rc1/
ORIG_PATH=/home/madrid/MaX/build/firefox4/ubufox-0.9/
ORIG_PATH=/home/madrid/MaX/build/ubufox/ubufox-1.5/
ORIG_PATH=/data/max/build/desktop/ubufox/ubufox-3.0/
ORIG_PATH=/data/max/build/desktop/ubufox/ubufox-3.2/
ORIG_PATH=/data/max/build/desktop/ubufox/ubufox-3.4/

#for f in $(find usr/ -type f -name "*py"); do
for e in $(find $ORIG_PATH -type f | grep -v -e ".svn" -e ".pc"); do

  orig=$e
  f=$(echo $e | sed -e s@$ORIG_PATH@''@g)


  if [ "$orig" = "" ]; then
    echo "WARNING: orig not found $f" >&2
    continue
  fi


  #echo diff -ur $orig $f >&2
  cmp $orig $f || echo meld $orig $f >&2
  #cmp $orig $f && echo "INFO: same file $orig $f" >&2
  diff -ur $orig $f


done
