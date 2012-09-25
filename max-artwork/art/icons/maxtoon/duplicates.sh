#!/bin/bash

DIR="./scalable"
if [ "$1" != "" ]; then
  DIR="$1"
fi

find $DIR -type f -name "*.svg" -exec md5sum {} \; | sort > /tmp/sums-sorted.txt

OLDSUM=""
IFS=$'\n'

for i in `cat /tmp/sums-sorted.txt`; do
 NEWSUM=`echo "$i" | sed 's/ .*//'`
 NEWFILE=`echo "$i" | sed 's/^[^ ]* *//'`
 if [ "$OLDSUM" == "$NEWSUM" ]; then
  echo ln -f "$OLDFILE" "$NEWFILE"
  if [ "$DO" = "1" ]; then
    rm -f "$NEWFILE"
    ln -s "$OLDFILE" "$NEWFILE"
  fi
 else
  OLDSUM="$NEWSUM"
  OLDFILE="$NEWFILE"
 fi
done
