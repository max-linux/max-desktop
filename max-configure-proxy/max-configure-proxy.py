#!/usr/bin/python
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
import gtk
import getopt
from threading import Thread
import gobject
gtk.gdk.threads_init()
gobject.threads_init()

debug=False
PACKAGE="max-configure-proxy"
DEFAULT_EXCP='localhost,127.0.0.1,max-server,.educa.madrid.org,.educa2.madrid.org'
for i in range(255):
    DEFAULT_EXCP="%s,192.168.1.%d" % (DEFAULT_EXCP, i)

UI_DIR = "/usr/share/max-configure-proxy/"
#UI_DIR = "./"
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
        
        #self.ui = gtk.glade.XML(GLADE_DIR + 'max-configure-proxy.ui')
        self.ui = gtk.Builder()
        print_debug("load ui %s"%(UI_DIR + 'max-configure-proxy.ui'))
        self.ui.add_from_file(UI_DIR + 'max-configure-proxy.ui')
        self.mainwindow = self.ui.get_object('mainwindow')
        
        # close windows signals
        self.mainwindow.connect('destroy', self.quitapp )
        self.mainwindow.connect('delete_event', self.quitapp)
        
        self.button_quit=self.ui.get_object("btn_quit")
        self.button_quit.connect('clicked', self.quitapp)
        
        
        # widgets
        self.w={}
        for widget in ['txt_ip', 'txt_port', 'txt_user', 'txt_password', 'txt_exceptions', 'btn_activate', 'btn_cancel', 'lbl_message']:
            self.w[widget]=self.ui.get_object(widget)
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
            self.w['txt_user'].set_text(data['user'])
            self.w['txt_password'].set_text(data['password'])
            self.w['txt_exceptions'].set_text(data['exceptions'])
            print_debug(data)
        else:
            self.w['btn_activate'].set_sensitive(True)
            self.w['btn_cancel'].set_sensitive(False)
            self.w['lbl_message'].set_markup( ("<b>No hay proxy configurado</b>") )
            self.w['txt_exceptions'].set_text(DEFAULT_EXCP)

##########################################################

    def on_btn_activate(self, *args):
        th=Thread(target=self.configure_proxy)
        th.start()

    def configure_proxy(self, *args):
        print_debug("configure_proxy() ")
        user=self.w['txt_user'].get_text().strip()
        password=self.w['txt_password'].get_text().strip()
        exceptions=self.w['txt_exceptions'].get_text().strip()
        excp_list="'" + "', '".join(exceptions.split(',')) + "'"
        ip=self.w['txt_ip'].get_text().strip()
        try:
            port=int(self.w['txt_port'].get_text().strip())
        except:
            port=''
        proxy_str="%s:%s"%(ip,port)
        use_auth='false'
        if user != '' and password != '':
            proxy_str="%s:%s@%s:%s"%(user,password,ip,port)
            use_auth='true'
        print_debug("export http_proxy=http://%s"%(proxy_str))
        if ip == '' or port == '':
            gtk.gdk.threads_enter()
            self.error_msg("La IP o el puerto no son correctos")
            gtk.gdk.threads_leave()
            return
        
        # escribir archivo
        f=open(PROFILE_CONF, 'w')
        f.write("#ip=%s\n" % ip)
        f.write("#port=%s\n" % port)
        f.write("#user=%s\n" % user)
        f.write("#password=%s\n" % password)
        f.write("#exceptions=%s\n" % exceptions)
        f.write("export all_proxy='http://%s'\n" % (proxy_str))
        f.write("export ftp_proxy='http://%s'\n" % (proxy_str))
        f.write("export http_proxy='http://%s'\n" % (proxy_str))
        f.write("export https_proxy='http://%s'\n" % (proxy_str))
        f.write("export socks_proxy='http://%s'\n" % (proxy_str))
        f.write("export no_proxy='%s'\n" % exceptions)
        f.write("\n")
        f.close()
        
        # generar dconf
        for profile in ['user', 'alumno']:
            need_dconf_edit=True
            if os.path.isfile('/etc/dconf/profile/'+profile):
                f=open('/etc/dconf/profile/'+profile, 'r')
                for line in f.readlines():
                    if "system-db:local" in line:
                        need_dconf_edit=False
                f.close()
            else:
                continue
            if need_dconf_edit:
                print_debug("EDIT '/etc/dconf/profile/%s' to add system-db:local"%(profile))
                f=open('/etc/dconf/profile/'+profile, 'a')
                f.write("system-db:local\n")
                f.close()
        
        # generar local.d/00_proxy
        if not os.path.isdir('/etc/dconf/db/local.d'):
            os.mkdir('/etc/dconf/db/local.d', 0755)
        
        f=open('/etc/dconf/db/local.d/00_proxy', 'w')
        f.write("""
[system/proxy/ftp]
host='%s'
port=%s

[system/proxy/http]
host='%s'
port=%s
enabled=true
authentication-user='%s'
authentication-password='%s'
use-authentication=%s

[system/proxy/https]
host='%s'
port=%s

[system/proxy]
ignore-hosts=[%s]
mode='manual'

[system/proxy/socks]
host='%s'
port=%s

""" %(ip, port, ip, port, user, password, use_auth, ip, port, excp_list, ip, port) )
        f.close()
        print_debug("CREATED '/etc/dconf/db/local.d/00_proxy'")
        
        # update dconf
        os.system("dconf update")
        
        
        # configurar apt-get
        f=open('/etc/apt/apt.conf.d/88max-proxy', 'w')
        f.write("Acquire::http::Proxy \"http://%s\";\n" %(proxy_str))
        f.close()
        print_debug("CREATED '/etc/apt/apt.conf.d/88max-proxy'")
        
        gtk.gdk.threads_enter()
        self.w['lbl_message'].set_markup( ("<b>Proxy activado.</b>") )
        self.w['btn_activate'].set_sensitive(False)
        self.w['btn_cancel'].set_sensitive(True)
        gtk.gdk.threads_leave()

##########################################################

    def on_btn_cancel(self, *args):
        th=Thread(target=self.desconfigure_proxy)
        th.start()

    def desconfigure_proxy(self, *args):
        print_debug("desconfigure_proxy() ")
        # borrar archivos
        for f in [PROFILE_CONF,
                  '/etc/dconf/db/local.d/00_proxy',
                  '/etc/apt/apt.conf.d/88max-proxy']:
            if os.path.isfile(f):
                print_debug("DELETE '%s'" %(f))
                os.unlink(f)
        
        
        # limpiar dconf
        for profile in ['user', 'alumno']:
            need_dconf_edit=False
            if os.path.isfile('/etc/dconf/profile/'+profile):
                f=open('/etc/dconf/profile/'+profile, 'r')
                lines=f.readlines()
                for line in lines:
                    if "system-db:local" in line:
                        need_dconf_edit=True
                f.close()
            if need_dconf_edit:
                print_debug("EDIT '/etc/dconf/profile/%s' to remove system-db:local"%(profile))
                f=open('/etc/dconf/profile/'+profile, 'w')
                for line in lines:
                    if "system-db:local" not in line:
                        f.write(line)
                f.close()
        # update dconf
        os.system("dconf update")
        
        # limpiar campos
        gtk.gdk.threads_enter()
        self.w['txt_ip'].set_text('')
        self.w['txt_port'].set_text('')
        self.w['txt_user'].set_text('')
        self.w['txt_password'].set_text('')
        self.w['txt_exceptions'].set_text(DEFAULT_EXCP)
        self.w['lbl_message'].set_markup( ("<b>Proxy desactivado.</b>") )
        self.w['btn_activate'].set_sensitive(True)
        self.w['btn_cancel'].set_sensitive(False)
        gtk.gdk.threads_leave()

    def read_proxy(self):
        data={'ip':'', 'port':'',
              'user':'', 'password':'',
              'exceptions':DEFAULT_EXCP}
        if not os.path.isfile(PROFILE_CONF):
            return data
        
        f=open(PROFILE_CONF, 'r')
        for line in f.readlines():
            if "#ip" in line:
                data['ip']=line.split('=')[1].strip()
            if "#port" in line:
                data['port']=line.split('=')[1].strip()
            if "#user" in line:
                data['user']=line.split('=')[1].strip()
            if "#password" in line:
                data['password']=line.split('=')[1].strip()
            if "#exceptions" in line:
                data['exceptions']=line.split('=')[1].strip()
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
