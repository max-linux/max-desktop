#!/bin/bash

FILES=644
DIRS=755

get_line(){
head -$1 $2|tail -1
}

find max-win32-apps -type f > files.tmp
NUM=$(cat files.tmp | wc -l)
for i in $(seq $NUM); do
  line=$(get_line $i files.tmp)
  chmod $FILES "$line"
done

find max-win32-apps -type d > dirs.tmp
NUM=$(cat dirs.tmp | wc -l)
for i in $(seq $NUM); do
  line=$(get_line $i dirs.tmp)
  chmod $DIRS "$line"
done


rm -f files.tmp
rm -f dirs.tmp

