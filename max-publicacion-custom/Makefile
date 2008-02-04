SCRIPTDIR=$(DESTDIR)/usr/share/max

all: 

install:
	test -d $(SCRIPTDIR) || install -d -m 755 $(SCRIPTDIR)
	install -p -m 750 max-publicacion.sh $(SCRIPTDIR)
	test -d $(DESTDIR)/usr/share/applications/ || install -d -m 755 $(DESTDIR)/usr/share/applications/
	install -p -m 644 maxcontrol.desktop $(DESTDIR)/usr/share/applications/
	test -d $(DESTDIR)/usr/share/max/ || install -d -m 755 $(DESTDIR)/usr/share/max/
	install -p -m 644 maxcontrol.tgz $(DESTDIR)/usr/share/max/
	install -p -m 644 maxcontrol.png $(DESTDIR)/usr/share/max/
	install -p -m 755 htpasswd $(DESTDIR)/usr/share/max/
	install -p -m 644 maxcontrol.tgz $(DESTDIR)/usr/share/max/
	install -p -m 644 max-publicacion.sh $(SCRIPTDIR)
	install -p -m 644 000-default $(DESTDIR)/usr/share/max
	test -d $(DESTDIR)/etc/gdm/ || install -d -m 755 $(DESTDIR)/etc/gdm/
	install -p -m 644 gdm.conf $(DESTDIR)/etc/gdm/
	test -d $(DESTDIR)/home/madrid/Desktop || install -d -m 755 $(DESTDIR)/home/madrid/Desktop

