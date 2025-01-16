#!/usr/sbin/bash

set -e

for u in $( awk -F":" '/\/bin\/bash/ {if($3>999) print $1}' /etc/passwd ); do
	
	if [ -f /home/"$u"/Escritorio/AulaVirtual-Examen.desktop ]; then 
	
	rm -f /home/"$u"/Escritorio/AulaVirtual-Examen.desktop
	
	fi 
done
