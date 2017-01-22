#!/bin/sh

D=$1

if [ "$1" = "" ]; then
  echo " Error: need path as argument"
  exit
fi


for p in $(find ${D} -name "*i386.deb"); do

  x64=$(echo $p| sed -e 's/i386/amd64/g')
  if [ ! -e "$x64" ]; then
    echo " NO EXISTS $x64"
  fi

done


