#!/bin/bash

LOG=/var/log/backharddi-ng-net
rm $LOG
exec > >(while read a; do echo "$(date):" "$a"; done >>$LOG)
exec 2> >(while read a; do echo "$(date):" "$a"; done >>$LOG)
set -xv
shopt -s extglob

TITLE="Backharddi NG - Net"
TMP_DIR=/tmp/backharddi-ng
TMP_RESPONSE=/tmp/backharddi-ng_response
MARCA=backharddi
NBSP=' '
TAB='	'
NL='
'
ERROR=/tmp/backharddi-ng_error
DHCPD_CONF=/etc/dhcp3/backharddi-ng_dhcpd
DHCPD_DEFAULT=/etc/default/dhcp3-server
DHCPD_SERVICE=/etc/init.d/dhcp3-server
TFTPD_DEFAULT=/etc/default/atftpd
TFTPD_SERVICE=/etc/init.d/atftpd
PORTMAP_SERVICE=/etc/init.d/portmap
NFSCOMMON_SERVICE=/etc/init.d/nfs-common
NFSD_SERVICE=/etc/init.d/nfs-kernel-server
EXPORTS=/etc/exports
FIFO=/tmp/progress_fifo

lista_backups(){
	for backup in $(ls $TMP_DIR); do
        	[ -d "$TMP_DIR/$backup" ] || continue
	        [ "$backup" = "boot" ] && continue
        	[ "$backup" = "lost+found" ] && continue
		[ -z "$(find $TMP_DIR/$backup -name img)" ] && continue
        	backup_nombre=$(echo $backup | tr "=" "/" | tr "_" " ")
		backups="$backup_nombre
$backups"
	done
}

abort(){
	if [ ! "x$AUTODHCP" = "x" ]; then
		$DHCPD_SERVICE stop
	fi
	if [ ! "x$AUTOTFTP" = "x" ]; then
		$TFTPD_SERVICE stop
	fi
	if [ ! "x$AUTONFS" = "x" ]; then
		$NFSD_SERVICE stop
		$NFSCOMMON_SERVICE stop
		$PORTMAP_SERVICE stop
	fi
	sed -i "/###INICIO\ BACKHARDDI-NG-NET/,/###FIN\ BACKHARDDI-NG-NET/d" $DHCPD_DEFAULT $TFTPD_DEFAULT $EXPORTS
	if [ ! "x$AUTOMULTICAST" = "x" ]; then
		/etc/init.d/backharddi-ng-net stop
	fi
	umount $TMP_DIR && rm -rf $TMP_DIR
	rm $TMP_RESPONSE $ERROR $DHCPD_CONF
	exit $1
}

error(){
	zenity --error --title $TITLE --text "Ha ocurrido un error."
}

progress(){
	mkfifo $FIFO
	zenity --progress --width 400 --title $TITLE --text "$1" --auto-close <$FIFO || abort $? &
	exec 3>&1 3>$FIFO
}

humandev(){
	vendor="$(udevinfo -q env -n $1 | grep ID_VENDOR | cut -d = -f 2)"
	model="$(udevinfo -q env -n $1 | grep ID_MODEL | cut -d = -f 2)"
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

cd /tmp
backuppart=$(findfs LABEL=$MARCA)
if [ -z $backuppart ]; then
	zenity --info --title $TITLE --text "No se ha encontrado ninguna partición de backup. Por favor, establezca una."
	IFS="$NL"
	for i in $(find /dev -maxdepth 1 -name sd? -o -name hd?); do
		device_list="$(echo $i) $(humandev $i)
$device_list"
	done
		
	if [ -z "$device_list" ]; then
		zenity --info --title $TITLE --text "No se ha encontrado ningún dispositivo de almacenamineto en su sistema. Por favor, inserte uno."
		exit 1
	fi
		
	IFS="$NL"
	zenity --list --width 400 --title $TITLE --text "Seleccione un dispositivo de almacenamiento:" --column "Dispositivos Detectados" $device_list > $TMP_RESPONSE || abort $?
	device=$(cat $TMP_RESPONSE | cut -d" " -f 1)
	for i in $device*; do
		[ "$device" = "$i" ] && continue
		part_list="$(echo $i) $(humanpart $i)
$part_list"
	done

	if [ -z "$part_list" ]; then
		zenity --question --title $TITLE --text "No se ha encontrado ninguna partición en el dispositivo seleccionado. Desea crear una?. Recuerde que cualquier dato que hubiera en el dispositivo se perderá." || abort $?
		dd if=/dev/zero of=$device count=126
		sfdisk $device <<EOT
unit: sectors
: start=63, size= , Id=83
EOT
		backuppart=${device}1
	else
		IFS="$NL"
		zenity --list --width 400 --title $TITLE --text "Seleccione la partición que desea establecer como partición de backup.\nRecuerde que se formateará la partición seleccionada y que por lo tanto, los datos de esta partición se perderán." --column "Particiones detectadas" $part_list > $TMP_RESPONSE || abort $?
		backuppart=$(cat $TMP_RESPONSE | cut -d" " -f 1)
	fi
	umount $backuppart || true
	progress "Formateando partición $backuppart como partición de backup..."
	echo 10 >&3
	mkfs.ext3 $backuppart -L "backharddi"
	echo 90 >&3
	sleep 1
	echo 100 >&3
fi

[ -d $TMP_DIR ] || mkdir $TMP_DIR
mount $backuppart $TMP_DIR 2>/dev/null

zenity --height=250 --title "$TITLE" --text "Seleccione que servicios desea que Backharddi NG Net autoconfigure y arranque:" --list --checklist --column "Arrancar Servicio" --column "Servicio" FALSE "DHCP" FALSE "TFTP" FALSE "NFS" TRUE "Backharddi NG MULTICAST" > $TMP_RESPONSE || abort 0

IFS='|'
for service in $(cat $TMP_RESPONSE); do
	case "$service" in
		*DHCP* ) AUTODHCP=true ;;
		*TFTP* ) AUTOTFTP=true ;;
		*NFS* ) AUTONFS=true ;;
		*MULTICAST* ) AUTOMULTICAST=true ;;
	esac
done

IFS="$NL"
progress "Arrancando servicios solicitados..." 
if [ ! "x$AUTODHCP" = "x" ]; then
	cat <<EOF >$DHCPD_CONF
allow booting;
allow bootp;
default-lease-time 600;
max-lease-time 1800;
EOF

	IFS="$NBSP"
	for dev in /sys/class/net/*; do
		[ -d "$dev" ] || continue
		[ -e "$dev/device" ] || continue
		[ ! -d "$dev/wireless" ] || continue
		eth=$(basename $dev)
		address="$(backharddi-ng-net-ipsc -i $eth -a | grep ^IP\ address: | cut -d ":" -f2 | tr -d " ")"
		[ -z "$address" ] && continue
		interfaces="$eth $interfaces"
		network="$(backharddi-ng-net-ipsc -i $eth -a | grep ^Network\ address: | cut -d ":" -f2 | tr -d " ")"
		netmask="$(backharddi-ng-net-ipsc -i $eth -a | grep ^Network\ mask: | cut -d ":" -f2 | tr -d " ")"
		range="$(backharddi-ng-net-ipsc -i $eth -a | grep ^Host\ allocation\ range: | cut -d ":" -f2 | tr -d " " | tr "-" " ")"
		cat <<EOF >>$DHCPD_CONF

subnet $network netmask $netmask {
  next-server $address;
  filename "pxelinux.0";
  option domain-name-servers $address;
  option routers $address;
  option subnet-mask $netmask;
  range $range; 
}
EOF
	done
	IFS="$NL"

	[ -z $interfaces ] && { echo 100 >&3; zenity --info --title $TITLE --text "No se ha encontrado ninguna interfaz de red ethernet configurada. Por favor, configure alguna."; abort 0; }

	sed "s/#INTERFACES#/$interfaces/" <<EOF >>$DHCPD_DEFAULT
###INICIO BACKHARDDI-NG-NET. NO BORRAR ESTA MARCA.

INTERFACES="#INTERFACES#"
CONFIG_FILE=$DHCPD_CONF

###FIN BACKHARDDI-NG-NET. NO BORRAR ESTA MARCA.
EOF

	$DHCPD_SERVICE stop
	if netstat -lu | grep -q bootps; then
		echo 100 >&3
		zenity --info --title $TITLE --text "Se ha encontrado un servicio DHCP activado en el sistema. Por favor pare el servicio o no marque su casilla. Si no marca su casilla el servicio deberá configurarse manualmente."
		abort $?
	fi
	$DHCPD_SERVICE start
fi
echo 25 >&3

if [ ! "x$AUTOTFTP" = "x" ]; then
	[ -d $TMP_DIR/tftpboot ] || mkdir -p $TMP_DIR/tftpboot
	[ -d $TMP_DIR/pxelinux.cfg ] || mv $TMP_DIR/pxelinux.cfg $TMP_DIR/tftpboot
	[ -d $TMP_DIR/tftpboot/pxelinux.cfg ] || mkdir -p $TMP_DIR/tftpboot/pxelinux.cfg
	if [ ! -f $TMP_DIR/tftpboot/pxelinux.cfg/default ]; then
		cat <<EOF > $TMP_DIR/tftpboot/pxelinux.cfg/default
DEFAULT menu.c32
TIMEOUT 100
MENU TITLE BACKHARDDI-NG

LABEL BACKHARDDI-NG-NET
KERNEL linux
APPEND video=vesa:ywrap,mtrr vga=788 locale=es_ES console-keymaps-at/keymap=es backharddi/medio=net netcfg/choose_interface=auto netcfg/get_hostname=edm netcfg/get_domain= initrd=initrd.gz --

LABEL LOCAL
LOCALBOOT 0
EOF
	fi

	if which atftpd; then
		cat <<EOF >>$TFTPD_DEFAULT
###INICIO BACKHARDDI-NG-NET. NO BORRAR ESTA MARCA.

USE_INETD=false
OPTIONS="--daemon --port 69 --tftpd-timeout 300 --retry-timeout 5 --mcast-port 1758 --mcast-addr 239.239.239.0-255 --mcast-ttl 1 --maxthread 100 --verbose=5  /usr/share/backharddi-ng/tftpboot"

###FIN BACKHARDDI-NG-NET. NO BORRAR ESTA MARCA.
EOF
	else
		TFTPD_DEFAULT=/etc/default/tftpd-hpa
		TFTPD_SERVICE=/etc/init.d/tftpd-hpa
		cp /usr/share/backharddi-ng/tftpboot/!(pxelinux.cfg) $TMP_DIR/tftpboot/ --remove-destination
                cat <<EOF >>$TFTPD_DEFAULT
###INICIO BACKHARDDI-NG-NET. NO BORRAR ESTA MARCA.

RUN_DAEMON="yes"
OPTIONS="-l -s $TMP_DIR/tftpboot"

###FIN BACKHARDDI-NG-NET. NO BORRAR ESTA MARCA.
EOF
	fi

	$TFTPD_SERVICE stop
	if netstat -lu | grep -q tftp; then
		echo 100 >&3
		zenity --info --title $TITLE --text "Se ha encontrado un servicio TFTP activado en el sistema. Por favor pare el servicio o no marque su casilla. Si no marca su casilla el servicio deberá configurarse manualmente."
		abort $?
	fi
	$TFTPD_SERVICE start
fi
echo 50 >&3

if [ ! "x$AUTONFS" = "x" ]; then
	$NFSD_SERVICE stop
	$NFSCOMMON_SERVICE stop
	$PORTMAP_SERVICE stop

	cat <<EOF >>$EXPORTS
###INICIO BACKHARDDI-NG-NET. NO BORRAR ESTA MARCA.

$TMP_DIR *(rw,no_root_squash,sync)

###FIN BACKHARDDI-NG-NET. NO BORRAR ESTA MARCA.
EOF

	$PORTMAP_SERVICE start
	$NFSCOMMON_SERVICE start
	$NFSD_SERVICE start
fi
echo 75 >&3

if [ ! "x$AUTOMULTICAST" = "x" ]; then
	/etc/init.d/backharddi-ng-net restart
fi
echo 100 >&3

cd /usr/share/backharddi-ng
backharddi-ng-net-monitor
abort 0
