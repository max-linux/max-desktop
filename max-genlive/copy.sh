#!/bin/sh


for f in $(LC_ALL=C git status ./usr | grep modified| awk '{print $NF}'); do

  echo cp $f /$f
  cp $f /$f

done
