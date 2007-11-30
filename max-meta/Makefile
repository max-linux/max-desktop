SCRIPTDIR = $(DESTDIR)/usr/share/max

all: 

install:
	test -d $(SCRIPTDIR) || install -d -m 755 $(SCRIPTDIR)
	install -p -m 644 max-meta.sh $(SCRIPTDIR)
	install -p -m 644 max-server.png $(SCRIPTDIR)

