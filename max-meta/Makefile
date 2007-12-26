SCRIPTDIR = $(DESTDIR)/usr/share/max

all: 

clean:
	rm -f *~ desktop-sin-dupli-i386 desktop-all-i386 desktop-dupli-i386

install:
	test -d $(SCRIPTDIR) || install -d -m 755 $(SCRIPTDIR)
	install -p -m 644 max-meta.sh $(SCRIPTDIR)
	install -p -m 644 max-server.png $(SCRIPTDIR)

