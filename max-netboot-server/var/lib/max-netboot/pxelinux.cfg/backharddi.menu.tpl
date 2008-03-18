DEFAULT menu.c32
PROMPT 0
TIMEOUT 200
ONTIMEOUT BACKHARDDI

MENU TITLE MaX Menu de arranque por red...

LABEL BACKHARDDI
       MENU LABEL Arranque de BackHarddi por red
       KERNEL /backharddi/__BK_KERNEL__
       APPEND backharddi/medio=net video=vesa:ywrap,mttr vga=788 netcfg/choose_interface=auto netcfg/get_hostname=bkd netcfg/get_domain= locale=es_ES console-keymaps-at/keymap=es initrd=/backharddi/__BK_INITRD__ --

# Return to the main menu
LABEL MAIN
        MENU LABEL <== Volver al menu principal
        KERNEL menu.c32
        APPEND ~

