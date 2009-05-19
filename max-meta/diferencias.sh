#!/bin/bash

DESKTOPS="common gnome kde xfce"

get_line() {
head -$1 $2| tail -1
}

rm -f desktop-all-i386
rm -f desktop-dupli-i386
rm -f desktop-dupli-all-i386


for desktop in $DESKTOPS; do
  cat desktop-${desktop}-i386 >> desktop-all-i386
  for line in $(cat desktop-${desktop}-i386); do
   [ "$line" != "" ] && /bin/echo -e "$line\t\t\t\t$desktop" >> desktop-dupli-i386
  done
done


  cat desktop-dupli-i386 | sort -n > tmp
  mv tmp desktop-dupli-i386

  lines=$(cat desktop-dupli-i386| wc -l)

  for line in $(seq $lines); do
    line=$(get_line $line desktop-dupli-i386)
    pkg=$(/bin/echo $line| awk '{print $1}')
    if [ "$pkg" = "$old_pkg" ]; then
     #echo "DUPLI: PKG= \"$pkg\" OLD-PKG=\"$old_pkg\" $line"
     /bin/echo "DUPLI: $line ### $old_line"
    fi
    old_pkg=$pkg
    old_line=$line
  done
  cat desktop-dupli-i386 | sort -n | uniq > tmp
  mv tmp desktop-sin-dupli-i386



  cat desktop-all-i386 | sort -n  |uniq > tmp
  mv tmp desktop-all-i386
