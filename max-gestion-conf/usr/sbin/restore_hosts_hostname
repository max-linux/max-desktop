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

# Comprobar si el usuario está en el grupo "editores"
if id -nG "$USER" | grep -qw "editores"; then
    # Remover el usuario del grupo "editores"
    sudo gpasswd -d "$USER" editores
    echo "El usuario $USER ha sido removido del grupo 'editores'."
else
    echo "El usuario $USER no está en el grupo 'editores'."
fi

# Comprobar si el grupo "editores" existe
if getent group editores > /dev/null 2>&1; then
    # Remover el grupo
    sudo groupdel editores
    echo "El grupo 'editores' ha sido eliminado."
else
    echo "El grupo 'editores' no existe."
fi

# Función para restaurar archivos desde la copia de seguridad
restaurar_ficheros() {
    FILE=$1
    BACKUP_FILE="$FILE.backup"
    if [ -f "$BACKUP_FILE" ]; then
        sudo mv "$BACKUP_FILE" "$FILE"
        echo "El archivo $FILE ha sido restaurado desde $BACKUP_FILE."
    else
        echo "El archivo de respaldo $BACKUP_FILE no existe."
    fi
}

eliminar_simbolicos() {
    if [ -d "$DIR" ]; then
        for FILE in "$DIR"/*; do
            BASENAME=$(basename "$FILE")
            if [ -L "$FILE" ] && { [ "$BASENAME" = "hosts" ] || [ "$BASENAME" = "hostname" ]; }; then
                sudo rm "$FILE"
                echo "Enlace simbólico eliminado: $FILE"
            fi
        done
    else
        echo "El directorio $DIR no existe."
    fi
}

# Llamar a la función para eliminar enlaces simbólicos específicos
eliminar_simbolicos

# Restaurar copia de seguridad de los archivos modificados
restaurar_ficheros /etc/hosts
restaurar_ficheros /etc/hostname

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