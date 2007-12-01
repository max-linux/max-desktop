themedir=/usr/share/icons/Tango-Brown
size = 24x24
context=emotes

iconsdir = $(themedir)/$(size)/$(context)

icons_DATA =                            \
	face-angel.png	\
	face-crying.png	\
	face-devil-grin.png	\
	face-glasses.png	\
	face-grin.png	\
	face-kiss.png	\
	face-sad.png	\
	face-smile-big.png	\
	face-smile.png	\
	face-surprise.png	\
	face-wink.png	

EXTRA_DIST =				\
	$(icons_DATA)

install-data-local: install-iconsDATA

uninstall-local:
	rm -rf $(themedir)/24x24/$(context)
