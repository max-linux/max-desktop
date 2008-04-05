all:

clean:
	rm -f *~ *glade.* *gladep*
	rm -rf pixmaps
	cd po && make clean

install: translate
	install -d $(DESTDIR)/$(PREFIX)/sbin
	install -m 755 tcos-configurator.py $(DESTDIR)/$(PREFIX)/sbin/tcos-configurator

	install -d $(DESTDIR)/$(PREFIX)/share/tcos-configurator
	install -d $(DESTDIR)/$(PREFIX)/share/tcos-configurator/images

	install -m 644 tcos-configurator.glade $(DESTDIR)/$(PREFIX)/share/tcos-configurator
	install -m 644 images/tcos-icon-32x32.png $(DESTDIR)/$(PREFIX)/share/tcos-configurator/images
	install -m 644 images/tcos-logo.png $(DESTDIR)/$(PREFIX)/share/tcos-configurator/images

	install -d $(DESTDIR)/$(PREFIX)/share/applications
	install -m 644 tcos-configurator.desktop $(DESTDIR)/$(PREFIX)/share/applications/

	# locales
	cd po && make install DESTDIR=$(DESTDIR) PREFIX=$(PREFIX)


pot:
	cd po && make pot

update-po:
	cd po && make po

translate:
	cd po && make gmo

