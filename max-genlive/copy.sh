#!/bin/sh


for f in $(svn status | awk '{print $2}'| grep ^usr); do

  echo cp $f /$f
  cp $f /$f

done
