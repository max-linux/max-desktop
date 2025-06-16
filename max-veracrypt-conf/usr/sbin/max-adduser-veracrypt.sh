#!/bin/bash


for u in $( awk -F":" '/\/bin\/bash/ { if($3>999) print $1}' /etc/passwd ); do

	grep veracrypt /etc/group| grep -q "$u" || adduser "$u" veracrypt

done

