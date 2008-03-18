# -*- coding: UTF-8 -*-

# Copyright (C) 2006 Canonical Ltd.
# Written by Colin Watson <cjwatson@ubuntu.com>.
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

from ubiquity.filteredcommand import FilteredCommand
import os
import syslog

class GrubInstaller(FilteredCommand):
    def prepare(self):
        try:
            self.detect_sti()
        except Exception, err:
            syslog.syslog("DEBUG GRUBINSTALLER Exception %s"%err)
        return (['/usr/share/grub-installer/grub-installer', '/target'],
                ['^grub-installer/bootdev$', 'ERROR'],
                {'OVERRIDE_UNSUPPORTED_OS': '1'})

    def detect_sti(self):
        syslog.syslog("DEBUG: GRUBINSTALLER detect_sti()")
        pin,pout,perr=os.popen3("/usr/bin/test-sti")
        pin.close()
        perr.close()
        response=pout.readline().strip()
        pout.close()
        syslog.syslog("DEBUG: GRUBINSTALLER detect_sti() response=%s"%response)
        if response == "YES":
            syslog.syslog("DEBUG: GRUBINSTALLER preseeding bootdev to (hd0,0)")
            self.preseed('grub-installer/bootdev', '(hd0,0)')
        else:
            syslog.syslog("DEBUG: GRUBINSTALLER no STI card found")
        

    def error(self, priority, question):
        self.frontend.error_dialog(self.description(question),
                                   self.extended_description(question))
        return FilteredCommand.error(self, priority, question)

    def run(self, priority, question):
        if question == 'grub-installer/bootdev':
            # Force to (hd0) in the case of an unsupported OS.
            if self.db.get(question) == '':
                self.preseed(question, '(hd0)')

        return FilteredCommand.run(self, priority, question)
