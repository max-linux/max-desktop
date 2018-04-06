#!/bin/sh


cd blocklyduino


for f in `find ../patches/ -type f`; do
  echo "cat '$f' | patch"
  cat "$f" | patch
done
