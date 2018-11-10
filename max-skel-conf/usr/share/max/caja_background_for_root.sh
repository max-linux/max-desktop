#!/bin/sh

if [ -e /root/.config/gtk-3.0/gtk.css ]; then
  exit
fi

mkdir -p /root/.config/gtk-3.0/
cat << EOF > /root/.config/gtk-3.0/gtk.css

.caja-notebook .view {
  background-color: #a31912;
}

EOF

echo " * Configured background for Caja file manager"
