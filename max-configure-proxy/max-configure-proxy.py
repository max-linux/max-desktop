#!/usr/bin/env python
# -*- coding: UTF-8 -*-
##########################################################################
# max-configure-proxy writen by MarioDebian <mariodebian@gmail.com>
#
#    max-configure-proxy
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

import sys
import os


import pygtk
pygtk.require('2.0')
import gtk.glade
import getopt

from threading import Thread

import gobject

#import threading
gtk.gdk.threads_init()
gobject.threads_init()





debug=False
PACKAGE="max-configure-proxy"


GLADE_DIR = "/usr/share/max-configure-proxy/"
PROFILE_CONF="/etc/profile.d/max-proxy.sh"

def print_debug(txt):
    if debug:
        print >> sys.stderr, "%s::%s" %("max-configure-proxy", txt)
    return

def usage():
    print "max-configure-proxy help:"
    print ""
    print "   max-configure-proxy -d [--debug]  (write debug data to stdout)"
    print "   max-configure-proxy -h [--help]   (this help)"


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

################################################################################

class MaxConfigureProxy(object):
    def __init__(self):
        print_debug("__init__()")
        
        # vars
        self.v={}
        self.begin_usernumber=1
        
        # Widgets
        self.ui = gtk.glade.XML(GLADE_DIR + 'max-configure-proxy.glade')
        self.mainwindow = self.ui.get_widget('mainwindow')
        
        # close windows signals
        self.mainwindow.connect('destroy', self.quitapp )
        self.mainwindow.connect('delete_event', self.quitapp)
        
        self.button_quit=self.ui.get_widget("btn_quit")
        self.button_quit.connect('clicked', self.quitapp)
        
        
        # widgets
        self.w={}
        for widget in ['txt_ip', 'txt_port', 'btn_activate', 'btn_cancel', 'lbl_message']:
            self.w[widget]=self.ui.get_widget(widget)
            #print_debug("widget name=%s obj=%s"%(widget, self.w[widget]))
        
        self.w['lbl_message'].set_text("")
        self.w['lbl_message'].show()
        
        self.w['btn_activate'].connect('clicked', self.on_btn_activate )
        self.w['btn_cancel'].connect('clicked', self.on_btn_cancel )
        
        if os.path.isfile(PROFILE_CONF):
            self.w['btn_activate'].set_sensitive(False)
            self.w['btn_cancel'].set_sensitive(True)
            self.w['lbl_message'].set_markup( ("<b>El proxy está activo</b>") )
            data=self.read_proxy()
            self.w['txt_ip'].set_text(data['ip'])
            self.w['txt_port'].set_text(data['port'])
            print_debug(data)
        else:
            self.w['btn_activate'].set_sensitive(True)
            self.w['btn_cancel'].set_sensitive(False)
            self.w['lbl_message'].set_markup( ("<b>No hay proxy configurado</b>") )

##########################################################

    def on_btn_activate(self, *args):
        th=Thread(target=self.configure_proxy)
        th.start()

    def configure_proxy(self, *args):
        print_debug("configure_proxy() ")
        ip=self.w['txt_ip'].get_text().strip()
        try:
            port=int(self.w['txt_port'].get_text().strip())
        except:
            port=''
        print_debug("export http_proxy=http://%s:%s"%(ip,port))
        if ip == '' or port == '':
            gtk.gdk.threads_enter()
            self.error_msg("La IP o el puerto no son correctos")
            gtk.gdk.threads_leave()
            return
        
        # escribir archivo
        f=open(PROFILE_CONF, 'w')
        f.write("#ip=%s\n"%ip)
        f.write("#port=%s\n"%port)
        f.write("export http_proxy='http://%s:%s'\n"%(ip,port))
        f.close()
        
        gtk.gdk.threads_enter()
        self.w['lbl_message'].set_markup( ("<b>Proxy guardado, es necesario reiniciar.</b>") )
        self.w['btn_activate'].set_sensitive(False)
        self.w['btn_cancel'].set_sensitive(True)
        gtk.gdk.threads_leave()

##########################################################

    def on_btn_cancel(self, *args):
        th=Thread(target=self.desconfigure_proxy)
        th.start()

    def desconfigure_proxy(self, *args):
        print_debug("desconfigure_proxy() ")
        # borrar archivo
        os.unlink(PROFILE_CONF)
        
        # limpiar campos
        gtk.gdk.threads_enter()
        self.w['txt_ip'].set_text('')
        self.w['txt_port'].set_text('')
        self.w['lbl_message'].set_markup( ("<b>Proxy eliminado, es necesario reiniciar.</b>") )
        self.w['btn_activate'].set_sensitive(True)
        self.w['btn_cancel'].set_sensitive(False)
        gtk.gdk.threads_leave()

    def read_proxy(self):
        data={'ip':'', 'port':''}
        if not os.path.isfile(PROFILE_CONF):
            return data
        
        f=open(PROFILE_CONF, 'r')
        for line in f.readlines():
            if "#ip" in line:
                data['ip']=line.split('=')[1].strip()
            if "#port" in line:
                data['port']=line.split('=')[1].strip()
        f.close()
        return data

##########################################################

    def error_msg(self, txt):
        d = gtk.MessageDialog(None,
                      gtk.DIALOG_MODAL |
                      gtk.DIALOG_DESTROY_WITH_PARENT,
                      gtk.MESSAGE_WARNING,
                      gtk.BUTTONS_OK,
                      txt)
        d.run()
        d.destroy()

    def quitapp(self,*args):
        print_debug ( "Exiting" )
        self.mainloop.quit()

    def run (self):
        if os.getuid() != 0:
            self.error_msg("Debe ser administrador para ejecutar este programa")
            sys.exit(0)
        
        self.mainloop = gobject.MainLoop()
        try:
            self.mainloop.run()
        except KeyboardInterrupt: # Press Ctrl+C
            self.quitapp()
   


if __name__ == '__main__':
    app = MaxConfigureProxy()
    app.run()
