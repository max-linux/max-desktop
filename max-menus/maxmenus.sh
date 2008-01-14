#!/bin/bash

VERSION=__VERSION__

if [ $(id -u) != 0 ]; then
  echo "You aren't root user. Access denied."
  exit 1
fi

decho() {
  echo "maxmenus:: $@" >&2
}

usage() {
  echo "$0 usage:"
  echo "           $0 --update (update all home menus)"
  echo "           $0 --purge (try to remove all home menus)"
}

case $1 in
   --update)
     ACTION=update
     ;;
   --purge)
     ACTION=purge
     ;;
   *)
     usage
     exit 1
     ;;
esac

USER_MENUS=".config/menus"
USER_LOCAL=".local/share/desktop-directories"
SKEL_MENUS="/etc/skel/.config/menus"
SKEL_LOCAL="/etc/skel/.local/share/desktop-directories"

for home in $(find /home/ -maxdepth 1 -mindepth 1 -type d); do

    user=$(basename $home)

    # look for lock file
    if [ -e $home/.maxmenus.lock ]; then
      decho "No updating menus, found $home/.maxmenus.lock"
      continue
    fi

    mkdir -p $home/$USER_MENUS
    mkdir -p $home/$USER_LOCAL

    # clean dirs
    rm -f $home/$USER_MENUS/*
    rm -f $home/$USER_LOCAL/*

    for file in $SKEL_MENUS/*; do
      [ "$ACTION" = "update" ] && cp $file $home/$USER_MENUS/
      [ "$ACTION" = "purge" ] &&  rm -f $home/$USER_MENUS/$(basename $file)
    done

    for file in $SKEL_LOCAL/*; do
      [ "$ACTION" = "update" ] && cp $file $home/$USER_LOCAL/
      [ "$ACTION" = "purge" ] &&  rm -f $home/$USER_LOCAL/$(basename $file)
    done

    # check for permissions
    chown -R $user $home/$USER_MENUS 2>/dev/null
    chown -R $user $home/$USER_LOCAL 2>/dev/null

done # end of for $home
