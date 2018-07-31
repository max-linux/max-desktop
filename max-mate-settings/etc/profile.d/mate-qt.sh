# MATE Desktop Qt integrations
if [ "x$DESKTOP_SESSION" == "xmate" ] || [ "x$XDG_SESSION_DESKTOP" == "xmate" ]; then
    # QT apps to use GTK styling
    export QT_QPA_PLATFORMTHEME=gtk2
fi

