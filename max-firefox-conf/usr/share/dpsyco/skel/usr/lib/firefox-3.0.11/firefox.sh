#!/bin/sh

# MAX 5.0 install FNMT CA certificate

if [ -e $HOME/.mozilla/firefox/profiles.ini ]; then

  MAX_PROFILE="$HOME/.mozilla/firefox/"$(awk -F"=" '/Path/ {print $2}' ~/.mozilla/firefox/profiles.ini)
  if [ ! -e "$MAX_PROFILE/cert8.db" ]; then
    # copy cert8.db to home profile
    cp /usr/share/max-firefox-conf/cert8.db "$MAX_PROFILE/cert8.db"
    
    # set correct permissions
    MAX_UID=$(env | awk -F"=" '/SUDO_UID=/ {print $2}')
    if [ "$MAX_UID" = "" ]; then
       MAX_UID=$(id -u)
    fi
    chown ${MAX_UID}:${MAX_UID} "$MAX_PROFILE/cert8.db"
    chmod 600 "$MAX_PROFILE/cert8.db"
  fi
fi



# Firefox launcher containing a Profile migration helper for
# temporary profiles used during alpha and beta phases.

# Authors:
#  Alexander Sack <asac@jwsdot.com>
#  Fabien Tassin <fta@sofaraway.org>
#  Steve Langasek <steve.langasek@canonical.com>
# License: GPLv2 or later

MOZDIR=$HOME/.mozilla
LIBDIR=/usr/lib/firefox-3.0.11
APPVER=3.0
META_NAME=firefox
GDB=/usr/bin/gdb
DROPPED=abandoned

NAME="$0"
if [ "x$META_NAME" != "x" ] ; then
  NAME="${NAME%%-$APPVER}"
fi
APPNAME="$(basename "$NAME")"

while [ ! -f "$LIBDIR/$APPNAME" ] && [ -L "$NAME" ]; do
  TARGET="$(readlink "$NAME")"
  if [ "x$TARGET" = "x$(basename "$TARGET")" ]; then
    TARGET="$(dirname "$NAME")/$TARGET"
  fi
  if [ "x$META_NAME" != "x" ] ; then
    TARGET=${TARGET%%-$APPVER}
  fi
  NAME="$TARGET"
  APPNAME="$(basename "$NAME")"
  if [ "x$APPNAME" = "xfirefox.sh" ]; then
    APPNAME=firefox
    break
  fi
done

usage () {
  $LIBDIR/$APPNAME -h | sed -e 's,/.*/,,'
  echo
  echo "        -g or --debug		Start within $GDB (Must be first)"
}

want_debug=0
while [ $# -gt 0 ]; do
  case "$1" in
    -h | --help | -help )
      usage
      exit 0 ;;
    -g | --debug )
      want_debug=1
      shift ;;
    -- ) # Stop option prcessing
      shift
      break ;;
    * )
      break ;;
  esac
done

# if there exists a beta profile (first found: $MOZDIR/firefox-3.0,
# $MOZDIR/firefox-trunk, $MOZDIR/granparadiso) and there is a standard
# firefox profile as well, ask the user what to do. In case he decides to
# import the firefox 2 profile settings, we keep the firefox directory
# untouched, but rename the beta profile by appending the suffix
# |.abandoned|. In case the user decides to keep using the firefox 3
# profile, we will rename the original firefox profile to firefox.2-replaced
# and rename the beta profile to be |.mozilla/firefox|.
#
# as a third option the user can defer his final decision. This will leave the
# directories untouched, thus making the user use the old firefox profile by
# default.
#
# in addition, even older profiles are renamed too, by appending |.abandoned|
# to their name.

FOUND=""
if [ -d $MOZDIR/firefox ] ; then
  FOUND=firefox
fi

FOUND_BETA=""
BETA_LIST=""
for betaname in granparadiso firefox-trunk firefox-3.0; do
  if [ -d $MOZDIR/$betaname ]; then
    BETA_LIST="$BETA_LIST $betaname"
    FOUND_BETA=$betaname
  fi
done

if [ "$FOUND" != "" -a "$FOUND_BETA" != "" ] ; then
  echo -n "Found Beta Participation ..."
  $LIBDIR/ffox-3-beta-profile-migration-dialog
  result=$?
  if [ $result = 1 ]; then
     mv $MOZDIR/$FOUND $MOZDIR/$FOUND.2-replaced
     mv $MOZDIR/$FOUND_BETA $MOZDIR/$FOUND
     for beta in $BETA_LIST ; do
       if [ $beta != $FOUND_BETA ] ; then
         mv $MOZDIR/$beta $MOZDIR/$beta.$DROPPED
       fi
     done
     echo " keep beta profile."
  elif [ $result = 2 ]; then
     for beta in $BETA_LIST ; do
       mv $MOZDIR/$beta $MOZDIR/$beta.$DROPPED
     done
     echo " use firefox 2 profile."
  else
     echo " use firefox 2 profile, but will ask again next time."
  fi
  echo " ... will check again next time."
elif [ "$FOUND_BETA" != "" -a "$FOUND" = "" ]; then
  # in case we only have a beta profile the user most likely wants to use
  # that. just doing, no questions asked.
  mv $MOZDIR/$FOUND_BETA $MOZDIR/firefox
  for beta in $BETA_LIST ; do
    # Move out the older beta profiles
    if [ $beta != $FOUND_BETA ] ; then
      mv $MOZDIR/$beta $MOZDIR/$beta.$DROPPED
    fi
  done
  echo "*NOTICE* Profile $FOUND_BETA found and moved as main profile"
fi

if [ $want_debug -eq 1 ] ; then
  if [ ! -x $GDB ] ; then
    echo "Sorry, can't find usable $GDB. Please install it."
    exit 1
  fi
  tmpfile=`mktemp /tmp/mozargs.XXXXXX` || { echo "Cannot create temporary file" >&2; exit 1; }
  trap " [ -f \"$tmpfile\" ] && /bin/rm -f -- \"$tmpfile\"" 0 1 2 3 13 15
  echo "set args ${1+"$@"}" > $tmpfile
  echo "$GDB $LIBDIR/$APPNAME -x $tmpfile"
  $GDB "$LIBDIR/$APPNAME" -x $tmpfile
  exit $?
else
  exec $LIBDIR/$APPNAME "$@"
fi
