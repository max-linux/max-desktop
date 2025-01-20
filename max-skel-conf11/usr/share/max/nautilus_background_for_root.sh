#!/bin/sh

# disable in live
#if [ -d "/cdrom" ]; then
#  if [ ! -d "/cdrom/casper" ]; then
#    echo " * Detect chroot not configuring nautilus_background_for_root"
#    exit
#  fi
#fi

if grep -q "/cdrom" /proc/1/mounts; then
  echo " * Detect chroot not configuring nautilus_background_for_root"
  exit
fi

# set background color to red
mkdir -p /root/.config/.dconf
HOME=/root dbus-launch gsettings set org.mate.caja.preferences background-color "'#AD3841'"
HOME=/root dbus-launch gsettings set org.mate.caja.preferences background-set true
exit

if [ -d /root/.themes/MaxTheme ]; then
  #echo " * Root theme configured"
  exit
fi


mkdir -p /root/.themes/MaxTheme
cd /root/.themes/MaxTheme

for a in /usr/share/themes/MaxTheme/*; do
  ln -s $a
done

rm -f gtk-3.0
mkdir -p gtk-3.0/apps gtk-3.0/assets


IFS='
'
for f in $(find /usr/share/themes/MaxTheme/gtk-3.0/ -type f); do
  dest=$(echo $f| sed 's@/usr/share/themes/MaxTheme/@@g')
  ln -s "$f" "$dest"
done

rm gtk-3.0/apps/nautilus.css

cat << EOF > gtk-3.0/apps/nautilus.css
@define-color cluebar_color shade (mix (@bg_color, @base_color, 0.5), 0.95);

NautilusWindow * {
    -GtkPaned-handle-size: 1;
    background-color: #ff0000;
}

.nautilus-canvas-item {
    border-radius: 4px;
}

/* desktop mode */
.nautilus-desktop.nautilus-canvas-item {
    color: @bg_color;
    text-shadow: 1 1 alpha (#000000, 0.8);
}

.nautilus-desktop.nautilus-canvas-item:active {
    background-image: none;
    background-color: alpha (@bg_color, 0.84);

    color: @fg_color;
}

.nautilus-desktop.nautilus-canvas-item:selected {
    background-image: none;
    background-color: alpha (@selected_bg_color, 0.84);

    color: @selected_fg_color;
}

.nautilus-desktop.nautilus-canvas-item:active,
.nautilus-desktop.nautilus-canvas-item:prelight,
.nautilus-desktop.nautilus-canvas-item:selected {
    text-shadow: none;
}

/* browser window */
NautilusTrashBar.info,
NautilusXContentBar.info,
NautilusSearchBar.info,
NautilusQueryEditor.info {
    background-image: -gtk-gradient (linear, left top, left bottom,
                                     from (shade (@cluebar_color, 1.02)),
                                     to (shade (@cluebar_color, 0.98)));
    background-color: @cluebar_color;
    border-bottom-color: shade (@cluebar_color, 0.92);
    border-radius: 0;
    border-style: solid;
    border-width: 0px 0px 1px 0px;

    -unico-border-gradient: none;
    -unico-inner-stroke-gradient: -gtk-gradient (linear, left top, left bottom,
                                                 from (shade (@cluebar_color, 1.04)),
                                                 to (shade (@cluebar_color, 1.01)));
}

NautilusTrashBar.info:backdrop,
NautilusXContentBar.info:backdrop,
NautilusSearchBar.info:backdrop,
NautilusQueryEditor.info:backdrop {
    background-image: -gtk-gradient (linear, left top, left bottom,
                                     from (shade (@cluebar_color, 1.01)),
                                     to (shade (@cluebar_color, 0.99)));
    background-color: @cluebar_color;
    border-bottom-color: shade (@cluebar_color, 0.92);

    -unico-inner-stroke-gradient: -gtk-gradient (linear, left top, left bottom,
                                                 from (shade (@cluebar_color, 1.02)),
                                                 to (shade (@cluebar_color, 1.0)));
}

NautilusSearchBar .entry {
}

.nautilus-cluebar-label {
    font: bold;
    text-shadow: 0 1 shade (@cluebar_color, 1.06);
}

.nautilus-cluebar-label:backdrop {
    color: mix (@fg_color, @cluebar_color, 0.2);
    text-shadow: 0 1 shade (@cluebar_color, 1.02);
}

#nautilus-search-button *:active,
#nautilus-search-button *:active:prelight {
    color: @dark_fg_color;
}

NautilusFloatingBar {
    background-color: @info_bg_color;
    border-radius: 3px 3px 0 0;
    border-style: solid;
    border-width: 1px;
    border-color: darker (@info_bg_color);

    -unico-border-gradient: none;
}

NautilusFloatingBar .button {
    -GtkButton-image-spacing: 0;
    -GtkButton-inner-border: 0;
}

/* sidebar */
NautilusWindow .sidebar,
NautilusWindow .sidebar .view {
    background-color: @dark_bg_color;

    color: @fg_color;
    text-shadow: 0 1 shade (@dark_bg_color, 1.04); 
}

NautilusWindow .sidebar:backdrop,
NautilusWindow .sidebar .view:backdrop {
    color: mix (@fg_color, @dark_bg_color, 0.2);
    text-shadow: 0 1 shade (@dark_bg_color, 1.02); 
}

NautilusWindow .sidebar row:selected {
    color: @selected_fg_color;
    text-shadow: 0 -1 shade (@selected_bg_color, 0.8); 
}

NautilusWindow .sidebar row:selected:backdrop {
    background-image: -gtk-gradient (linear, left top, left bottom,
                                     from (shade (@dark_bg_color, 0.94)),
                                     to (shade (@dark_bg_color, 0.86)));
    border-top-color: shade (@dark_bg_color, 0.88);

    color: @fg_color;
    text-shadow: 0 1 shade (@dark_bg_color, 0.96); 
}

NautilusWindow .sidebar .frame {
}

NautilusWindow .pane-separator {
    background-color: shade (@dark_bg_color, 0.94);
    border-color: @dark_bg_color;
    border-style: solid;
    border-width: 0;

    -unico-border-gradient: none;
    -unico-inner-stroke-width: 0;
}

NautilusWindow .pane-separator:backdrop {
    background-color: shade (@dark_bg_color, 0.98);
}

EOF
