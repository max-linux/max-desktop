#!/bin/bash
#
# Copyright © 2006 Dennis Kaarsemaker <dennis@kaarsemaker.net>
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
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

set -e

while [ ! -z $1 ]; do
    case $1 in
        -v)
            VERBOSE=-v
            shift
            ;;
        -x)
            shift
            X="-x $1"
            shift
            ;;
        -y)
            shift
            Y="-y $1"
            shift
            ;;
        *)
            echo >&2 "Error: unknown argument $1"
            exit 1
            ;;
    esac
done

usplash -c $X $Y $VERBOSE  2>&1 >/dev/null &
pid=$!
sleep 3

for i in `seq 3`; do
    usplash_write "TEXT loop iteration $i"
    usplash_write "SUCCESS ok"
    usplash_write "TEXT loop iteration $i"
    usplash_write "FAILURE fail"
    usplash_write "PROGRESS $((33*$i))"
    sleep 1
done

usplash_write "QUIT"

