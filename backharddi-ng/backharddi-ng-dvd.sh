#!/bin/bash

LOG=/var/log/backharddi-ng-dvd
rm $LOG
exec > >(while read a; do echo "$(date):" "$a"; done >>$LOG)
exec 2> >(while read a; do echo "$(date):" "$a"; done >>$LOG)
set -xv

TITLE="Backharddi NG - CD/DVD's de Recuperación"
TMP_DIR=/tmp/backharddi-ng
TMP_SIZE_LIST=/tmp/backharddi-ng_sizelist
TMP_RESPONSE=/tmp/backharddi-ng_response
TMP_FILE_LIST=/tmp/backharddi-ng_filelist
TMP_FILENAME=/tmp/backharddi-ng_filename
TMP_ISOLINUX_CFG=/tmp/backharddi-ng_isolinux.cfg
ISOLINUX_CFG=/usr/share/backharddi-ng/isolinux/isolinux.cfg 
MARCA=backharddi
NBSP=' '
TAB='	'
NL='
'
ERROR=/tmp/backharddi-ng_error


to_secure_string() {
        tr " " "_" | sed "s/[^a-zA-Z0-9ñÑçÇáéíóúàèìòù\+\.,:;-]/_/g"
}

to_original_string() {
	tr "_" " " 
}

humandev(){
	vendor="$(udevinfo -q env -n $1 | grep ID_VENDOR | cut -d = -f 2)"
	model="$(udevinfo -q env -n $1 | grep ID_MODEL | cut -d = -f 2)"
	[ -z $vendor ] && vendor="$(cat /sys/block/$(basename $1)/device/vendor)"
	[ -z $model ] && model="$(cat /sys/block/$(basename $1)/device/model)"
	echo "$vendor $model"
}

abort(){
	umount $TMP_DIR && rmdir $TMP_DIR
	rm $TMP_SIZE_LIST $TMP_RESPONSE $TMP_FILE_LIST* $TMP_FILENAME $TMP_ISOLINUX_CFG $ERROR 
	exit $1
}

error(){
	zenity --error --title $TITLE --text "Ha ocurrido un error en la generación del $MEDIO número $medio."
}
	 
cd /tmp
if ! grep -q $TMP_DIR /proc/mounts; then
	MSG="¿Desea generar la imagen desde un servidor de imágenes o desde un dispositivo local?"
	while true; do
		zenity --list --title "$TITLE" --text "$MSG" --column "Origen" "Servidor de imágenes" "Dispositivo local" > $TMP_RESPONSE || abort $?
		origen=$(cat $TMP_RESPONSE)
		case "$origen" in
			Servidor*)
				zenity --entry --title "$TITLE" --text "Seleccione el nombre o dirección ip del servidor:" --entry-text "" > $TMP_FILENAME || abort $?
				backuppart="-t nfs -o nolock,proto=tcp $(cat $TMP_FILENAME):$TMP_DIR"
			;;
			Dispositivo*)
				backuppart=$(findfs LABEL=$MARCA)
				if [ -z $backuppart ]; then
					MSG="No se ha encontrado ninguna partición de backup. ¿Desea generar la imagen desde un servidor de imágenes o desde un dispositivo local?" 
					continue
				fi
			;;
		esac

		[ -d $TMP_DIR ] || mkdir $TMP_DIR
		[ -f $TMP_SIZE_LIST ] && rm $TMP_SIZE_LIST
		mount $backuppart $TMP_DIR 2>/dev/null && break
		MSG="No se ha encontrado el servidor de imagenes en "$(cat $TMP_FILENAME)". ¿Desea generar la imagen desde un servidor de imágenes o desde un dispositivo local?"
	done
fi

dir=$TMP_DIR
while true; do
	for backup in $(ls $dir); do
        	[ -d "$dir/$backup" ] || continue
	        [ "$backup" = "boot" ] && continue
	        [ "$backup" = "lost+found" ] && continue
		case "$backup" in
	                +*) true ;;
	                *) ls "$dir/$backup"/=dev=* >/dev/null 2>&1 || continue ;;
	        esac
	        backup_nombre=$(echo $backup | to_original_string)
		backups="$backup_nombre
$backups"
	done
	
	if [ -z "$backups" -a "$dir" = "$TMP_DIR" ]; then
		zenity --info --title $TITLE --text "No se ha encontrado ninguna copia de seguridad en la partición de backup. Por favor, arranque el sistema seleccionando \"Sistema de Backup\" y genere una."
		exit 1
	fi
	
	IFS="$NL"
	zenity --list --width 400 --title $TITLE --text "Seleccione una Copia de Seguridad :" --column "Copias de Seguridad detectadas" $backups > $TMP_RESPONSE || abort $?
	backup=$dir/$(cat $TMP_RESPONSE | to_secure_string)
	if ls "$backup"/=dev=* >/dev/null 2>&1; then
		break;
	else
		unset backups
		dir="$backup"
	fi
done

IFS="$NL"
for f in $(find $backup -name img); do
	size=0
	for s in $(find ${f%img} -name img.[0-9][0-9] | sort); do
		filesize=$(du -b $s | cut -f 1)
		size=$((size+filesize))
	done
	echo "$f" "$size" >> $TMP_SIZE_LIST
	[ -z "$s" ] || echo "$(basename $s)" > $f
done

if [ ! -f $TMP_SIZE_LIST ]; then
	zenity --info --title $TITLE --text "No se ha encontrado ninguna imagen de partición en la copia de seguridad seleccionada. Por favor, arranque el sistema seleccionando \"Sistema de Backup\" y genere una."
	abort $?
fi

zenity --list --width 400 --height 200 --title $TITLE --text "Seleccione el tipo de Medio para generar los CD/DVD's de Recuperación del Sistema:" --column "Medio" "CD" "DVD" "DVD de máximo 4 GB" "DVD de doble capa" > $TMP_RESPONSE || abort $?
MEDIO="$(cat $TMP_RESPONSE)"
case "$MEDIO" in
	"CD") block_count=358400;;
	"DVD") block_count=2294291;;
	"DVD de máximo 4 GB") block_count=2097152;;
	"DVD de doble capa") block_count=4248046;;
esac

IFS=' '
if [ -f "$backup"/cmdline ]; then
	for x in $(cat "$backup"/cmdline); do
		case "$x" in
			BOOT_IMAGE=*) kernel=${x#BOOT_IMAGE=} ;;
			initrd=*) initrd=${x#initrd=} ;;
			video=*|vga=*|backharddi/*|--|netcfg/*) continue ;;
			*) params="$params $x" ;;
		esac
	done
fi
sed "s/DEFAULT\ local/DEFAULT\ cdrom/" "$ISOLINUX_CFG" > "$TMP_ISOLINUX_CFG"
cat >> $TMP_ISOLINUX_CFG <<EOT

LABEL cdrom
  menu label Recuperar Sistema
  kernel /boot/linux
  append initrd=/boot/initrd.gz backharddi/medio=cdrom backharddi/modo=rest backharddi/imagenes=/target/Imagenes video=vesa:ywrap,mtrr vga=788 $params --
EOT

IFS="$NL"
COUNT=1
file_list=$TMP_FILE_LIST$COUNT
cp /usr/share/backharddi-ng/backharddi-ng-dvd.filelist $file_list
for f in $(find $backup -type f ! -name *.[0-9][0-9]); do
	file=$(echo $f | sed 's/=/\\=/g')
	echo Imagenes/${file#$backup/}=$file >> $file_list
done
cp $file_list $TMP_FILE_LIST
for line in $(cat $TMP_SIZE_LIST | sort); do
	f=$(echo $line | cut -d " " -f 1)
	for s in $(find ${f%img} -name img.[0-9][0-9] | sort); do
		file=$(echo $s | sed 's/=/\\=/g')
		echo Imagenes/${file#$backup}=$file >> $file_list
		if [ $(mkisofs -r -V "Recuperación del Sistema $medio" -b boot/isolinux/isolinux.bin -c boot/isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -hide-rr-moved -iso-level 2 -graft-points -path-list $file_list -print-size 2>/dev/null) -gt $block_count ]; then
			COUNT=$((COUNT+1))
			sed -i "$(wc -l $file_list | cut -d " " -f 1)d" $file_list
			file_list=$TMP_FILE_LIST$COUNT
			cp $TMP_FILE_LIST $file_list
			echo Imagenes/${file#$backup/}=$file >> $file_list
		fi
	done	
done

zenity --question --title $TITLE --text "Se necesitan $COUNT $MEDIO para guardar las copias de seguridad del sistema existentes. ¿Desea continuar?" || abort $? 

IFS="$TAB"
if [ -f /proc/sys/dev/cdrom/info ]; then
	for i in $(grep 'drive name' /proc/sys/dev/cdrom/info | cut -f 2-); do 
		device_list="$(echo /dev/$i) $(humandev $i)
$device_list"
	done
fi
device_list="$device_list""Archivo de la imagen"

IFS="$NL"
zenity --list --width 400 --title $TITLE --text "Seleccione un Grabador:" --column "Grabadores Detectados" $device_list > $TMP_RESPONSE || abort $?
grabador=$(cat $TMP_RESPONSE | cut -d" " -f 1)

medio=1
IFS="$NBSP"
file="$HOME/backharddi-ng_$medio.iso"
while [ $medio -le $COUNT ]; do
	rm $ERROR
	if [ $grabador = "Archivo" ]; then
		zenity --entry --title $TITLE --text "Seleccione el nombre de la imagen para el $MEDIO número $medio:" --entry-text "$file" > $TMP_FILENAME || abort $?
		{ mkisofs -r -iso-level 2 -V "Recuperación del Sistema $medio" -b boot/isolinux/isolinux.bin -c boot/isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -hide-rr-moved -graft-points -path-list $TMP_FILE_LIST$medio -o $(cat $TMP_FILENAME) 2>&1 || touch $ERROR; } | sed -u 's/^[\ \t]*//' | tee /dev/stderr | zenity --progress --width 400 --title $TITLE --text "Generando CD/DVD de recuperación número $medio..." --auto-close
		[ -f $ERROR ] && { error; continue; }
		if [ "$SUDO_UID" != "" ]; then
			chmod ${SUDO_UID}:${SUDO_GID} $(cat $TMP_FILENAME) || true
		fi
		file="$(cat $TMP_FILENAME | sed "s/_${medio}\./_$((medio+1))\./")"
	else
		device=$(echo $grabador | cut -d" " -f 1) 
		zenity --question --title $TITLE --text "Introduzca un $MEDIO virgen para grabar el $MEDIO número $medio." || abort $?
		if [ $MEDIO = "CD" ]; then
			{ mkisofs -r -iso-level 2 -V "Recuperación del Sistema $medio" -b boot/isolinux/isolinux.bin -c boot/isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -hide-rr-moved -graft-points -path-list $TMP_FILE_LIST$medio | cdrecord driveropts=burnfree gracetime=2 dev=$device - 2>&1 || touch $ERROR; } | sed -u 's/^[\ \t]*//' | tee /dev/stderr | zenity --progress --width 400 --title $TITLE --text "Generando CD/DVD de recuperación número $medio..." --auto-close
		else
			{ growisofs -dvd-compat -Z $device -r -iso-level 2 -V "Recuperación del Sistema $medio" -b boot/isolinux/isolinux.bin -c boot/isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -hide-rr-moved -graft-points -path-list $TMP_FILE_LIST$medio 2>&1 || touch $ERROR; } | sed -u 's/^[\ \t]*//' | tee /dev/stderr | zenity --progress --width 400 --title $TITLE --text "Generando CD/DVD de recuperación número $medio..." --auto-close
		fi
		[ -f $ERROR ] && { error; continue; }
		eject $device
	fi
	medio=$((medio+1))
done

zenity --info --title $TITLE --text "Los $MEDIO de recuperación del sistema se han generado correctamente."

abort 0
