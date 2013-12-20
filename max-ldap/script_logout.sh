#!/bin/bash

# Para activar el script al cierre de la sesión en lightdm, añadir al fichero /etc/lightdm/lightdm.conf
# la siguiente línea: session-cleanup-script=/etc/lightdm/script_logout.sh

# Descomentar para crear log
#log=/tmp/cierresesion.txt

if [ $log != "" ] ; then
   echo "Usuario: $USER" > $log
fi

# Antes de hacer el killall se espera que se cierren de forma normal los procesos
# que usan el punto de montaje del usuario, normalmente sólo hace una interación del for.
for x in 1 2 3 4 5 ; do
        if [ $log != "" ] ; then
           /bin/fuser -m "/home/samba/users/${USER}" >> $log 2>&1
           echo "$x ===========" >> $log
        fi
        /bin/fuser -m "/home/samba/users/${USER}" || break
        sleep 1
done

if [ $log != "" ] ; then
   ps -u $USER >> $log 2>&1
fi

# Ahora sí, se fuerza el cierre de todos los procesos del usuario.
killall -u ${USER}
# Como no se haga una pequeña pausa umount sigue fallando
sleep 1

if [ $log != "" ] ; then
   umount -v /home/samba/users/$USER >> $log 2>&1
else
   umount /home/samba/users/$USER
fi
