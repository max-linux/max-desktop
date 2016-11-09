INCLUDES = -I./                 \
	   -I/usr/include/plymouth-1/ply   \
           -I/usr/include/plymouth-1/ply-splash-core \
           -I/usr/include/plymouth-1/ply-splash-graphics \
           -I/usr/include/plymouth-1/ply-boot-client

plugindir = /usr/share/plymouth


CFLAGS=-Wall -g -O2 -c -Wno-sign-compare -Wno-deprecated-declarations

COLORS=	-DPLYMOUTH_BACKGROUND_COLOR=0x5d5950 \
	-DPLYMOUTH_BACKGROUND_END_COLOR=0x3a362f \
	-DPLYMOUTH_BACKGROUND_START_COLOR=0x807c71 

LDFLAGS=-module -avoid-version -export-dynamic -lm -lrt -ldl \
        /lib/libply.a /lib/libply-splash-core.a -lply -lply-splash-core

SOFLAGS=-shared -Wl,-soname,text.so

all: text.so


text.o: plugin.c
	gcc $(INCLUDES) $(CFLAGS) $(COLORS) -c -fPIC plugin.c -o text.o   

text.so: text.o
	gcc $(INCLUDES) $(LDFLAGS) $(SOFLAGS) -o text.so text.o
	strip text.so

install: text.so
	mkdir -p $(DESTDIR)/$(plugindir)/
	install -m 644 text.so $(DESTDIR)/$(plugindir)/

clean:
	rm -f text.so text.o
