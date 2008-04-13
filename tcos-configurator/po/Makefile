LANGUAGES=es
PACKAGE=tcos-configurator

all:


clean:
	@rm -f *.mo *.gmo *autosave
	@for lang in $(LANGUAGES); do \
		echo " **CLEAN** $$lang"; \
		rm -rf $$lang;\
	done

files:
	find ../ -name "*.py" -o -name "*.glade" > FILES
pot:
	xgettext  -o $(PACKAGE).pot --files-from=FILES

po:
	@for lang in $(LANGUAGES); do \
		echo " **PO** $$lang"; \
		if [ ! -f $$lang.po ]; then \
			msginit --input $(PACKAGE).pot -o $$lang-old.po;\
		else \
			mv $$lang.po $$lang-old.po; \
		fi ; \
		msgmerge -o $$lang.po $$lang-old.po $(PACKAGE).pot; \
		rm -f $$lang-old.po; \
	done

gmo:
	@echo "Compiling languages: $(LANGUAGES)"
	@for lang in $(LANGUAGES); do \
		echo " **MO** $$lang" ; \
		mkdir -p $$lang/LC_MESSAGES/; \
		msgfmt -o $$lang/LC_MESSAGES/$(PACKAGE).mo $$lang.po; \
		touch $$lang.gmo; \
	done
	

install: gmo
	@for lang in $(LANGUAGES); do \
		echo " **INSTALL MO** $$lang" ; \
		mkdir -p $(DESTDIR)/$(PREFIX)/share/locale/$$lang/LC_MESSAGES/; \
		echo "    Installing $$lang.mo into $(DESTDIR)/$(PREFIX)/share/locale/$$lang/LC_MESSAGES/$(PACKAGE).mo"; \
		msgfmt -o $(DESTDIR)/$(PREFIX)/share/locale/$$lang/LC_MESSAGES/$(PACKAGE).mo $$lang.po; \
	done
