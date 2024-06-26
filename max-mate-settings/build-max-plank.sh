#!/bin/bash

set -e

OUT="usr/share/max-mate/settings-overlay/config/plank/dock1/launchers"
SCHEMA="usr/share/glib-2.0/schemas/zz-max-mate-plank.gschema.override"

LIST="educamadrid max-educamadrid max-nextcloud firefox thunderbird max-search-apps mate-screenshot galculator simplescreenrecorder libreoffice-startcenter"

mkdir -p "${OUT}"
rm -f "${SCHEMA}"
rm -f ${OUT}/*.dockitem

if [ "$1" = "clean" ]; then
  echo " INFO: exit only clean"
  exit
fi

i=0
for d in $LIST; do

  if [ ! -e "usr/share/mate/plank/normal/${d}.desktop" ]; then
    echo " ERROR: usr/share/mate/plank/normal/${d}.desktop no exists !!!"
    exit 1
  fi

  outfile=$(printf "%s/%02d_%s.dockitem" ${OUT} ${i} ${d})
  echo " * Writing to ${outfile}"
  cat << EOF > ${outfile}
[PlankDockItemPreferences]
Launcher=file:///usr/share/mate/plank/normal/${d}.desktop
EOF


  i=$((i + 1))
done

# create desktop launcher in last position
outfile=$(printf "%s/%02d_desktop.dockitem" ${OUT} ${i})

cat << EOF > ${outfile}
[PlankDockItemPreferences]
Launcher=docklet://desktop
EOF

# create clippy launcher in last position
i=$((i + 1))
outfile=$(printf "%s/%02d_clippy.dockitem" ${OUT} ${i})

cat << EOF > ${outfile}
[PlankDockItemPreferences]
Launcher=docklet://clippy

[DockyClippyPreferences]
MaxEtries=15
TrackMouseSelections=false
EOF


i=0
ITEMS=$(for d in $LIST; do
  if [ "$d" = "libreoffice-startcenter" ]; then
    printf "'%02d_%s.dockitem', '%s'" $i $d `basename ${outfile}`
  else
    printf "'%02d_%s.dockitem', " $i $d
  fi
  i=$((i + 1))
done
)

# dock-items=['desktop.dockitem','clock.dockitem','firefox.dockitem','matecc.dockitem']

cat << EOF > ${SCHEMA}

[net.launchpad.plank.dock.settings]
dock-items=[${ITEMS}]
hide-mode='intelligent'
show-dock-item=false
theme='Transparent'
position='bottom'
pinned-only=false
lock-items=true
show-dock-item=false
hide-delay=1000
unhide-delay=1000
icon-size=50

[net.launchpad.plank.docks.dock1]
dock-items=[${ITEMS}]
hide-mode='intelligent'
show-dock-item=false
theme='Transparent'
position='bottom'
pinned-only=false
lock-items=true
show-dock-item=false
hide-delay=1000
unhide-delay=1000
icon-size=50

EOF

