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
import syslog
from ubiquity.plugin import *

NAME = 'welcome'
AFTER = None
WEIGHT = 100


def get_intro(fname):
    intro_file = open(fname)
    text = intro_file.read()
    intro_file.close()
    return text


class PageBase(PluginUI):
    def __init__(self):
        self.intro_file="/usr/share/ubiquity/intro.txt"
        self.intro_file_nano="/usr/share/ubiquity/intro-nanomax.txt"


class PageGtk(PageBase):
    def __init__(self, controller, *args, **kwargs):
        PageBase.__init__(self, *args, **kwargs)
        self.controller = controller
        

        import gtk
        builder = gtk.Builder()
        self.controller.add_builder(builder)
        builder.add_from_file('/usr/share/ubiquity/gtk/stepWelcome.ui')
        builder.connect_signals(self)
        self.page = builder.get_object('stepWelcome')

        self.intro_image = builder.get_object('intro_image')
        self.intro_label = builder.get_object('intro_label')
        
        self.intro_image=self.intro_image.set_from_file("/usr/share/ubiquity/pixmaps/max/logo.png")

        fname=self.intro_file
        if os.path.isfile("/cdrom/casper/nanomax"):
            fname=self.intro_file_nano
        
        self.intro_label.set_markup( get_intro(fname).rstrip('\n') )
        syslog.syslog("DEBUG: PageGTK intro")

        sh = gtk.gdk.get_default_root_window().get_screen().get_height()
        self.plugin_widgets = self.page




class PageKde(PageBase):
    pass

class PageDebconf(PageBase):
    plugin_title = 'ubiquity/text/install_heading_label'

    def __init__(self, controller, *args, **kwargs):
        self.controller = controller

class PageNoninteractive2(PageBase):
    pass

class Page(Plugin):
    def ok_handler(self):
        syslog.syslog("DEBUG: PageGTK Page ok_handler")
        Plugin.ok_handler(self)

