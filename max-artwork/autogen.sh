#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="max-artwork"

(test -f "$srcdir/configure.in" && test -d "$srcdir/art") || {
  echo "$srcdir doesn't look like source directory for $PKG_NAME" >&2
  exit 1
}

which gnome-autogen.sh || {
  echo "You need to install gnome-common to build $PKG_NAME" >&2
  exit 1
}

REQUIRED_AUTOMAKE_VERSION=1.8
export REQUIRED_AUTOMAKE_VERSION

USE_GNOME2_MACROS=1 . gnome-autogen.sh
