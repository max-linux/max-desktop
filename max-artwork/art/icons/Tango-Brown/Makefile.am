themedir = $(datadir)/icons/Tango-Brown

SUBDIRS = 16x16 22x22 24x24 scalable

@INTLTOOL_THEME_RULE@

theme_DATA = index.theme

install-data-local:
		if [ -h $(DESTDIR)$(themedir) ]; then \
				rm -f $(DESTDIR)$(themedir); \
		fi
		if [ -d $(DESTDIR)$(themedir) ]; then \
				touch $(DESTDIR)$(themedir); \
		fi
