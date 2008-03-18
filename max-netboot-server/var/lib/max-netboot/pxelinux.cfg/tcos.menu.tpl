DEFAULT menu.c32
PROMPT 0
TIMEOUT 200
ONTIMEOUT TCOS

MENU TITLE MaX Menu de arranque por red...

LABEL TCOS
       MENU LABEL Arranque en red TCOS (por defecto)
       KERNEL /tcos/vmlinuz-__TCOS_KERNEL__
       APPEND ramdisk_size=65536 initrd=/tcos/initramfs-__TCOS_KERNEL__ root=/dev/ram0 boot=tcos quiet splash

LABEL TCOSNFS
       MENU LABEL Arranque en red TCOS (equipos con menos de 40 MB)
       KERNEL /tcos/vmlinuz-__TCOS_KERNEL__
       APPEND ramdisk_size=32768 initrd=/tcos/initramfs-__TCOS_KERNEL__-nfs root=/dev/ram0 boot=tcos quiet

# Return to the main menu
LABEL MAIN
        MENU LABEL <== Volver al menu principal
        KERNEL menu.c32
        APPEND ~

