#!/bin/bash
###########################################################################
#
# Author: Mario Izquierdo (a.k.a mariodebian)
# Email: mariodebian at gmail.com
# Creation Date: 28 Dic 2007
#
#  Script que vuelca entradas de gconf (--get) y
#  inyecta entradas (--set) a partir de archivos con el formato:
#          clave_gconf tipo valor [prioridad]
#
#  prioridad es opcional, sino se pone se toma default, puede ser mandatory
#
#
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
###########################################################################

usage() {
  echo "Usage:"
  echo "       $0 --get /gconf/tree"
  echo "       $0 --set file_with_gconf_settings.gconf"
  echo "       $0 --unset file_with_gconf_settings.gconf"
}

decho() {
  echo "maxgconf::$@" >&2
}

if [ "$1" != "--get" ] && [ "$1" != "--set" ] && [ "$1" != "--unset" ]; then
  usage
  exit 1
fi

if [ "$2" = "" ]; then
  decho "ERROR: Need a second argument!!!"
  usage
  exit 1
fi

export LC_ALL=C
export LC_MESSAGES=C

get_line() {
  head -$1 $2| tail -1
}

get_subkeys() {
  KEYS=$(gconftool-2 --all-dirs $1)
  if [ "$KEYS" = "" ]; then
      return
  fi
  #echo "DEBUG: KEYS=$KEYS"
  for key in $KEYS; do
    #echo "  ** DEBUG key **: $key" >&2
    echo "$key"
    get_subkeys $key
  done
}

is_username() {
  id=$(grep "${1}:" /etc/passwd| awk -F':' '{print $3}')
  id=$(($id+0))
  if [ "$id" -gt 999 ]; then
    return 0
  else
    return 1
  fi
}

get_schema_type() {
 TYPE=$(gconftool-2 --dump $(dirname $1)|grep -A2 "$(basename $1)" |tail -1 |egrep "int|bool|float|string|list|pair")
 case $TYPE in
   *\<bool\>*)
      echo "bool"
      ;;
   *\<int\>*)
      echo "int"
      ;;
   *\<float\>*)
      echo "float"
      ;;
   *\<list*)
      echo "list"
      ;;
   *\<pair\>*)
      echo "pair"
      ;;
   *\<string\>*)
      echo "string"
      ;;
   *)
      echo ""
      ;;
 esac
}

set_key() {
   case $4 in
     *mandatory*)
         prio="mandatory"
         ;;
     *)
         prio="defaults"
         ;;
   esac

   if [ "$3" = "" ]; then decho "ERROR: key $1 don't have value"; return;  fi

   # set at gconf home settings before general
   for home in $(find /home/ -maxdepth 1 -mindepth 1 -type d); do
      username=$(basename $home)
      gconftool-2 --config-source xml:readwrite:$home/.gconf --type $2 --set $1 "$3" >> /tmp/maxgconf.errors 2>&1
      is_username "$username" && chown -R $username $home/.gconf >> /tmp/maxgconf.errors 2>&1
      is_username "$username" && chown -R $username:$username $home/.gconf >> /tmp/maxgconf.errors 2>&1
   done

   gconftool-2 --direct --type $2 --config-source xml:readwrite:/etc/gconf/gconf.xml.${prio} --set $1 "$3" >> /tmp/maxgconf.errors 2>&1
}

unset_key() {
   # set at gconf home settings before general
   for home in $(find /home/ -maxdepth 1 -mindepth 1 -type d); do
      username=$(basename $home)
      gconftool-2 --direct --config-source xml:readwrite:$home/.gconf --unset $1  >> /tmp/maxgconf.errors 2>&1
      is_username "$username" && chown -R $username $home/.gconf >> /tmp/maxgconf.errors 2>&1
      is_username "$username" && chown -R $username:$username $home/.gconf >> /tmp/maxgconf.errors 2>&1
   done

   gconftool-2 --direct --config-source xml:readwrite:/etc/gconf/gconf.xml.defaults --unset $1 >> /tmp/maxgconf.errors 2>&1
   gconftool-2 --direct --config-source xml:readwrite:/etc/gconf/gconf.xml.mandatory --unset $1 >> /tmp/maxgconf.errors 2>&1
}



if [ "$1" = "--get" ]; then
  KEYS=$(get_subkeys $2)
  if [ "$KEYS" = "" ]; then
    KEYS=$2
  fi
  for key in $KEYS; do
    VALUES=$(gconftool-2 -a $key > /tmp/gconf.tmp)
    lines=$(cat /tmp/gconf.tmp| wc -l)
    for i in $(seq $lines); do
      line=$(get_line $i /tmp/gconf.tmp)
      var=$(echo $line | awk '{print $1}')
      value=$(echo $line | awk '{print $3}')
      if [ "$value" = "(no" ]; then
        value=""
      fi
      type=$(get_schema_type $key/$var)
      if [ "$value" = "(no" ] || [ "$value" = "" ] ; then
         decho "  ** WARNING ** value of $key/$var not set"
      elif [ "$type" = "list" ]; then
         decho "  ** WARNING ** $key/$var type list not supported"
      elif [ "$type" = "" ]; then
         decho "  ** WARNING ** $key/$var empty type"
      else
         echo "$key/$var $type $value"
      fi
    done
    rm -f /tmp/gconf.tmp
  done
  exit 0
fi

if [ "$1" = "--set" ]; then
  if [ ! -e $2 ]; then
    decho "File $2 don't exists"
    exit 1
  fi
  decho "Reading \"$(basename $2)\" to set gconf values..."
  lines=$(cat $2| wc -l)
  for i in $(seq $lines); do
   line=$(get_line $i $2)
   var=$(echo $line | awk '{print $1}')
   type=$(echo $line | awk '{print $2}')
   # value is something like this "foo bar"
   if [ $(echo $line | grep -c '"') -gt 0 ]; then
     value=$(echo $line | awk -F'"' '{print $2}')
     prio=$(echo $line | awk -F '"' '{print $3}' | sed 's| ||g')
   else
     value=$(echo $line | awk '{print $3}')
     prio=$(echo $line | awk '{print $4}')
   fi

   # prio can be defaults or mandatory (empty defaults)
   set_key "$var" "$type" "$value" "$prio"
  done
  exit 0
fi

if [ "$1" = "--unset" ]; then
  if [ ! -e $2 ]; then
    decho "File $2 don't exists"
    exit 1
  fi
  decho "Reading \"$(basename $2)\" to unset gconf values..."
  lines=$(cat $2| wc -l)
  for i in $(seq $lines); do
   line=$(get_line $i $2)
   var=$(echo $line | awk '{print $1}')
   unset_key $var
  done
  exit 0
fi

exit 0
