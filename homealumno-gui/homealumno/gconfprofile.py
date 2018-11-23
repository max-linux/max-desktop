# -*- coding: UTF-8 -*-
##########################################################################
# HomeAlumno-GUI writen by MarioDebian <mariodebian@gmail.com>
#
#    HomeAlumno-GUI version __VERSION__
#
# Copyright (c) 2010 Mario Izquierdo <mariodebian@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.
###########################################################################

import os
import sys
import pwd
import subprocess

sys.path.append('./')
import homealumno

def print_debug(txt):
    if homealumno.debug:
        print >> sys.stderr, "%s::%s" % ("homealumno-gui::gconfprofile", txt)

class GconfProfile(object):
    def __init__(self, uid, gid, direct=True):
        self.uid=int(uid)
        self.gid=int(gid)
        self.direct=direct
        self.dconf=True
        self.home=str(pwd.getpwuid(self.uid).pw_dir)

    def drop_all_privileges(self):
        # gconf needs both the UID and effective UID set.
        if 'SUDO_GID' in os.environ:
            # gid = int(os.environ['SUDO_GID'])
            print_debug("drop_all_privileges GID=%s" % self.gid)
            os.setregid(self.gid, self.gid)
        if 'SUDO_UID' in os.environ:
            # uid = int(os.environ['SUDO_UID'])
            print_debug("drop_all_privileges UID=%s HOME=%s" % (self.uid, self.home))
            os.setreuid(self.uid, self.uid)
            os.environ['HOME'] = self.home

    def _run_cmd(self, cmd, background=True):
        """Run command into shell"""
        if background:
            proc = subprocess.Popen(
                cmd, shell=False,
                preexec_fn=self.drop_all_privileges
            )
        else:
            proc = subprocess.Popen(
                cmd, shell=False,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                preexec_fn=self.drop_all_privileges
            )
            stdout,stderr = proc.communicate()
            return stdout,stderr

    def __gconf(self, cmd):
        # use gsetting to write into  dconf
        value = cmd['value']
        if cmd['type'] == 'string':
            value = "'%s'" % cmd['value']
        #
        #
        schema = ".".join( cmd['key'].split('.')[0:-1] )
        key = cmd['key'].split('.')[-1]
        #
        #
        cmd=['dbus-launch', '--exit-with-session', 'gsettings', 'set', schema, key, value]

        print_debug("cmd=%s" % cmd)
        stdout, stderr = self._run_cmd(cmd, False)
        if stdout:
            print_debug("WARNING STDOUT=%s" % stdout.strip('\n'))
        if stderr:
            print_debug("WARNING STDERR=%s" % stderr.strip('\n'))

        return stdout.rstrip('\n')

    def do(self, data):
        """
        data should be a array of dicts
        data=[
                {'key':'/apps/nautilus/desktop/home_icon_visible',
                 'type':'bool',
                 'value':'true'}
               ]
        """
        for key in data:
            res=self.__gconf(key)
            if res != '':
                print_debug("GconfProfile::do() res=%s"%res)


if __name__ == '__main__':
    # test
    homealumno.debug=True
    app=GconfProfile(1000, 1000)
    data=[
            {'key':'/apps/nautilus/desktop/home_icon_visible',
             'type':'bool',
             'value':'true'},
             {'key':'/apps/nautilus/desktop/computer_icon_visible',
             'type':'bool',
             'value':'true'},
             {'key':'/apps/nautilus/desktop/network_icon_visible',
             'type':'bool',
             'value':'true'},
           ]
    app.do(data)
