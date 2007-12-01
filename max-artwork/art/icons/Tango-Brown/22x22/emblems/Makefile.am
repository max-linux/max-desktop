themedir=/usr/share/icons/Tango-Brown
size = 22x22
context=emblems

iconsdir = $(themedir)/$(size)/$(context)

icons_DATA =                            \
	emblem-favorite.png	\
	emblem-important.png	\
	emblem-photos.png	\
	emblem-readonly.png	\
	emblem-symbolic-link.png	\
	emblem-system.png	\
	emblem-unreadable.png	\
	stock_mail-priority-high.png	

EXTRA_DIST =						\
	$(icons_DATA)

install-data-local: install-iconsDATA

uninstall-local:
	rm -rf $(themedir)/22x22/$(context)
