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

# Eliminar el fichero maxvpnconfig en admin_gestion si existe
if [ -f "$DIR/maxvpnconfig" ]; then
    sudo rm "$DIR/maxvpnconfig"
    echo "El fichero $DIR/maxvpnconfig ha sido eliminado."
else
    echo "El fichero $DIR/maxvpnconfig no existe."
fi

# Eliminar el directorio admin_gestion si está vacío
if [ -d "$DIR" ]; then
    if [ -z "$(ls -A "$DIR")" ]; then
        sudo rmdir "$DIR"
        echo "El directorio $DIR ha sido eliminado."
    else
        echo "El directorio $DIR no está vacío."
    fi
else
    echo "El directorio $DIR no existe."
fi