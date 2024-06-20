#!/bin/bash

# Obtener el nombre del usuario que ha iniciado sesión
#USER="$USER"

# Indicar un usuario concreto
#USER="nombre_apellido"

# Función para encontrar el usuario cuyo ID esté entre 1002 y 1005
encontrar_usuario() {
    for ID in {1002..1005}; do
        USER=$(getent passwd "$ID" | cut -d: -f1)
        if [ -n "$USER" ]; then
            echo "$USER"
            return 0
        fi
    done
    echo "No se encontró ningún usuario con ID entre 1002 y 1005."
    exit 1
}

# Indicar el usuario cuyo ID se encuentra entre 1002 y 1005
USER=$(encontrar_usuario)

# Definir la ruta del directorio admin_gestion usando el home del usuario indicado
DIR="/home/$USER/admin_gestion"

# Comprobar si el directorio admin_gestion existe o crearlo 
if [ -d "$DIR" ]; then
    echo "El directorio $DIR ya existe."
else
    # Crear el directorio
    sudo mkdir -p "$DIR"
    echo "El directorio $DIR ha sido creado."
    sudo chown "$USER":"$USER" "$DIR"
    echo "Modificado el propietario del directorio $DIR a $USER"
fi

# Comprobar si el fichero maxvpnconfig existe y no está vacío
if [ -s "$DIR/maxvpnconfig" ]; then
    echo "El fichero $DIR/maxvpnconfig ya existe."
else
    # Crear el fichero maxvpnconfig si no existe o está vacío
    sudo touch "$DIR/maxvpnconfig"

    # Añadir el contenido al fichero maxvpnconfig
    sudo bash -c "cat << EOF > \"$DIR/maxvpnconfig\"
### configuration file for openfortivpn, see man openfortivpn(1) ###

host = 193.146.123.126
port = 8443
username = vpnuser
password = VPNpassw0rd
# trusted-cert = 
EOF"

    echo "El fichero $DIR/maxvpnconfig se ha creado correctamente."

    sudo chown "$USER":"$USER" "$DIR/maxvpnconfig"
    echo "Modificado el propietario del fichero $DIR/maxvpnconfig a $USER"
fi

# Ejecución de la VPN desde el directorio /home/usuario/admin_gestion
#sudo openfortivpn -c maxvpnconfig 