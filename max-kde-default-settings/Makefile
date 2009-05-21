
VERSION = 5.03

WALLPAPER_DIR = $(DESTDIR)/usr/share/wallpapers
SOUNDS_DIR = $(DESTDIR)/usr/share/sounds
KONQ_TILES_DIR = $(DESTDIR)/usr/share/apps/konqueror/tiles
KSPLASH_THEME_DIR = $(DESTDIR)/usr/share/apps/ksplash/Themes/kubuntu
SETTINGS_DIR = $(DESTDIR)/usr/share/max-kde-default-settings
CURSOR_DIR = $(DESTDIR)/usr/share/icons/contrastlarge
KCONF_UPDATE_DIR = $(DESTDIR)/usr/share/apps/kconf_update
KSPLASH_THEME_DIR = $(DESTDIR)/usr/share/kde4/apps/ksplash/Themes

build:
	#make -C usplash

clean:
	#make -C usplash clean

install:
	#make -C usplash install

	# The default wallpaper comes from kdebase now...
	#mkdir -p "$(WALLPAPER_DIR)"
	#install -m644 artwork/kubuntu-wallpaper.jpg  #"$(WALLPAPER_DIR)"
	#install -m644 artwork/kubuntu-wallpaper.jpg.desktop  #"$(WALLPAPER_DIR)"

	# We're using KDE's default login sound this time around
	# mkdir -p "$(SOUNDS_DIR)"
	# install -m644 artwork/kubuntu-login.ogg  "$(SOUNDS_DIR)"

	mkdir -p "$(KSPLASH_THEME_DIR)"
	cp -r ksplash/max  "$(KSPLASH_THEME_DIR)"


	mkdir -p "$(DESTDIR)/etc"
	install -m644 kde-profile/kderc "$(DESTDIR)/etc"
	install -m644 kde-profile/kde4rc "$(DESTDIR)/etc"
	install -m644 kde-profile/kde-user-profile "$(DESTDIR)/etc"

	mkdir -p "$(DESTDIR)/etc/qt3"
	install -m644 qt-rc-files/*rc "$(DESTDIR)/etc/qt3"

	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/config/"
	install -m644 kde3/kde-rc-files/* "$(SETTINGS_DIR)/kde-profile/default/share/config/"
	# Apparently we're just using the KDE defaults here too.
	#install -m644 kde-rc-files/uiserver "$(SETTINGS_DIR)/kde-profile/default/share/config/"
	install -m644 kde3/kde-rc-files/kdeglobals "$(SETTINGS_DIR)/kde-profile/default/share/config/"
	install -m644 kde3/kde-rc-files/cryptodefaults "$(SETTINGS_DIR)/kde-profile/default/share/config/"

	mkdir -p "$(SETTINGS_DIR)/kde4-profile/default/share/config"
	install -m644 kde-rc-files/*rc "$(SETTINGS_DIR)/kde4-profile/default/share/config/"
	install -m644 kde-rc-files/cryptodefaults "$(SETTINGS_DIR)/kde4-profile/default/share/config/"
	#kdeglobals doesn't seem to work in our profile dir, also put a copy in /etc/kde4
	install -m644 kde-rc-files/kdeglobals "$(SETTINGS_DIR)/kde4-profile/default/share/config/"
	mkdir -p "$(DESTDIR)/etc/kde4/"
	install -m644 kde-rc-files/kdeglobals "$(DESTDIR)/etc/kde4/"

	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/apps/color-schemes/"
	install -m644 color-schemes/*colors "$(SETTINGS_DIR)/kde-profile/default/share/apps/color-schemes/"

	mkdir -p "$(CURSOR_DIR)/cursors"
	install -m644 cursors/index.theme "$(CURSOR_DIR)/"
	mkdir -p "$(CURSOR_DIR)cursors/"
	install -m644 cursors/contrastlarge/cursors/* "$(CURSOR_DIR)/cursors/"

	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/apps/konqueror/"
	install -m644 ui-rc-files/konqueror.rc "$(SETTINGS_DIR)/kde-profile/default/share/apps/konqueror/"
#	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/apps/konversation/"
#	install -m644 ui-rc-files/konversationui.rc "$(SETTINGS_DIR)/kde-profile/default/share/apps/konversation/"
	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/apps/kopete/"
	install -m644 ui-rc-files/kopetechatwindow.rc "$(SETTINGS_DIR)/kde-profile/default/share/apps/kopete/"

	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/apps/konqueror/servicemenus/"
	install -m644 konqueror-profile/email.desktop "$(SETTINGS_DIR)/kde-profile/default/share/apps/konqueror/servicemenus/email.desktop"
	mkdir -p "$(DESTDIR)/usr/bin"
	install -m644 konqueror-profile/konq_plugins.xml "$(SETTINGS_DIR)/kde-profile/default/share/apps/konqueror/"

	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/icons/crystalsvg/16x16/actions/"
	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/icons/crystalsvg/22x22/actions/"
	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/icons/crystalsvg/32x32/actions/"
	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/icons/crystalsvg/48x48/actions/"

	install -m644 kde3/artwork/throbber/16x16/kde.png "$(SETTINGS_DIR)/kde-profile/default/share/icons/crystalsvg/16x16/actions/"
	install -m644 kde3/artwork/throbber/22x22/kde.png "$(SETTINGS_DIR)/kde-profile/default/share/icons/crystalsvg/22x22/actions/"
	install -m644 kde3/artwork/throbber/32x32/kde.png "$(SETTINGS_DIR)/kde-profile/default/share/icons/crystalsvg/32x32/actions/"
	install -m644 kde3/artwork/throbber/48x48/kde.png "$(SETTINGS_DIR)/kde-profile/default/share/icons/crystalsvg/48x48/actions/"

	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/icons/crystalsvg/64x64/actions/"
	install -m644 kde3/kaffeine/fileopen64.png "$(SETTINGS_DIR)/kde-profile/default/share/icons/crystalsvg/64x64/actions/fileopen.png"
	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/icons/crystalsvg/128x128/actions/"
	install -m644 kde3/kaffeine/fileopen128.png "$(SETTINGS_DIR)/kde-profile/default/share/icons/crystalsvg/128x128/actions/fileopen.png"
	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/icons/crystalsvg/scalable/actions/"

	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/apps/kaffeine/"
	install -m644 kde3/kaffeine/wizard_stamp_v0.7.1 "$(SETTINGS_DIR)/kde-profile/default/share/apps/kaffeine"

	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/apps/ark/"
	install -m644 kpart-rc-files/ark_part.rc "$(SETTINGS_DIR)/kde-profile/default/share/apps/ark/"
	install -m644 kpart-rc-files/ark_part_readonly.rc "$(SETTINGS_DIR)/kde-profile/default/share/apps/ark/"

	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/apps/khtml/"
	install -m644 kpart-rc-files/khtml_browser.rc "$(SETTINGS_DIR)/kde-profile/default/share/apps/khtml/"

	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/mimelnk/application/"
	install -m644 mimelnk/x-tar.desktop "$(SETTINGS_DIR)/kde-profile/default/share/mimelnk/application/"
	install -m644 mimelnk/x-tarz.desktop "$(SETTINGS_DIR)/kde-profile/default/share/mimelnk/application/"
	install -m644 mimelnk/x-tbz.desktop "$(SETTINGS_DIR)/kde-profile/default/share/mimelnk/application/"
	install -m644 mimelnk/x-tgz.desktop "$(SETTINGS_DIR)/kde-profile/default/share/mimelnk/application/"
	install -m644 mimelnk/x-zip.desktop "$(SETTINGS_DIR)/kde-profile/default/share/mimelnk/application/"

	mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/apps/kwallet/"
	install -m644 kwallet-default-wallet/kdewallet.kwl "$(SETTINGS_DIR)/kde-profile/default/share/apps/kwallet/"

	# We're not using this for Intrepid
	#mkdir -p "$(SETTINGS_DIR)/kde-profile/default/share/apps/kwin/"
	#install -m644 artwork/window-decoration-gradient.png "$(SETTINGS_DIR)/kde-profile/default/share/apps/kwin/"
	#install -m644 artwork/window-decoration-gradient-inactive.png "$(SETTINGS_DIR)/kde-profile/default/share/apps/kwin/"

	install -m644 gtk2-engines-qtcurve-rc-files/dot-gtkrc-2.0-kde4 "$(SETTINGS_DIR)/dot-gtkrc-2.0-kde4"
	install -m744 gtk2-engines-qtcurve-rc-files/gtk2-engines-qtcurve.rc.sh "$(SETTINGS_DIR)/gtk2-engines-qtcurve.rc.sh"
	install -m644 hal/hal-policy-ntfs-config-write-policy.fdi "$(SETTINGS_DIR)/hal-policy-ntfs-config-write-policy.fdi"

	#TODO: polish the accessibility profiles

	#mkdir -p "$(SETTINGS_DIR)/kde4-profile/lesser-visual-impairment/share/config/"
	#cp -r accessibility/lesser-visual-impairment/* "$(SETTINGS_DIR)/kde4-profile/lesser-visual-impairment/share/config/"
	#rm "$(SETTINGS_DIR)/kde4-profile/lesser-visual-impairment/share/config/TODO_kickerrc2plasmarc" # TODO needs porting

	#mkdir -p "$(SETTINGS_DIR)/kde4-profile/moderate-visual-impairment/share/config/"
	#cp -r accessibility/moderate-visual-impairment/* "$(SETTINGS_DIR)/kde4-profile/moderate-visual-impairment/share/config/"
	#rm "$(SETTINGS_DIR)/kde4-profile/moderate-visual-impairment/share/config/TODO_kickerrc2plasmarc" # TODO needs porting

	#mkdir -p "$(SETTINGS_DIR)/kde4-profile/minor-motor-difficulties/share/config/"
	#cp -r accessibility/minor-motor-difficulties/* "$(SETTINGS_DIR)/kde4-profile/minor-motor-difficulties/share/config/"
	#rm "$(SETTINGS_DIR)/kde4-profile/minor-motor-difficulties/share/config/TODO_kickerrc2plasmarc" # TODO needs porting

	#mkdir -p "$(SETTINGS_DIR)/kde4-profile/motor-difficulties-pointing-devices/share/config/"
	#cp -r accessibility/motor-difficulties-pointing-devices/* "$(SETTINGS_DIR)/kde4-profile/motor-difficulties-pointing-devices/share/config/"

	mkdir -p "$(DESTDIR)/etc/max-kde-default-settings"
	install -m644 kde-hidden/hidden-media "$(DESTDIR)/etc/max-kde-default-settings/"
	install -m644 kde-hidden/directory-media "$(DESTDIR)/etc/max-kde-default-settings/"
	install -m644 kde-hidden/directory-home "$(DESTDIR)/etc/max-kde-default-settings/"

	mkdir -p "$(SETTINGS_DIR)/kde4-profile/default/share/apps/desktoptheme/default/widgets/"
	install -p -D -m644 artwork/branding.svgz "$(SETTINGS_DIR)/kde4-profile/default/share/apps/desktoptheme/default/widgets/"

	mkdir -p "$(SETTINGS_DIR)/kde4-profile/default/share/apps/kwin/"
	install -p -D -m644 artwork/cubecap.png "$(SETTINGS_DIR)/kde4-profile/default/share/apps/kwin/"

	mkdir -p "$(KCONF_UPDATE_DIR)"
	install -m644 kconf_update/kubuntu-fix-kwin-default-size.upd $(KCONF_UPDATE_DIR)/
	install -m755 kconf_update/kubuntu-fix-kwin-default-size.py $(KCONF_UPDATE_DIR)/
