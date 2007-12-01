themedir=/usr/share/icons/Tango-Brown
size = 22x22
context=status

iconsdir = $(themedir)/$(size)/$(context)

icons_DATA =                            \
	battery-caution.png	\
	dialog-error.png	\
	dialog-information.png	\
	dialog-warning.png	\
	edittrash.png	\
	error.png	\
	folder-drag-accept.png	\
	folder_open.png	\
	folder-open.png	\
	folder-visiting.png	\
	gnome-dev-wavelan-encrypted.png	\
	gnome-fs-directory-accept.png	\
	gnome-fs-directory-visiting.png	\
	gnome-fs-loading-icon.png	\
	gnome-fs-trash-full.png	\
	gnome-stock-trash-full.png	\
	gtk-dialog-error.png	\
	gtk-dialog-info.png	\
	gtk-dialog-warning.png	\
	gtk-missing-image.png	\
	image-loading.png	\
	image-missing.png	\
	important.png	\
	info.png	\
	mail-attachment.png	\
	messagebox_critical.png	\
	messagebox_info.png	\
	messagebox_warning.png	\
	network-wireless-encrypted.png	\
	stock_dialog-error.png	\
	stock_dialog-info.png	\
	stock_dialog-warning.png	\
	stock_open.png	\
	stock_trash_full.png	\
	trashcan_full.png	\
	user-trash-full.png	

EXTRA_DIST =						\
	$(icons_DATA)

install-data-local: install-iconsDATA

uninstall-local:
	rm -rf $(themedir)/22x22/$(context)
