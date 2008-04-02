#!/bin/bash

VERSION=__VERSION__


decho() {
  echo "maxmenus:: $@" >&2
}

usage() {
  echo "$0 usage:"
  echo "           $0 --update (update all home menus)"
  echo "           $0 --user  (update current user menus)"
  echo "           $0 --purge (try to remove all home menus)"
}

case $1 in
   --update)
     ACTION=update
     ;;
   --purge)
     ACTION=purge
     ;;
   --user)
     ACTION=user
     ;;
   *)
     usage
     exit 1
     ;;
esac

# salir si start-stop-daemon es un script
if [ $(file /sbin/start-stop-daemon | grep -c ELF) != 0 ]; then
  echo "No ejecutando.... start-stop-daemon no es un binario"
  exit 0
fi

if [ "$ACTION" != "user" ] && [ $(id -u) != 0 ]; then
  echo "No eres usuario root. Permiso denegado."
  exit 1
fi


USER_MENUS=".config/menus"
USER_LOCAL=".local/share/desktop-directories"
SKEL_MENUS="/etc/skel/.config/menus"
SKEL_LOCAL="/etc/skel/.local/share/desktop-directories"

if [ "$ACTION" = "user" ]; then
  mkdir -p $HOME/$USER_MENUS
  mkdir -p $HOME/$USER_LOCAL

  rm -f $HOME/$USER_MENUS/*
  rm -f $HOME/$USER_LOCAL/*

  for file in $SKEL_MENUS/*; do
      cp $file $HOME/$USER_MENUS/
  done

  for file in $SKEL_LOCAL/*; do
      cp $file $HOME/$USER_LOCAL/
  done

  # check for permissions
  chown -R $USER $HOME/$USER_MENUS 2>/dev/null
  chown -R $USER $HOME/$USER_LOCAL 2>/dev/null
  exit 0
fi

for home in $(find /home/ -maxdepth 1 -mindepth 1 -type d); do

    user=$(basename $home)
    if [ "$user" = "ftp" ]; then
      # no updating ftp user
      continue
    fi

    # look for a lock file
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
