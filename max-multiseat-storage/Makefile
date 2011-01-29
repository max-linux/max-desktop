
all: build


install: build make-install-dirs
	install -m 755 nautilus-umount-multiseat.py $(DESTDIR)/usr/lib/nautilus/extensions-2.0/python/nautilus-umount-multiseat.py
	install -m 755 multiseat-udisks.py $(DESTDIR)/usr/sbin/multiseat-udisks
	install -m 755 src/umount.multiseat $(DESTDIR)/sbin/umount.multiseat


make-install-dirs:
	mkdir -p $(DESTDIR)/usr/lib/nautilus/extensions-2.0/python/
	mkdir -p $(DESTDIR)/usr/sbin/
	mkdir -p $(DESTDIR)/sbin/

clean:
	rm -f src/umount.multiseat


build: src/umount.multiseat.c
	gcc -O2 -ansi -pedantic -Wall src/umount.multiseat.c -o src/umount.multiseat

test: clean build
	chmod +s src/umount.multiseat
	su madrid -c 'src/umount.multiseat /dev/sdc1'
