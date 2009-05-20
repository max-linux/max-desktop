CC=gcc
CFLAGS=-g -Wall -fPIC
LDFLAGS=
INCLUDES=

COMPILE = $(CC) $(INCLUDES) $(CFLAGS)
LINK = $(CC) $(CFLAGS) $(LDFLAGS)

INSTALL = /usr/bin/install -c
INSTALL_DATA = $(INSTALL) -m 644
INSTALL_PROGRAM = $(INSTALL) -m 755
sizes = 640_480 800_600 1024_768
usplashs = $(addsuffix .png.c.o, $(addprefix usplash_, $(sizes)))

max-splash.so: throbber_back.png.c.o throbber_back_16.png.c.o throbber_fore.png.c.o throbber_fore_16.png.c.o \
						 throbber_back_32.png.c.o throbber_fore_32.png.c.o \
						 $(usplashs) \
						 max-splash.c.o

	$(COMPILE) -shared -o $@ $^

cmap.gif: usplash.svg throbber_fore_16.png
	rsvg-convert -w 1024 -h 768 -f png usplash.svg | composite -compose src_over -gravity center throbber_fore_16.png png:- cmap.gif

usplash_%.png: usplash.svg cmap.gif
	rsvg-convert -w `echo $* | cut -d_ -f1` -h `echo $* | cut -d_ -f2` -f png $< > $@

%.png.c: %.png
	convert $< +dither -map cmap.gif gif:- | convert gif:- $<
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
