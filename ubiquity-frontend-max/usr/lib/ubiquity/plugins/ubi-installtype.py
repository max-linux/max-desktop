# -*- coding: utf-8; Mode: Python; indent-tabs-mode: nil; tab-width: 4 -*-
#
# «usersetup» - User creation plugin.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Canonical Ltd.
#
# Authors:
#
# - Colin Watson <cjwatson@ubuntu.com>
# - Evan Dandrea <evand@ubuntu.com>
# - Roman Shtylman <shtylman@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

import os
import sys
from subprocess import Popen, PIPE, STDOUT

from ubiquity import validation
from ubiquity import misc
from ubiquity import plugin
import debconf
import syslog

NAME = 'installtype'
# debug
#AFTER = None
#WEIGHT = 100
# normal
AFTER = 'console_setup'
WEIGHT = 10

def check_hostname(hostname):
    """Returns a list of reasons why the hostname is invalid."""
    errors = []
    for result in validation.check_hostname(misc.utf8(hostname)):
        if result == validation.HOSTNAME_LENGTH:
            errors.append('hostname_error_length')
        elif result == validation.HOSTNAME_BADCHAR:
            errors.append('hostname_error_badchar')
        elif result == validation.HOSTNAME_BADHYPHEN:
            errors.append('hostname_error_badhyphen')
        elif result == validation.HOSTNAME_BADDOTS:
            errors.append('hostname_error_baddots')
    return errors


class PageBase(plugin.PluginUI):
    def __init__(self):
        self.install_types={"escritorio":1, "profesor":2, "alumno":3, "infantil":4, "nanomax":5, "terminales":6}
        self.install_type="escritorio"
        self.install_type_file="/tmp/max_install_type"
        self.desktop_type="gnome"
        self.desktop_type_file="/tmp/max_desktop_type"

    def set_install_type(self, value):
        """Set the set_install_type."""
        raise NotImplementedError('set_install_type')

    def get_install_type(self):
        """get the set_install_type."""
        raise NotImplementedError('get_install_type')

    def on_install_type_radio_toggled(self, widget, *args):
        raise NotImplementedError('on_install_type_radio_toggled')

    def set_hostname(self, hostname):
        raise NotImplementedError('set_hostname')

    def get_hostname(self):
        raise NotImplementedError('get_hostname')

    def clear_errors(self):
        pass

    def info_loop(self, *args):
        """Verify user input."""
        pass

class PageGtk(PageBase):
    def __init__(self, controller, *args, **kwargs):
        PageBase.__init__(self, *args, **kwargs)
        self.controller = controller
        self.install_types=["escritorio", "profesor", "alumno", "infantil", "nanomax", "terminales"]
        self.desktop_types=["gnome", "xfce"]
        #self.username_changed_id = None
        #self.hostname_changed_id = None
        #self.username_edited = False
        #self.hostname_edited = False

        from gi.repository import Gtk
        builder = Gtk.Builder()
        self.controller.add_builder(builder)
        builder.add_from_file('/usr/share/ubiquity/gtk/stepInstallType.ui')
        builder.connect_signals(self)
        self.page = builder.get_object('stepInstallType')

        self.install_type_group = builder.get_object('install_type_group')
        self.install_type_escritorio = builder.get_object('install_type_escritorio')
        self.install_type_profesor = builder.get_object('install_type_profesor')
        self.install_type_alumno = builder.get_object('install_type_alumno')
        self.install_type_infantil = builder.get_object('install_type_infantil')
        self.install_type_nanomax = builder.get_object('install_type_nanomax')
        self.install_type_terminales = builder.get_object('install_type_terminales')

        self.hostname_widget = builder.get_object('hostname')
        self.hostname_error_widget = builder.get_object('hostname_error')

        self.sendinfo_widget = builder.get_object('sendinfo_check')
        
        self.install_vbox = builder.get_object('install_vbox')
        self.scrolledwin = builder.get_object('install_scrolledwindow')
        self.install_warn_nano = builder.get_object('install_warn_nano')
        self.install_warn_sti = builder.get_object('install_warn_sti')
        
        self.desktop_type_group = builder.get_object('desktop_type_group')
        self.desktop_type_gnome = builder.get_object('desktop_gnome')
        self.desktop_type_xfce = builder.get_object('desktop_xfce')
        #
        for radio in self.desktop_types:
            getattr(self, "desktop_type_%s"%radio).connect('toggled', self.on_desktop_type_radio_toggled, radio)
        self.set_desktop_type('gnome')

        #self.install_type_group.connect('toggled', self.on_install_type_radio_toggled)
        try:
            os.popen("sudo rm -f /tmp/max_install_type /tmp/sendinfo")
        except:
            pass
        # syslog.syslog("DEBUG: preseeding max.seed")
        # os.popen("sudo debconf-set-selections < /cdrom/preseed/max.seed")

        self.set_install_type('escritorio')
        for radio in self.install_types:
            getattr(self, "install_type_%s"%radio).connect('toggled', self.on_install_type_radio_toggled, radio)

        self.set_hostname('max95')
        self.hostname_widget.connect('changed', self.on_hostname_changed)
        self.hostname_error_widget.hide()

        self.plugin_widgets = self.page
        
        self.sti=False
        self.get_test_sti()

        self.sendinfo_widget.connect('toggled', self.on_sendinfo_toggled)
        # by default enabled
        os.popen("sudo touch /tmp/sendinfo")
        
        if os.path.isfile("/cdrom/casper/nanomax"):
            self.install_type_escritorio.set_sensitive(False)
            self.install_type_profesor.set_sensitive(False)
            self.install_type_alumno.set_sensitive(False)
            self.install_type_infantil.set_sensitive(False)
            self.install_type_nanomax.set_sensitive(False)
            self.install_type_terminales.set_sensitive(False)
            self.install_warn_nano.show()

        if not os.path.isfile("/cdrom/nanomax/casper/filesystem.squashfs"):
            #self.install_type_nanomax.set_sensitive(False)
            self.install_type_nanomax.hide()

    # Functions called by the Page.

    def set_desktop_type(self, value):
        if hasattr(self, "desktop_type_%s"%value):
            obj=getattr(self, "desktop_type_%s"%value)
            obj.set_active(True)
        self.desktop_type=value
        f=open(self.desktop_type_file, "w")
        f.write(value)
        f.close()
        syslog.syslog("DEBUG: set_desktop_type %s"%value)

    def set_install_type(self, value):
        if hasattr(self, "install_type_%s"%value):
            obj=getattr(self, "install_type_%s"%value)
            obj.set_active(True)
        self.install_type=value
        self.save_install_type(value)

    def save_install_type(self, itype):
        f=open(self.install_type_file, "w")
        f.write(itype)
        f.close()
        syslog.syslog("DEBUG: save_install_type %s"%itype)

    def get_install_type(self):
        return self.install_type

    def on_desktop_type_radio_toggled(self, widget, *args):
        if not widget.get_active():
            # do nothing
            return
        self.set_desktop_type(args[0])
        syslog.syslog("DEBUG: on_desktop_type_radio_toggled() TYPE=%s"%self.desktop_type)

    # MaX install type
    def on_install_type_radio_toggled(self, widget, *args):
        if not widget.get_active():
            # do nothing
            return
        self.set_install_type(args[0])
        syslog.syslog("DEBUG: on_install_type_radio_toggled() TYPE=%s"%self.install_type)

    def set_hostname(self, hostname):
        self.hostname=hostname
        self.hostname_widget.set_text(hostname)

    def get_hostname(self):
        return self.hostname_widget.get_text().strip()

    def on_sendinfo_toggled(self, widget):
        syslog.syslog("DEBUG: on_sendinfo_toggled() STATUS=%s"%(("OFF", "ON")[widget.get_active()]))
        if not widget.get_active():
            os.popen("sudo rm -f /tmp/sendinfo")
        else:
            os.popen("sudo touch /tmp/sendinfo")

    def get_test_sti(self):
        subp = Popen(['/usr/bin/test-sti'], stdout=PIPE)
        result = subp.communicate()[0].splitlines()
        self.sti=result[0].strip()
        syslog.syslog("DEBUG: get_test_sti() result=%s sti=%s"%(result, self.sti))
        if self.sti == b'YES':
            self.install_warn_sti.show()
            os.popen("sudo touch /tmp/max_sti")
            syslog.syslog("DEBUG: get_test_sti() YES")
        else:
            self.install_warn_sti.hide()
            os.popen("sudo rm -f /tmp/max_sti")
            syslog.syslog("DEBUG: get_test_sti() NO")

    def on_hostname_changed(self, widget):
        if widget.get_text() != '':
            self.info_loop(None)

    def info_loop(self, *args):
        """Verify user input."""
        complete=False

        txt = self.hostname_widget.get_text()
        errors = check_hostname(txt)
        if errors:
            # show a alert message
            m = '<small><span foreground="darkred"><b>El nombre de equipo es incorrecto, evite guiones bajos o caracteres distintos de letras y números</b></span></small>'
            self.hostname_error_widget.set_markup(m)
            self.hostname_error_widget.show()
        else:
            complete = True
            self.hostname_error_widget.hide()
        self.controller.allow_go_forward(complete)


class PageKde(PageBase):
    pass

class PageDebconf(PageBase):
    plugin_title = 'ubiquity/text/install_heading_label'

    def __init__(self, controller, *args, **kwargs):
        self.controller = controller

class PageNoninteractive2(PageBase):
    pass


class Page(plugin.Plugin):

    def prepare(self, unfiltered=False):
        self.ui.info_loop(None)

    def ok_handler(self):
        install_type=self.ui.get_install_type()
        self.preseed('ubiquity/max_install_type', install_type)

        hostname=self.ui.get_hostname()
        self.preseed('netcfg/get_hostname', hostname)
        syslog.syslog("DEBUG: set hostname '%s'"%hostname)

        if install_type == 'nanomax':
            self.installing=True
            p=Popen("sudo /usr/sbin/nanomax-installer", shell=True, bufsize=0, stdout=PIPE, stderr=STDOUT, close_fds=True)
            while self.installing:
                if p.poll() != None: self.installing=False
                line=p.stdout.readline()
                syslog.syslog("DEBUG: %s"%line.strip())
            # quit when done
            self.quit_installer()
        
        plugin.Plugin.ok_handler(self)

    def quit_installer(self):
        """quit installer cleanly."""
        try:
            os.popen("sudo rm -f /tmp/max_install_type")
        except:
            pass
        from gi.repository import Gtk
        Gtk.main_quit()
        sys.exit(0)

#class Install(InstallPlugin):
#    def prepare(self, unfiltered=False):
#        if 'UBIQUITY_OEM_USER_CONFIG' in os.environ:
#            environ = {'OVERRIDE_SYSTEM_USER': '1'}
#            return (['/usr/lib/ubiquity/user-setup/user-setup-apply'], [], environ)
#        else:
#            return (['/usr/lib/ubiquity/user-setup/user-setup-apply', '/target'],
#                    [])

#    def error(self, priority, question):
#        self.ui.error_dialog(self.description(question),
#                             self.extended_description(question))
#        return InstallPlugin.error(self, priority, question)

#    def install(self, target, progress, *args, **kwargs):
#        progress.info('ubiquity/install/user')
#        return InstallPlugin.install(self, target, progress, *args, **kwargs)

