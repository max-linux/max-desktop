datadir = /usr/share/
themedir = $(datadir)/icons/Papirus-Light

##SIZES=16x16 22x22 24x24 32x32 48x48 64x64 72x72 96x96 128x128
#SIZES=16x16

SIZES=16x16 22x22 24x24 32x32 48x48 64x64


all: build
clean:
	@for size in $(SIZES); do\
		echo "  ** Deleting dir $$size"; \
		rm -rf $$size;\
	done

distclean:

printindextheme:
	@for size in $(SIZES); do\
		_size=`echo $$size | cut -dx -f1`;\
		for _dir in $(shell find scalable -mindepth 1 -type d |grep -v ".svn"); do\
			dir=`echo $$_dir | sed "s|scalable/|$$size/|g"`; \
			echo -n "$$dir,";\
		done; \
	done
	@for size in $(SIZES); do\
		_size=`echo $$size | cut -dx -f1`;\
		for _dir in $(shell find scalable -mindepth 1 -type d |grep -v ".svn"); do\
			dir=`echo $$_dir | sed "s|scalable/|$$size/|g"`; \
			context=`basename $$dir| sed 's/\([a-z]\)\([a-zA-Z0-9]*\)/\u\1\2/g'` ;\
			echo  "[$$dir]";\
			echo  "Size=$$_size";\
			echo  "Context=$$context";\
			echo  "Type=Fixed";\
			echo  "";\
		done; \
	done



build:
	@for size in $(SIZES); do\
		_size=`echo $$size | cut -dx -f1`;\
		echo "Building theme for $$size size"; \
		for _dir in $(shell find scalable -mindepth 1 -type d |grep -v ".svn"); do\
			dir=`echo $$_dir | sed "s|scalable/|$$size/|g"`; \
			echo "    ** Create dir $$dir";\
			mkdir -p $$dir; \
		done; \
		for _specialfile in $(shell find scalable -type f -not -name "*.svg"| grep -v svn); do\
			dest=`echo $$_specialfile | sed "s|scalable/|$$size/|g"`; \
			echo "     ** Copy special file $$_specialfile";\
			cp $$_specialfile $$dest || true ;\
		done;\
		echo "   ** Converting files at size $$size...";\
		for _file in $(shell find scalable -type f -name "*.svg" |grep -v svn); do\
			dest=`echo $$_file | sed "s|scalable/|$$size/|g" | sed 's|.svg|.png|g'`; \
			echo "    ** Convert file $$_file" ;\
			(rsvg-convert -w $$_size -h $$_size -f png $$_file > $$dest || echo " WARN NO BIN rsvg-convert -w $$_size -h $$_size -f png $$_file > $$dest");\
		done; \
		echo "   ** Linking files at size $$size...";\
		for _link in $(shell find scalable -type l |grep -v svn); do \
			dest=`readlink $$_link | sed 's|.svg|.png|g'`; \
			link=`basename $$_link | sed 's|.svg|.png|g'`; \
			dir=`dirname $$_link | sed "s|scalable/|$$size/|g"`; \
			echo "    ** Creating link $$link => $$dest ($$dir)"; \
			(cd $$dir && ln -s $$dest $$link || true);\
		done; \
	done


install:
	rm -rf $(DESTDIR)/$(datadir)/icons/Papirus-Light
	install -d $(DESTDIR)/$(datadir)/icons/Papirus-Light

#	install -m 644 index.theme $(DESTDIR)/$(datadir)/icons/Papirus-Light

	@for size in $(SIZES) scalable; do \
		echo "  Installing $$size icons";\
		for dir in `find $$size -type d | grep -v ".svn"`; do \
			echo "     Creating dir $$dir"; \
			install -d $(DESTDIR)/$(themedir)/$$dir;\
		done; \
		for file in `find $$size -type f | grep -v ".svn"`; do\
			echo "Installing file $$file in dir `dirname $$file`"; \
			install -m 644 $$file $(DESTDIR)/$(themedir)/`dirname $$file`;\
		done; \
		for link in `find $$size -type l`; do \
			echo "Installing link $$link in dir `dirname $$link`"; \
			cp -a $$link $(DESTDIR)/$(themedir)/`dirname $$link`; \
		done; \
	done
