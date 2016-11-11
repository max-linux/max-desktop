# Ensure GTK accessibility modules are active.
if [ -z "$GTK_MODULES" ] ; then
    GTK_MODULES=gail:atk-bridge
else
    GTK_MODULES="$GTK_MODULES:gail:atk-bridge"
fi
export GTK_MODULES
