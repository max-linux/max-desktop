#!/bin/sh -e
#
# Copyright (C) 2007-2008  Canonical Ltd.
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
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

set -e

DB_FILENAME=mozilla-plugin-data.sqlite3db

echo -- Wiping DB ...
echo
echo

rm -rfv ${DB_FILENAME}

echo -- Recreating Tables ...
echo
echo

cat recreate_tables.sql | sqlite3 -echo ${DB_FILENAME}

echo
echo
echo -- Inserting Data ...
echo
echo

distributionIDs="8.04 8.10 9.04 9.10 10.04"

archs_7_10="amd64 i386 powerpc sparc"
archs_8_04="amd64 i386"
archs_8_10="amd64 i386"
archs_9_04="amd64 i386"
archs_9_10="amd64 i386"
archs_10_04="amd64 i386"

for distro in $distributionIDs; do

  echo   ... for $distro ...
  cp sources.list.$distro sources.list
  distroT=`echo $distro | tr '.' '_'`
  python plugindb.py $distro "`eval echo \\\$archs_$distroT`"

done

echo
echo
echo -- Done
