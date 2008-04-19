#!/bin/sh -e

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

distributionIDs="7.10 8.04"


for distro in $distributionIDs; do

  echo   ... for $distro ...
  cp sources.list.$distro sources.list
  python plugindb.py $distro

done

echo
echo
echo -- Done
