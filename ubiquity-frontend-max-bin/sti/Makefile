all: stidump

stidump: stidump.c
	gcc -Wall -O2 stidump.c -o stidump

clean:
	rm -f stidump *~

install: stidump
	install -d $(DESTDIR)/usr/bin
	install -m 755 stidump  $(DESTDIR)/usr/bin
	install -m 755 test-sti $(DESTDIR)/usr/bin
