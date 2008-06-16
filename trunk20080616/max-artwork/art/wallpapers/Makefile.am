wallpaperdir = $(datadir)/backgrounds
wallpaper_DATA = \
	max.png

props_in_files = \
	max-wallpapers.xml.in

propsdir = $(datadir)/gnome-background-properties
props_DATA = \
	max-wallpapers.xml

@INTLTOOL_XML_RULE@

EXTRA_DIST = $(wallpaper_DATA) $(props_in_files) $(props_DATA)

DISTCLEANFILES = \
	max-wallpapers.xml
