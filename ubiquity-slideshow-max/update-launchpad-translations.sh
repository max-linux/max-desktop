#!/bin/sh
#
# «update-launchpad-translations» - Merge a Launchpad translations export with
# a specified branch.
#
# Copyright (C) 2010 Canonical Ltd.
#
# Authors:
#
# - Evan Dandrea <evand@ubuntu.com>
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
# 
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.

WORKING_DIR="/tmp/launchpad-export"
if [ -z "$1" ] || [ -z "$2" ]; then
	echo "$0 launchpad-export.tar.gz path-to-slideshow-repo"
	exit 1
fi
rm -rf $WORKING_DIR
mkdir -p $WORKING_DIR
tar -C $WORKING_DIR -zxvf $1
save="$pwd"
cd $WORKING_DIR/po
for distro in ubuntu kubuntu; do
	cd $distro
	for d in *; do
		cd $d; rename 's/.*-//' *.po; cd ..
	done
	rm */*.pot
	for d in $(find -name *.po | sed "s,.*/\(.*\)\.po$,\1," | sort | uniq); do
		msgcat --use-first */$d.po > $2/po/$distro/$d.po
	done
	cd ..
done
cd $save
rm -rf $WORKING_DIR

for d in $2/po/*; do
	[ -d $d ] || continue
	for p in $d/*.po; do
		[ -e $p ] || continue
		msgmerge -U $p $d/slideshow-*.pot
	done
done
