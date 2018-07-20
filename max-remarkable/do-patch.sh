#!/bin/sh
set -e

cd "$1"

for p in $(find ../patches -type f); do

	echo "cat $p| patch -p1"
	cat $p | patch -p1

done
