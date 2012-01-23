#!/bin/bash

#
# This script is part of nanomax-installer
#
#

DEVICE=$1
PORTABLES=$2
PERSISTENTE=$3
PORTABLES_SIZE=$4
PERSISTENTE_SIZE=$5
PORTABLES_TYPE=fat16

# tamaño para nanomax
NANOMAX=750

# tamaño para aplicaciones portables
PORTSIZE=800

get_human_size() {
  if [ $1 -gt 1000 ]; then
    echo $(($1/1000)) GiB
  else
    echo "$1 MiB"
  fi
}

if [ "$DEVICE" = "" ] || [ "$PORTABLES" = "" ] || [ "$PERSISTENTE" = "" ] ; then
  zenity --error --text="Error en los parámetros:\nDispositivo: '$DEVICE'\nPortables: '$PORTABLES'\nPersistente: '$PERSISTENTE'"
  exit 1
fi

if [ ! -d /cdrom/nanomax ]; then
  zenity --error --text="No se esta ejecutando desde el DVD de MaX... saliendo"
  exit 1
fi

# leemos el tamaño
#SIZE=$(LC_ALL=C /sbin/fdisk -l $DEVICE| awk '/^Disk*.*bytes/ {print $3}')
# usar en bytes y dividir entre 10^6 para tener megas
SIZE=$(LC_ALL=C /sbin/fdisk -l $DEVICE | awk '/^Disk*.*bytes/ {print int($5/1000000)}')

echo " * Tamaño del dispositivo $DEVICE detectado $(get_human_size $SIZE)"

if [ "$PORTABLES" = "1" ] && [ "$PERSISTENTE" = "1" ]; then
  P1=$PORTABLES_SIZE        # portables
  P2=$PERSISTENTE_SIZE      # casper-rw
  P3=$NANOMAX               # nanomax
  TIPO=1
  if [ $PORTABLES_SIZE -gt 1900 ]; then
    PORTABLES_TYPE=fat32
  fi
  echo " * Instalación tipo 1: nanoMaX ($(get_human_size $P3)) + portables ($(get_human_size $P1)) ($PORTABLES_TYPE) + persistencia ($(get_human_size $P2))"
elif [ "$PORTABLES" = "0" ] && [ "$PERSISTENTE" = "1" ]; then
  P1=$NANOMAX              # nanomax
  P2=$(($SIZE-$NANOMAX))   # persistente
  P3=0
  TIPO=2
  echo " * Instalación tipo 2: nanoMaX ($(get_human_size $P1)) + persistencia ($(get_human_size $P2))"
elif [ "$PORTABLES" = "0" ] && [ "$PERSISTENTE" = "0" ]; then
  P1=$(($SIZE))
  P2=0
  P3=0
  TIPO=3
  echo " * Instalación tipo 3: sólo nanoMaX ($(get_human_size $P1))"
elif [ "$PORTABLES" = "1" ] && [ "$PERSISTENTE" = "0" ]; then
  P1=$(($SIZE-$NANOMAX)) # portables
  P2=$NANOMAX            # nanomax
  P3=0
  TIPO=4
  if [ $P1 -gt 1900 ]; then
    PORTABLES_TYPE=fat32
  fi
  echo " * Instalación tipo 4: nanoMaX ($(get_human_size $P2)) + portables ($(get_human_size $P1)) ($PORTABLES_TYPE)"
fi

#echo " * Desmontando particiones..."
# desmontamos particiones
sync
for dev in $(grep "$DEVICE" /proc/mounts | awk '{print $1}'); do
  umount -l $dev
done

zenity --question --text="Si continúa se formateará el contenido de $DEVICE y se perderán todos los archivos y particiones \n¿Desea seguir?"
if [ $? != 0 ]; then
  echo " * Cancelado !!!!!"
  exit 1
fi

# rebajamos 2 megas el final
SIZE=$(($SIZE-2))

copiar_nanomax() {
  echo " * Copiando archivos de nanoMaX... (tarda un rato)"
  rsync -Pazv /cdrom/nanomax/ /mnt/nanomax/ | zenity --progress --auto-close --pulsate --text="Copiando nanoMax..."
  mv /mnt/nanomax/isolinux /mnt/nanomax/syslinux
  cp /mnt/nanomax/syslinux/txt-usb.cfg  /mnt/nanomax/syslinux/txt.cfg
}

copiar_portables() {
  echo " * Extrayendo aplicaciones portables... (tarda un rato)"
  tar -vzxf /cdrom/portables/portables.tar.gz -C /mnt/portables 2>/dev/null | zenity --progress --auto-close --pulsate --text="Extrayendo aplicaciones portables..."
}


# borrar MBR
dd if=/dev/zero of=$DEVICE bs=512 count=1 >/dev/null 2>&1

mkdir -p /mnt/nanomax /mnt/portables

if [ "$TIPO" = "1" ]; then
  echo " * Haciendo particiones..."
  # portables y casper
  parted $DEVICE -s mklabel msdos mkpart primary $PORTABLES_TYPE 0 ${P1}MB
  parted $DEVICE -s mkpart primary ext3 ${P1}MB $(($P1+$P2))MB 
  parted $DEVICE -s mkpart primary fat16 $(($P1+$P2))MB ${SIZE}MB
  parted $DEVICE -s set 3 boot on
  sleep 1
  echo " * Formateando..."
  # formatear
  if [ "$PORTABLES_TYPE" = "fat16" ]; then
      mkdosfs -n "datos" -F 16 ${DEVICE}1 >/dev/null 2>&1
  else
      mkdosfs -n "datos" -F 32 ${DEVICE}1 >/dev/null 2>&1
  fi
  mkfs.ext3 -L "casper-rw" ${DEVICE}2 >/dev/null 2>&1
  mkdosfs -n "nanomax" -F 16 ${DEVICE}3 >/dev/null 2>&1
  mount -t vfat -o noatime,rw ${DEVICE}3 /mnt/nanomax
  mount -t vfat -o noatime,rw ${DEVICE}1 /mnt/portables
  copiar_nanomax
  copiar_portables
  echo " * Sincronizando... (puede tardar un rato)"
  sync
  syslinux ${DEVICE}3
  umount /mnt/portables
  umount /mnt/nanomax
  install-mbr -e3 ${DEVICE}
fi

if [ "$TIPO" = "2" ]; then
  # casper
  echo " * Haciendo particiones..."
  parted $DEVICE -s mklabel msdos mkpart primary fat16 0 ${P1}MB
  parted $DEVICE -s mkpart primary ext3 ${P1}MB ${SIZE}MB
  parted $DEVICE -s set 1 boot on
  sleep 1
  echo " * Formateando..."
  # formatear
  mkfs.ext3 ${DEVICE}2 -L "casper-rw" >/dev/null 2>&1
  mkdosfs -n "nanomax" -F 16 ${DEVICE}1 >/dev/null 2>&1
  mount -t vfat -o noatime,rw ${DEVICE}1 /mnt/nanomax 
  copiar_nanomax
  echo " * Sincronizando... (puede tardar un rato)"
  sync
  syslinux ${DEVICE}1
  umount /mnt/nanomax
  install-mbr -e1 ${DEVICE}
fi

if [ "$TIPO" = "3" ]; then
  # solo nanomax
  echo " * Haciendo particiones..."
  parted $DEVICE -s mklabel msdos mkpart primary fat16 0 ${SIZE}MB
  parted $DEVICE -s set 1 boot on
  sleep 1
  echo " * Formateando..."
  # formatear
  mkdosfs -n "nanomax" -F 16 ${DEVICE}1 >/dev/null 2>&1
  sleep 1
  mount -t vfat -o noatime,rw ${DEVICE}1 /mnt/nanomax
  copiar_nanomax
  sed -i -e 's|persistent ||g' /mnt/nanomax/syslinux.cfg
  echo " * Sincronizando... (puede tardar un rato)"
  sync
  syslinux ${DEVICE}1
  umount /mnt/nanomax
  install-mbr -e1 ${DEVICE}
fi

if [ "$TIPO" = "4" ]; then
  # portables y nanomax
  echo " * Haciendo particiones..."
  parted $DEVICE -s mklabel msdos mkpart primary $PORTABLES_TYPE 0 ${P1}MB
  parted $DEVICE -s mkpart primary fat16 ${P1}MB ${SIZE}MB
  parted $DEVICE -s set 2 boot on
  sleep 1
  echo " * Formateando..."
  # formatear
  if [ "$PORTABLES_TYPE" = "fat16" ]; then
      mkdosfs -n "datos" -F 16 ${DEVICE}1 >/dev/null 2>&1
  else
      mkdosfs -n "datos" -F 32 ${DEVICE}1 >/dev/null 2>&1
  fi
  mkdosfs -n "nanomax" -F 16 ${DEVICE}2   >/dev/null 2>&1
  mount -t vfat -o noatime,rw ${DEVICE}2 /mnt/nanomax
  mount -t vfat -o noatime,rw ${DEVICE}1 /mnt/portables
  copiar_nanomax
  copiar_portables
  sed -i -e 's|persistent ||g' /mnt/nanomax/syslinux.cfg
  echo " * Sincronizando... (puede tardar un rato)"
  sync
  syslinux ${DEVICE}2
  umount /mnt/portables
  umount /mnt/nanomax
  install-mbr -e2 ${DEVICE}
fi

echo " * Borrando directorios temporales"
rm -rf /mnt/nanomax /mnt/portables

# mejor no mostrar este mensaje
#zenity --info --text="Ya esta listo su nanoMaX, inicie su equipo desde el dispositivo USB."

echo " * Terminado, puede retirar su nueva nanoMaX."

exit 0


