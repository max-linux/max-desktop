#!/usr/bin/env python
# -*- coding: UTF-8 -*-
##########################################################################
# MaXMirrorUtils writen by MarioDebian <mariodebian@gmail.com>
#
#    MaXMirrorUtils version __VERSION__
#
# Copyright (c) 2009 Mario Izquierdo <mariodebian@gmail.com>
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

import sys
import os
import pwd

# for get_ip_address
import socket
import fcntl
import struct
import netifaces
#####################


import pygtk
pygtk.require('2.0')
from gtk import *
import gtk.glade

import time
import getopt
from gettext import gettext as _
from gettext import bindtextdomain, textdomain
from locale import setlocale, LC_ALL

from subprocess import Popen, PIPE, STDOUT
from threading import Thread

import gobject

#import threading
gtk.gdk.threads_init()
gobject.threads_init()





debug=False
PACKAGE="max-mirror-utils"

# if exec from svn or sources dir
if os.path.isfile('/home/mario/MaX/svn/max/trunk/max-mirror-utils/debian/rules'):
    LOCALE_DIR = "./po/"
    GLADE_DIR = "usr/share/max-mirror-utils/"
    IMG_DIR = "./usr/share/pixmaps/"
    CONF_PATH="./approx.conf"
    print "exec in sources dir"
else:
    GLADE_DIR = "/usr/share/max-mirror-utils/"
    IMG_DIR = "/usr/share/pixmaps/"
    LOCALE_DIR = "/usr/share/locale/"
    CONF_PATH="/etc/approx/approx.conf"





def print_debug(txt):
    if debug:
        print >> sys.stderr, "%s::%s" %("max-mirror-utils", txt)
    return

def usage():
    print "max-mirror-utils help:"
    print ""
    print "   max-mirror-utils -d [--debug]  (write debug data to stdout)"
    print "   max-mirror-utils -h [--help]   (this help)"


try:
    opts, args = getopt.getopt(sys.argv[1:], ":hd", ["help", "debug"])
except getopt.error, msg:
    print msg
    print "for command line options use tcosconfig --help"
    sys.exit(2)

# process options
for o, a in opts:
    if o in ("-d", "--debug"):
        print "DEBUG ACTIVE"
        debug = True
    if o in ("-h", "--help"):
        usage()
        sys.exit()


# gettext support
setlocale( LC_ALL )
bindtextdomain( PACKAGE, LOCALE_DIR )
textdomain( PACKAGE )
        


################################################################################

CONF_PATH="/etc/approx/approx.conf"

DEFAULT_CONF="""
# Here are some examples of remote repository mappings.
# See http://www.debian.org/mirror/list for mirror sites.

#debian		http://ftp.debian.org/debian
#security	http://security.debian.org/debian-security
#volatile	http://volatile.debian.org/debian-volatile

# The following are the default parameter values, so there is
# no need to uncomment them unless you want a different value.
# See approx.conf(5) for details.

#$interface	any
#$port		9999
#$max_wait	10
#$max_rate	unlimited
#$user		approx
#$group		approx
#$syslog	daemon
#$pdiffs	true
#$verbose	false
#$debug		false
"""

MIRROR_SUFFIX="archive.ubuntu.com/ubuntu"
MAX_MIRROR="http://max.educa.madrid.org/"
UBUNTU_SECTIONS="main universe multiverse restricted"
MAX_SECTIONS="main"
PORT="9999"
HIDDEN_INTERFACES=['lo', 'pan0', 'sit0']


UBUNTU_MIRRORS=[
                [ _("Main (recomended)"), '' ],
                [ _("Spain"), 'es.' ],
                [ _("France"), 'fr.' ],
                [ _("United Kingdom"), 'uk.' ],
                [ _("Germany"), 'de.' ],
                [ _("Finland"), 'fi.' ],
                ]

DISTROS=[
          [ _("MaX 4.0"), ['hardy', 'hardy-updates', 'hardy-security'], ['max40'] ],
          [ _("MaX 5.0"), ['jaunty', 'jaunty-updates', 'jaunty-security'], ['max50'] ],
          [ _("MaX 6.0"), ['lucid', 'lucid-updates', 'lucid-security'], ['max60'] ],
          [ _("MaX 7.5"), ['precise', 'precise-updates', 'precise-security'], ['max75'] ],
          [ _("MaX 8.0"), ['trusty', 'trusty-updates', 'trusty-security'], ['max80'] ],
          [ _("MaX 4.0, 5.0, 6.0, 7.5 y 8.0"), ['hardy', 'hardy-updates', 'hardy-security', 
                                             'jaunty', 'jaunty-updates', 'jaunty-security', 
                                             'lucid', 'lucid-updates', 'lucid-security',
                                             'precise', 'precise-updates', 'precise-security'
                                             'trusty', 'trusty-updates', 'trusty-security'], 
                                            ['max40', 'max50', 'max60', 'max75', 'max80'] ],
        ]


"""
debian­ubu   http://ftp.crihan.fr/ubuntu/
debian­secu  http://security.ubuntu.com/ubuntu
debian­max4 http://max.educa.madrid.org/max40
"""

################################################################################

class MaXMirrorUtils:
    def __init__(self):
        print_debug("__init__()")
        self.data=None
        # vars
        self.v={}
        self.begin_usernumber=1
        gtk.glade.bindtextdomain(PACKAGE, LOCALE_DIR)
        gtk.glade.textdomain(PACKAGE)

        # Widgets
        self.ui = gtk.glade.XML(GLADE_DIR + 'max-mirror-utils.glade')
        self.mainwindow = self.ui.get_widget('mainwindow')
        self.mainwindow.set_icon_from_file(IMG_DIR +'max-mirror-utils.png')
        
        # close windows signals
        self.mainwindow.connect('destroy', self.quitapp )
        self.mainwindow.connect('delete_event', self.quitapp)
        
        self.button_quit=self.ui.get_widget("btn_quit")
        self.button_quit.connect('clicked', self.quitapp)
        
        
        # widgets
        self.w={}
        for widget in ['combo_options', 'combo_distro', 'btn_configure', 'table',
                       'btn_generate', 'lbl_message', 'expander', 'textview']:
            self.w[widget]=self.ui.get_widget(widget)
            print_debug("widget name=%s obj=%s"%(widget, self.w[widget]))
        
        self.w['lbl_message'].set_text("")
        self.w['lbl_message'].show()
        self.w['combo_options'].connect('changed', self.combo_options_change )
        self.w['btn_configure'].connect('clicked', self.on_btn_configure )
        self.w['btn_configure'].set_sensitive(True)
        self.w['btn_generate'].connect('clicked', self.on_btn_generate )
        self.w['btn_generate'].set_sensitive(False)
        
        self.w['expander'].hide()
        
        self.populate_select(self.w['combo_options'], UBUNTU_MIRRORS)
        self.set_active_in_select(self.w['combo_options'], UBUNTU_MIRRORS[0][0])
        
        self.populate_select(self.w['combo_distro'], DISTROS)
        self.set_active_in_select(self.w['combo_distro'], DISTROS[2][0])
        
        
        
    def combo_options_change(self, widget):
        print_debug("combo_options_change() '%s'"%self.read_select_value(widget))
        self.w['btn_configure'].set_sensitive(True)

    def on_btn_configure(self, *args):
        th=Thread(target=self.configure_approx)
        th.start()

    def on_btn_generate(self, *args):
        th=Thread(target=self.generate_sources_list)
        th.start()

    def configure_approx(self, *args):
        print_debug("configure_approx() ")
        gtk.gdk.threads_enter()
        self.w['btn_generate'].set_sensitive(True)
        self.w['btn_configure'].set_sensitive(False)
        gtk.gdk.threads_leave()
        mode=self.read_select_value(self.w['combo_options'])
        mirror=None
        for item in UBUNTU_MIRRORS:
            print_debug("SELMIRROR item[0]=%s mode=%s"%(item[0], mode) )
            if item[0] == mode:
                mirror="http://%s%s"%(item[1], MIRROR_SUFFIX)
                #print_debug("configure_aprox() setting mode with '%s' mirror=%s"%(mode, mirror))
        
        
        seldistro=self.read_select_value(self.w['combo_distro'])
        distros=None
        for item in DISTROS:
            print_debug("SELDISTRO item[0]=%s mode=%s"%(item[0], seldistro) )
            if item[0] == seldistro:
                distros=item
                print_debug("configure_aprox() setting mode with '%s' distros=%s"%(seldistro, distros))

        if not mirror:
            return
        if not distros:
            return

        self.data=[mirror, distros]

        # open config and write new
        f=open(CONF_PATH, 'w')
        f.write(DEFAULT_CONF)
        f.write("\n\n")
        f.write("ubuntu %s \n" %(mirror))
        for line in distros[2]:
            f.write("%s %s%s\n"%(line, MAX_MIRROR, line))
        f.write("\n\n")
        f.close()

        # reload approx
        self.exe_cmd("/etc/init.d/approx stop")
        result=self.exe_cmd("/etc/init.d/approx start")
        fail=False
        for line in result:
            if "fail" in line: fail=True
            if "OK" in line: fail=False
            if "done" in line: fail=False

        # update lbl_message        
        gtk.gdk.threads_enter()
        if not fail:
            self.w['lbl_message'].set_markup( _("<b>Proxy configured</b>") )
        else:
            self.w['lbl_message'].set_markup( _("<b>Error configuring proxy.</b>") )
        gtk.gdk.threads_leave()


    def generate_sources_list(self, *args):
        print_debug("generate_sources_list() ")
        
        # use self.data
        ips=[]
        for iface in self.getNetInterfaces():
            if iface[1]:
                ips.append(iface[1])

        print_debug("generate_sources_list() ips=%s"%ips)
        if len(ips) != 1:
            # show a message
            gtk.gdk.threads_enter()
            self.error_msg( _("Detected %s network interfaces, need only one.") %len(ips) )
            gtk.gdk.threads_leave()

        mirror=self.data[0]
        distros=self.data[1]
        z=self.w['textview']

        # generate sources.list text
        gtk.gdk.threads_enter()
        self.writeIntoTextView(z, _("# sources.list for %s") %("Ubuntu") )
        for uversion in distros[1]:
            self.writeIntoTextView(z, "deb http://%s:%s/ubuntu %s %s"%(ips[0], PORT, uversion, UBUNTU_SECTIONS) )

        self.writeIntoTextView(z, "\n" )
        self.writeIntoTextView(z, _("# sources.list for %s") %(_(" and ").join(distros[2])) )
        for mversion in distros[2]:
            self.writeIntoTextView(z, "deb http://%s:%s/%s max %s"%(ips[0], PORT, mversion,MAX_SECTIONS) )
        gtk.gdk.threads_leave()

        gtk.gdk.threads_enter()
        self.w['table'].hide()
        self.w['expander'].show()
        gtk.gdk.threads_leave()

    def getNetInterfaces(self):
        interfaces=[]
        for dev in netifaces.interfaces():
            if not dev in HIDDEN_INTERFACES and not dev.startswith("vbox") and not dev.startswith("vmnet") and not dev.startswith("wmaster"):
                ip=self.get_ip_address(dev)
                print_debug("getNetInterfaces() iface=%s data=%s"%(dev,ip))
                interfaces.append( [dev, ip] )
        return interfaces

    def get_ip_address(self, ifname):
        print_debug("get_ip_address() ifname=%s" %(ifname) )
        if not ifname in netifaces.interfaces():
            return None
        ip=netifaces.ifaddresses(ifname)
        if ip.has_key(netifaces.AF_INET):
            return ip[netifaces.AF_INET][0]['addr']
        return None


################### combo stuff ##############################

    def populate_select(self, widget, values):
        valuelist = gtk.ListStore(str)
        for value in values:
            valuelist.append([value[0]])
        widget.set_model(valuelist)
        #widget.set_text_column(0)
        if widget.get_text_column() != 0:
            widget.set_text_column(0)
        model=widget.get_model()
        return

    def set_active_in_select(self, widget, default):
        model=widget.get_model()
        for i in range(len(model)):
            if model[i][0] == default:
                print_debug ("set_active_in_select() default is '%s', index %d" %( model[i][0] , i ) )
                widget.set_active(i)
        return

    def read_select_value(self, widget):
        selected=-1
        try:
            selected=widget.get_active()
        except:
            print_debug ( "read_select() ERROR reading " )
        model=widget.get_model()
        value=model[selected][0]
        print_debug ( "read_select() reading %s" %(value) )
        return value

    def writeIntoTextView(self, widget, txt):
        buffer = widget.get_buffer()
        iter = buffer.get_end_iter()
        mark = buffer.get_insert()
        txt=str(txt)
        buffer.insert(iter, '\n' + txt)
        # scroll window
        widget.scroll_to_mark(mark, 0.2)
        return




################################################################################


    def exe_cmd(self, cmd, verbose=1):
        print_debug("exe_cmd() cmd=%s" %cmd)
        
        self.p = Popen(cmd, shell=True, bufsize=0, stdout=PIPE, stderr=STDOUT, close_fds=True)
        output=[]
        stdout = self.p.stdout
        for line in stdout.readlines():
            if line != '\n':
                line=line.replace('\n', '')
                output.append(line)
        if len(output) == 1:
            return output[0]
        elif len(output) > 1:
            if verbose==1:
                print_debug ( "exe_cmd(%s) %s" %(cmd, output) )
            return output
        else:
            if verbose == 1:
                print_debug ( "exe_cmd(%s)=None" %(cmd) )
            return []

    def error_msg(self,txt):
        d = gtk.MessageDialog(None,
                          gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                          gtk.MESSAGE_WARNING, gtk.BUTTONS_OK,
                          txt)
        d.run()
        d.destroy()


    def quitapp(self,*args):
        print_debug ( "Exiting" )
        self.mainloop.quit()

    def run (self):
        self.mainloop = gobject.MainLoop()
        try:
            self.mainloop.run()
        except KeyboardInterrupt: # Press Ctrl+C
            self.quitapp()
   


if __name__ == '__main__':
    app = MaXMirrorUtils ()
    # Run app
    app.run ()
