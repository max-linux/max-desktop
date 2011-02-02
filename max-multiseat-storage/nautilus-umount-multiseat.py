#!/usr/bin/env python
# -*- coding: UTF-8 -*-
##########################################################################
#
# Multiseat Nautilus umount USB storage devices
# Copyright 2011, Mario Izquierdo, mariodebian at gmail
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

import gtk
import nautilus
import os
import urllib
from subprocess import Popen, PIPE, STDOUT

def normalize(path):
    path = path.replace(" ", "\\ ")
    path = path.replace("(", "\\(")
    path = path.replace(")", "\\)")
    path = path.replace("'", "\\'")
    path = path.replace("&", "\\&")
    return path

def log(txt):
    return
    #f = open('/tmp/nautilus-umount-multiseat.log', 'a')
    #f.write(str(txt) + "\n")
    #f.close()

def error_msg(txt):
    d = gtk.MessageDialog(None,
                  gtk.DIALOG_MODAL |
                  gtk.DIALOG_DESTROY_WITH_PARENT,
                  gtk.MESSAGE_WARNING,
                  gtk.BUTTONS_OK,
                  txt)
    d.run()
    d.destroy()
    return

def info_msg(txt):
    d = gtk.MessageDialog(None,
                  gtk.DIALOG_MODAL |
                  gtk.DIALOG_DESTROY_WITH_PARENT,
                  gtk.MESSAGE_INFO,
                  gtk.BUTTONS_OK,
                  txt)
    d.run()
    d.destroy()
    return


class MultiseatUmountExtension(nautilus.MenuProvider):
    def __init__(self):
        self.file_names = []

    def menu_activate_umount_multiseat(self, menu, data):
        log("umount selected")
        log(data)
        umounted=False
        # read /proc/mounts for device
        found=False
        mountline=''
        f=open('/proc/mounts', 'r')
        for line in f.readlines():
            if line.startswith(data['Dev']):
                mountline=line
                found=True
                log(line)
        f.close()
        if not found:
            log("dispositivo no montado")
            return error_msg("El dispositivo no está montado")
        # umount.multiseat a C app with bit SUID
        cmd="/sbin/umount.multiseat %s"%data['Dev']
        p=Popen(cmd, shell=True, bufsize=0, 
                stdout=PIPE, stderr=STDOUT, close_fds=True)
        for _line in p.stdout.readlines():
            line=_line.strip()
            log("umount.multiseat called, line=%s"%line)
            if line == "no-mounted":
                return error_msg("El dispositivo no está montado.")
            elif line == "invalid-user" or \
                 line.strip() == "not-yours" or \
                 line.strip() == "no-uid":
                return error_msg("El usuario no tiene permiso para desmontar ese dispositivo.")
            elif line == "no-serial":
                return error_msg("No se pudo leer el número de serie del dispositivo.")
            elif line == "ok":
                umounted=True
            else:
                return error_msg("Error desconocido:\n\n%s"%line)
        # show a message if ok (umount OK)
        log("desmontaje correcto")
        if umounted:
            return info_msg("Dispositivo desmontado y listo para extraer.")

    def get_file_items(self, window, files):
        """Called when the user selects a file in Nautilus."""

        # This script will only accept one file
        if len(files) != 1:
            return

        f = files[0]
        log(f.get_uri())
        filepath = normalize(urllib.unquote(f.get_uri()[7:]))
        name = os.path.basename(filepath)
        
        log(filepath)
        log(name)
        if not ".desktop" in name:
            log("no .desktop file")
            return
        
        # read desktop file and search 
        # X-multiseat-desktop=true
        f = open(filepath, 'r')
        data={}
        for line in f.readlines():
            if "=" in line:
                data[line.split('=')[0]]=line.strip().split('=')[1]
        f.close()
        log(data)
        
        if not data.has_key('X-multiseat-desktop'):
            log("no key X-multiseat-desktop")
            return
        
        umount_item = nautilus.MenuItem("NautilusPython::umount_multiseat_item",
                                        "Desmontar dispositivo extraíble multiseat",
                                        "Desmontar dispositivo extraíble multiseat",
                                        "nautilus-mount-image")
        
        if os.path.isfile('/usr/share/icons/maxtoon/16x16/devices/usbpendrive_unmount.png'):
            umount_item.set_property('icon', '/usr/share/icons/maxtoon/16x16/devices/usbpendrive_unmount.png')
        elif os.path.isfile('/usr/share/icons/gnome/16x16/devices/usbpendrive_unmount.png'):
            umount_item.set_property('icon', '/usr/share/icons/gnome/16x16/devices/usbpendrive_unmount.png')
        else:
            umount_item.set_property('icon', 'gtk-dialog-warning')
        
        umount_item.connect("activate", self.menu_activate_umount_multiseat, data)
        log("found X-multiseat-desktop in %s"%filepath)
        return umount_item,


