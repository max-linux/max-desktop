themedir = $(datadir)/icons/gartoon

@INTLTOOL_THEME_RULE@

16x16/places/distributor-logo.png:
	mkdir -p 16x16/places/
	rsvg-convert -w 16 -h 16 -f png -o 16x16/places/distributor-logo.png scalable/places/distributor-logo.svg

22x22/places/distributor-logo.png:
	mkdir -p 22x22/places/
	rsvg-convert -w 22 -h 22 -f png -o 22x22/places/distributor-logo.png scalable/places/distributor-logo.svg

24x24/places/distributor-logo.png:
	mkdir -p 24x24/places/
	rsvg-convert -w 24 -h 24 -f png -o 24x24/places/distributor-logo.png scalable/places/distributor-logo.svg

32x32/places/distributor-logo.png:
	mkdir -p 32x32/places/
	rsvg-convert -w 32 -h 32 -f png -o 32x32/places/distributor-logo.png scalable/places/distributor-logo.svg


# r! (find scalable -type f -name "*.svg" -o -name "*.icon"; find cursors -type f -o -type l) | sed 's,^,\t,;s,$, \\,'
nobase_dist_theme_DATA = \
	scalable/places/distributor-logo.svg \
	16x16/places/distributor-logo.png \
	22x22/places/distributor-logo.png \
	24x24/places/distributor-logo.png \
	32x32/places/distributor-logo.png 

clean:
	rm -rf 16x16/ 22x22/ 24x24/ 32x32/
