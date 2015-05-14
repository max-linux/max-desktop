# -*- coding: utf-8; Mode: Python; indent-tabs-mode: nil; tab-width: 4 -*-
#
# Copyright (C) 2009 Canonical Ltd.
# Written by Michael Terry <michael.terry@canonical.com>.
#
# This file is part of Ubiquity.
#
# Ubiquity is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Ubiquity is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Ubiquity.  If not, see <http://www.gnu.org/licenses/>.

import os
import sys
import syslog
from ubiquity import plugin
from subprocess import Popen, PIPE, STDOUT

NAME = 'intro'
AFTER = None
WEIGHT = 100


def get_intro(fname):
    intro_file = open(fname)
    text = intro_file.read()
    intro_file.close()
    return text


class PageBase(plugin.PluginUI):
    def __init__(self):
        self.intro_file="/usr/share/ubiquity/intro.txt"
        self.intro_file_nano="/usr/share/ubiquity/intro-nanomax.txt"


class PageGtk(PageBase):
    def __init__(self, controller, *args, **kwargs):
        PageBase.__init__(self, *args, **kwargs)
        self.controller = controller
        

        from gi.repository import Gtk
        builder = Gtk.Builder()
        self.controller.add_builder(builder)
        builder.add_from_file('/usr/share/ubiquity/gtk/stepWelcome.ui')
        builder.connect_signals(self)
        self.page = builder.get_object('stepWelcome')

        self.intro_image = builder.get_object('intro_image')
        self.intro_label = builder.get_object('intro_label')
        self.btn_usb = builder.get_object('btn_usb')
        self.btn_usb.connect('clicked', self.genUSB)
        
        # don't show USB button
        self.vbox1 = builder.get_object('vbox1')
        self.vbox1.hide()

        self.intro_image=self.intro_image.set_from_file("/usr/share/ubiquity/pixmaps/max/logo.png")

        fname=self.intro_file
        if os.path.isfile("/cdrom/casper/nanomax"):
            fname=self.intro_file_nano
        
        self.intro_label.set_markup( get_intro(fname).rstrip('\n') )
        syslog.syslog("DEBUG: PageGTK intro")

        #sh = gtk.gdk.get_default_root_window().get_screen().get_height()
        self.plugin_widgets = self.page

    def genUSB(self, *args):
        self.installing=True
        p=Popen("sudo nohup /usr/bin/usb-creator-gtk 2>&1 > /dev/null &", shell=True, bufsize=0,
                                                 stdout=PIPE,
                                                 stderr=STDOUT, close_fds=True)
        #while self.installing:
        #    if p.poll() != None: self.installing=False
        #    line=p.stdout.readline()
        #    syslog.syslog("USBCREATOR: %s"%line.strip())
        # quit when done
        self.quit_installer()

    def quit_installer(self):
        """quit installer cleanly."""
        try:
            os.popen("sudo rm -f /tmp/max_install_type")
            os.popen("sudo rm -f /tmp/max_desktop_type")
        except:
            pass
        from gi.repository import Gtk
        Gtk.main_quit()
        sys.exit(0)


class PageKde(PageBase):
    pass

class PageDebconf(PageBase):
    plugin_title = 'ubiquity/text/install_heading_label'

    def __init__(self, controller, *args, **kwargs):
        self.controller = controller

class PageNoninteractive2(PageBase):
    pass

class Page(plugin.Plugin):
    def ok_handler(self):
        syslog.syslog("DEBUG: PageGTK Page ok_handler")
        plugin.Plugin.ok_handler(self)

