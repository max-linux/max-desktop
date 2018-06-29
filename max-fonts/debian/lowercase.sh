#!/bin/sh

TARGET=$1

for f in $(find "$TARGET" -type f); do

  lcase=$(echo $f| sed 's/\.TTF$/\.ttf/')
  if [ "$f" != "$lcase" ]; then
    mv -v "${f}" "${lcase}"
  fi
done
