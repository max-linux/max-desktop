themedir = $(datadir)/icons/maxtoon

SIZES=16x16 22x22 32x32 48x48 64x64 72x72 96x96 128x128
#SIZES=16x16

all: build
clean:
	@for size in $(SIZES); do\
		echo "  ** Deleting dir $$size"; \
		rm -rf $$size;\
	done

distclean:


build: clean
	@for size in $(SIZES); do\
		_size=`echo $$size | cut -dx -f1`;\
		echo "Building theme for $$size size"; \
		for _dir in $(shell find scalable -mindepth 1 -type d |grep -v ".svn"); do\
			dir=`basename $$_dir`;\
			echo "  ** Create dir $$dir";\
			mkdir -p $$size/$$dir; \
		done; \
		for _specialfile in $(shell find scalable -type f -not -name "*.svg"| grep -v svn); do\
			dest=`echo $$_specialfile | sed "s|scalable/|$$size/|g"`; \
			echo "     ** Copy special file $$_specialfile";\
			cp $$_specialfile $$dest || true ;\
		done;\
		echo "   ** Converting files at size $$size...";\
		for _file in $(shell find scalable -type f -name "*.svg" |grep -v svn); do\
			dest=`echo $$_file | sed "s|scalable/|$$size/|g" | sed 's|.svg|.png|g'`; \
			#echo "    ** Convert file $$_file" ;\
			rsvg-convert -w $$_size -h $$_size -f png $$_file > $$dest;\
		done; \
		echo "   ** Linking files at size $$size...";\
		for _link in $(shell find scalable -type l |grep -v svn); do \
			dest=`readlink $$_link | sed 's|.svg|.png|g'`; \
			link=`basename $$_link | sed 's|.svg|.png|g'`; \
			dir=`dirname $$_link | sed "s|scalable/|$$size/|g"`; \
			#echo "    ** Creating link $$link => $$dest ($$dir)"; \
			(cd $$dir && ln -s $$dest $$link || true);\
		done; \
	done


install:
	rm -rf $(DESTDIR)/$(datadir)/icons/maxtoon
	install -d $(DESTDIR)/$(datadir)/icons/maxtoon

	install -m 644 index.theme $(DESTDIR)/$(datadir)/icons/maxtoon

	@for size in $(SIZES) scalable; do \
		echo "  Installing $$size icons";\
		for dir in `find $$size -type d | grep -v ".svn"`; do \
			#echo "     Creating dir $$dir"; \
			install -d $(DESTDIR)/$(themedir)/$$dir;\
		done; \
		for file in `find $$size -type f | grep -v ".svn"`; do\
			#echo "Installing file $$file in dir `dirname $$file`"; \
			install -m 644 $$file $(DESTDIR)/$(themedir)/`dirname $$file`;\
		done; \
		for link in `find $$size -type l`; do \
			#echo "Installing link $$link in dir `dirname $$link`"; \
			cp -a $$link $(DESTDIR)/$(themedir)/`dirname $$link`; \
		done; \
	done
