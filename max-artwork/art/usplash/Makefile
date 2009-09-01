CC=gcc
CFLAGS=-g -Wall -fPIC
LDFLAGS=
INCLUDES=

COMPILE = $(CC) $(INCLUDES) $(CFLAGS)
LINK = $(CC) $(CFLAGS) $(LDFLAGS)

INSTALL = /usr/bin/install -c
INSTALL_DATA = $(INSTALL) -m 644
INSTALL_PROGRAM = $(INSTALL) -m 755

max-splash.so: throbber_back.png.c.o throbber_back_16.png.c.o throbber_fore.png.c.o throbber_fore_16.png.c.o \
						 throbber_back_32.png.c.o throbber_fore_32.png.c.o \
						 usplash_640_480.png.c.o usplash_800_600.png.c.o usplash_1024_768.png.c.o \
						 max-splash.c.o

	$(COMPILE) -shared -o $@ $^

%.png.c: %.png
	pngtousplash $< > $@

%.bdf.c: %.bdf
	bdftousplash $< > $@

%.c.o: %.c
	$(COMPILE) -o $@ -c $<

install: all
	$(INSTALL) -d $(DESTDIR)/usr/lib/usplash
	$(INSTALL_PROGRAM) max-splash.so $(DESTDIR)/usr/lib/usplash/max-splash.so
clean:
	rm -f *.png.c *.bdf.c *.c.o *.so

all: max-splash.so
