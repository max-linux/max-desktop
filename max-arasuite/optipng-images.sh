#!/bin/bash

DEST=$1

for p in $(find ${DEST} -name "*.png"| sort -h); do

  echo " * optipng ${p}"
  optipng -q "${p}"

done
