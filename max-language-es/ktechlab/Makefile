
all: ktechlab.mo



clean:
	rm -f *.mo


ktechlab.mo:
	msgfmt -v ktechlab.po -o ktechlab.mo


install:
	mkdir -p $(DESTDIR)/usr/share/dpsyco/skel/usr/share/locale/es/LC_MESSAGES
	cp ktechlab.mo $(DESTDIR)/usr/share/dpsyco/skel/usr/share/locale/es/LC_MESSAGES/

	mkdir -p $(DESTDIR)/usr/share/dpsyco/skel/usr/share/apps/ktechlab/contexthelp/
	cp help-es $(DESTDIR)/usr/share/dpsyco/skel/usr/share/apps/ktechlab/contexthelp/

	mkdir -p $(DESTDIR)/usr/share/apps/ktechlab/contexthelp
	cp fccc.png xor3.png $(DESTDIR)/usr/share/apps/ktechlab/contexthelp/


