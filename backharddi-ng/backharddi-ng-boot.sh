#!/bin/bash

LOG=/var/log/backharddi-ng-boot
rm $LOG
exec 2> >(while read a; do echo "$(date):" "$a"; done >>$LOG) 
exec > >(while read a; do echo "$(date):" "$a"; done >>$LOG) 
set -xv

TITLE="Backharddi NG - Dispositivo de Arranque" 
KERNEL=/boot/linux-#KVERSION#-backharddi-ng
INITRD=/boot/minirt-#KVERSION#-backharddi-ng.gz
MEMTEST=/boot/memtest86+.bin
TMP_DIR=/tmp/backharddi-ng_boot
TMP_RESPONSE=/tmp/backharddi-ng_response
TMP_FILENAME=/tmp/backharddi-ng_filename
MARCA="backharddi"
NBSP=' '
TAB='	'
NL='
'
DOS_VOLUME_ID=bacadd10
SYSLINUX_DIR=/usr/share/backharddi-ng/syslinux
GRUB_DIR=/usr/share/backharddi-ng/grub
COMMON_DIR=/usr/share/backharddi-ng/common
FIFO=/tmp/progress_fifo
ERROR=/tmp/backharddi-ng_error
FILELIST=/usr/share/backharddi-ng/backharddi-ng-boot.filelist
GRUB_LOG=$LOG.grub

abort(){
	exec 3>&-
	rm $ERROR $TMP_FILENAME $TMP_RESPONSE $FIFO
	exit $1
}

error(){
	echo 100 >&3
	zenity --error --title $TITLE --text "$1"
	umount $TMP_DIR && rmdir $TMP_DIR
	abort 1
}

umount_device(){
	for m in $(mount | grep $1 | cut -d" " -f 1); do
		umount $m
	done
}

mksyslinux_boot(){
	monitor
	umount_device $1
	if [ x"$2" = xformat ]; then
		echo 10 >&3
		mkfs.msdos -I -i $DOS_VOLUME_ID -n "$MARCA" $1
		echo 20 >&3
		sfdisk -R $1
	fi
	syslinux $1 || error "No se ha podido instalar SYSLINUX en $1.\nEl sistema de archivos en $1 debe ser FAT16 o FAT32"
	echo 30 >&3
	mkdir -p $TMP_DIR
	mount $1 $TMP_DIR
	echo 40 >&3
	cp $KERNEL $TMP_DIR/linux
	echo 50 >&3
	cp $INITRD $TMP_DIR/initrd.gz
	cp $MEMTEST $TMP_DIR/mt86plus
	echo 60 >&3
	cp $SYSLINUX_DIR/* $TMP_DIR
	echo 70 >&3
	cp $COMMON_DIR/* $TMP_DIR
	echo 80 >&3
	umount $TMP_DIR && rm -r $TMP_DIR
	echo 100 >&3
}

mkgrub_boot(){
	monitor 
	umount_device $1
	if [ -z "$2" ]; then 
		echo 10 >&3
		dd if=/dev/zero of=$1 count=126
		echo 20 >&3
		sfdisk $1 <<EOT
unit: sectors
: start=63, size= , Id=83, bootable
EOT
		echo 30 >&3
		umount_device $1
		echo 40 >&3
		mkfs.ext3 -I 128 "$1"1 -L "$MARCA"
		echo 50 >&3
		part="$1"1
	else
		part="$2"
	fi
	mkdir -p $TMP_DIR
	mount $part $TMP_DIR || error "No se ha podido montar la partición $2.\nCompruebe que en la partición seleccionada hay un sistema de archivos."
	mkdir -p $TMP_DIR/boot/grub
	echo 60 >&3
	cp $KERNEL $TMP_DIR/boot
	cp $INITRD $TMP_DIR/boot
	if [ -f $TMP_DIR/boot/grub/menu.lst ]; then
		sed -i "/###INICIO\ SISTEMA\ DE\ BACKUP/,/###FIN\ SISTEMA\ DE\ BACKUP/d" $TMP_DIR/boot/grub/menu.lst
		cat <<EOF >> $TMP_DIR/boot/grub/menu.lst
###INICIO SISTEMA DE BACKUP. NO BORRAR ESTA MARCA.

title           Backharddi NG HD
root            (hd0,#PARTNUM#)
kernel          /boot/linux-#KVERSION#-backharddi-ng backharddi/medio=hd-media video=vesa:ywrap,mtrr vga=788 locale=es_ES console-keymaps-at/keymap=es quiet --
initrd          /boot/minirt-#KVERSION#-backharddi-ng.gz
boot

title           Backharddi NG NET
root            (hd0,#PARTNUM#)
kernel          /boot/linux-#KVERSION#-backharddi-ng backharddi/medio=net video=vesa:ywrap,mtrr vga=788 netcfg/choose_interface=auto netcfg/get_hostname=bkd netcfg/get_domain= locale=es_ES console-keymaps-at/keymap=es quiet --
initrd          /boot/minirt-#KVERSION#-backharddi-ng.gz
boot

###FIN SISTEMA DE BACKUP. NO BORRAR ESTA MARCA.
EOF
	else
		cp $GRUB_DIR/* $TMP_DIR/boot/grub
	fi
	echo 70 >&3
	cat >$TMP_DIR/boot/grub/device.map <<EOT
(hd0) $1
EOT
	echo 80 >&3
	partnum=$((${part#$1}-1))
	sed -i "s/#PARTNUM#/$partnum/" $TMP_DIR/boot/grub/menu.lst
	grub --batch --device-map=$TMP_DIR/boot/grub/device.map > $GRUB_LOG <<EOF
root (hd0,$partnum)
setup (hd0)
quit
EOF
	if grep "Error [0-9]*: " $GRUB_LOG >/dev/null; then
		error "GRUB no se ha instalado correctamente en el dispositivo seleccionado.\nVea el fichero $GRUB_LOG para obtener información del error." 
	fi
	echo 90 >&3
	umount $TMP_DIR && rm -r $TMP_DIR
	echo 100 >&3
}

monitor(){
	mkfifo $FIFO
	zenity --progress --width 400 --title $TITLE --text "Preparando dispositivo de almacenamiento USB..." --auto-close <$FIFO || abort $? &
	exec 3>&1 3>$FIFO
}
	 
humandev(){
	vendor="$(udevadm info -q env -n $1 | grep ID_VENDOR= | cut -d = -f 2)"
	model="$(udevadm info -q env -n $1 | grep ID_MODEL= | cut -d = -f 2)"
	[ -z $vendor ] && vendor="$(cat /sys/block/$(basename $1)/device/vendor)"
	[ -z $model ] && model="$(cat /sys/block/$(basename $1)/device/model)"
	echo "$vendor $model"
}

humanpart(){
	udi="$(hal-find-by-property --key block.device --string $1 | head -n 1)"
	[ -z "$udi" ] && return 0
	label="$(hal-get-property --udi "$udi" --key volume.label)"
	if [ -n "$label" ]; then
		echo "Etiquetada como: $label"
	fi
}

alert(){
	zenity --question --width 400 --title $TITLE --text "Recuerde que si continúa perderá cualquier dato que hubiera en el dispositivo de almacenamiento USB seleccionado. ¿Desea continuar?" || abort $?
}		

cd /tmp
IFS="$NBSP"
zenity --list --width 400 --title $TITLE --text "Seleccione el tipo de medio para arrancar el sistema Backharddi NG:" --column "Medios soportados" "Dispositivo de almacenamiento USB" "CD-ROM" > $TMP_RESPONSE || abort $?
medio=$(cat $TMP_RESPONSE)

case "$medio" in
	*USB*)
		IFS="$NL"
		for i in $(find /dev -maxdepth 1 -name sd?); do
			bus="$(udevadm info -q env -n $i | grep ID_BUS= | cut -d = -f 2)"
			case "$bus" in
				*[Uu][Ss][Bb]*) device_list="$(echo $i) $(humandev $i)
$device_list";;
				*[Ss][Cc][Ss][Ii]* | *[Ii][Dd][Ee]*) true ;;
				*) device_list="$(echo $i) $(humandev $i)
$device_list";;
			esac
		done
		
		if [ -z "$device_list" ]; then
			zenity --info --title $TITLE --text "No se ha encontrado ningún dispositivo de almacenamineto USB en su sistema. Por favor, inserte uno."
			exit 1
		fi
		
		IFS="$NL"
		zenity --list --width 400 --title $TITLE --text "Seleccione un dispositivo de almacenamiento USB:" --column "Dispositivos USB Detectados" $device_list > $TMP_RESPONSE || abort $?
		device=$(cat $TMP_RESPONSE | cut -d" " -f 1)
		
		zenity --list --width 400 --title $TITLE --text "Seleccione el gestor de arranque para el dispositivo seleccionado:" --column "Gestores de arranque disponibles" "GRUB" "SYSLINUX" > $TMP_RESPONSE || abort $?
		boot=$(cat $TMP_RESPONSE)
		zenity --list --title $TITLE --text "Desea formatear todo el dispositivo seleccionado?" --column "" "Sí" "No" > $TMP_RESPONSE || abort $?
		format=$(cat $TMP_RESPONSE)

		case "$boot" in
			GRUB) 
				if [ "$format" = "Sí" ]; then
					alert
					mkgrub_boot $device
				else
					for i in $device*; do
						[ "$device" = "$i" ] && continue
						part_list="$(echo $i) $(humanpart $i)
$part_list"
					done

					if [ -z "$part_list" ]; then
						zenity --info --title $TITLE --text "No se ha encontrado ninguna partición en el dispositivo seleccionado. Por favor, formatéelo antes."
						exit 1
					fi
					IFS="$NL"
					zenity --list --width 400 --title $TITLE --text "Seleccione la partición donde desea instalar el sistema de arranque.\nRecuerde que los datos de esta partición no se perderán y que debe tener al menos 16MB libres en esta partición." --column "Particiones detectadas" $part_list > $TMP_RESPONSE || abort $?
					part=$(cat $TMP_RESPONSE | cut -d" " -f 1)
					mkgrub_boot $device $part
				fi
			;;
			SYSLINUX) 
				if [ "$format" = "Sí" ]; then
					alert
					mksyslinux_boot $device format
				else
					mksyslinux_boot $device
				fi
			;;
		esac
	;;	
	*CD-ROM*)
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

		IFS="$NBSP"
		while true; do
			rm $ERROR
			if [ $grabador = "Archivo" ]; then
				zenity --entry --title $TITLE --text "Seleccione el nombre de la imagen:" --entry-text "$HOME/backharddi-ng.iso" > $TMP_FILENAME || abort $?
				{ mkisofs -r -iso-level 2 -V "Backharddi NG" -b boot/isolinux/isolinux.bin -c boot/isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -hide-rr-moved -graft-points -path-list $FILELIST -o $(cat $TMP_FILENAME) 2>&1 || touch $ERROR; } | sed -u 's/^[\ \t]*//' | tee /dev/stderr | zenity --progress --width 400 --title $TITLE --text "Generando CD de arranque Backharddi NG..." --auto-close
				[ -f $ERROR ] && { error; continue; }
			else
				device=$(echo $grabador | cut -d" " -f 1) 
				zenity --question --title $TITLE --text "Introduzca un CD virgen." || abort $?
				{ mkisofs -r -iso-level 2 -V "Backharddi NG" -b boot/isolinux/isolinux.bin -c boot/isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -hide-rr-moved -graft-points -path-list $FILELIST | cdrecord driveropts=burnfree gracetime=2 dev=$device - 2>&1 || touch $ERROR; } | sed -u 's/^[\ \t]*//' | tee /dev/stderr | zenity --progress --width 400 --title $TITLE --text "Generando CD de arranque Backharddi NG..." --auto-close
				[ -f $ERROR ] && { error; continue; }
				eject $device
			fi
			break
		done
	;;
esac

zenity --info --title $TITLE --text "La preparación del medio de arranque ha terminado correctamente."

abort 0
