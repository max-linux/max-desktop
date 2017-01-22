#!/bin/bash

set -e

OUT="usr/share/max-mate/settings-overlay/config/plank/dock1/launchers"
SCHEMA="usr/share/glib-2.0/schemas/zz-max-mate-plank.gschema.override"

LIST="gcompris childsplay omnitux pysycache pysiogame org.kde.khangman tuxtype tuxmath tuxpaint org.kde.ktuberling pinta scratch"

mkdir -p "${OUT}"
rm -f "${SCHEMA}"
rm -f ${OUT}/*.dockitem

if [ "$1" = "clean" ]; then
  echo " INFO: exit only clean"
  exit
fi

i=0
for d in $LIST; do

  if [ ! -e "usr/share/mate/plank/${d}.desktop" ]; then
    echo " ERROR: usr/share/mate/plank/${d}.desktop no exists !!!"
    exit 1
  fi

  outfile=$(printf "%s/%02d_%s.dockitem" ${OUT} ${i} ${d})
  echo " * Writing to ${outfile}"
  cat << EOF > ${outfile}
[PlankDockItemPreferences]
Launcher=file:///usr/share/mate/plank/${d}.desktop
EOF


  i=$((i + 1))
done


i=0
ITEMS=$(for d in $LIST; do
  if [ "$d" = "scratch" ]; then
    printf "'%02d_%s.dockitem'" $i $d
  else
    printf "'%02d_%s.dockitem'," $i $d
  fi
  i=$((i + 1))
done
)

# dock-items=['desktop.dockitem','clock.dockitem','firefox.dockitem','matecc.dockitem']

cat << EOF > ${SCHEMA}

[net.launchpad.plank.dock.settings]
dock-items=[${ITEMS}]
hide-mode='window-dodge'
show-dock-item=true
theme='Transparent'
position='top'
pinned-only=true
lock-items=true
show-dock-item=false

[net.launchpad.plank.docks.dock1]
dock-items=[${ITEMS}]
hide-mode='window-dodge'
show-dock-item=true
theme='Transparent'
position='top'
pinned-only=true
lock-items=true
show-dock-item=false

EOF

