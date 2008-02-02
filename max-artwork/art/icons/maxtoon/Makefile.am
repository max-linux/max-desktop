themedir = $(datadir)/icons/maxtoon



all:
clean:
distclean:

install:
	install -d $(DESTDIR)/$(datadir)/icons/maxtoon

	install -m 644 index.theme $(DESTDIR)/$(datadir)/icons/maxtoon

	@for dir in $(shell find scalable -type d); do \
		echo "Creating dir $$dir"; \
		install -d $(DESTDIR)/$(themedir)/$$dir;\
	done

	@for file in $(shell find scalable -type f); do\
		echo "Installing file $$file in dir `dirname $$file`"; \
		install -m 644 $$file $(DESTDIR)/$(themedir)/`dirname $$file`;\
	done

	@for link in $(shell find scalable -type l); do \
		echo "Installing link $$link in dir `dirname $$link`"; \
		cp -a $$link $(DESTDIR)/$(themedir)/`dirname $$link`; \
	done
