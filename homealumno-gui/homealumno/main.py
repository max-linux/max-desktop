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

import sys
import gtk
import os

import homealumno
import gobject

from subprocess import Popen, PIPE, STDOUT

def print_debug(txt):
    if homealumno.debug:
        print >> sys.stderr, "%s::%s" % ("homealumno-gui::edit", txt)

class HomeAlumnoGui(object):
    def __init__(self):
        print_debug("__init__()")
        
        self.profiles_model=gtk.TreeStore(str)
        
        import homealumno
        
        # Widgets
        self.ui = gtk.Builder()
        self.ui.set_translation_domain(homealumno.PACKAGE)
        print_debug("Loading ui file...")
        self.ui.add_from_file(homealumno.GLADE_DIR + 'homealumno-gui-main.ui')
        self.mainwindow = self.ui.get_object('mainwindow')
        if os.path.isfile('/usr/share/pixmaps/max.png'):
            self.mainwindow.set_icon_from_file('/usr/share/pixmaps/max.png')
        self.mainwindow.connect('destroy', self.quitapp )
        self.mainwindow.connect('delete_event', self.quitapp )
        
        # close windows signals
        self.mainwindow.connect('delete_event', self.quitapp)
        self.mainwindow.show_all()
        
        self.img_logo = self.ui.get_object('img_logo')
        self.img_logo.set_from_file(homealumno.IMG_DIR +'max_logo.png')
        
        self.tree_profiles=self.ui.get_object('tree_profiles')
        self.tree_profiles.set_model (self.profiles_model)
        
        self.profile_row = self.tree_profiles.get_selection()
        self.profile_row.connect("changed", self.on_tree_profiles_selected)
        
        # buttons
        self.btn_edit=self.ui.get_object('btn_edit')
        self.btn_delete=self.ui.get_object('btn_delete')
        self.btn_add=self.ui.get_object('btn_add')
        
        self.btn_close=self.ui.get_object('btn_close')
        self.btn_close.connect('clicked', self.quitapp)
        
        # init
        renderer = gtk.CellRendererText()
        column = gtk.TreeViewColumn('Nombre del perfil', renderer, text=0)
        self.tree_profiles.append_column(column)
        self.render_profiles()
        
        # init edit class
        import homealumno.edit
        self.edit=homealumno.edit.EditProfile()
        self.edit.triggers.append(self.render_profiles)
        self.btn_edit.connect('clicked', self.edit.show_edit, 'edit')
        self.btn_add.connect('clicked', self.edit.show_edit, 'add')
        self.btn_delete.connect('clicked', self.on_btn_delete)
        
        homealumno.profiler.reload_function=self.render_profiles()

    def on_tree_profiles_selected(self, *args):
        (model, iter) = self.tree_profiles.get_selection().get_selected()
        if not iter:
            self.control_buttons(False)
            homealumno.selected_profile=None
            return
        homealumno.selected_profile=model.get_value(iter,0)
        print_debug("selected_profile=%s" %(homealumno.selected_profile))
        self.control_buttons(True)

    def control_buttons(self, seteditable):
        if seteditable:
            self.btn_edit.set_sensitive(True)
            self.btn_delete.set_sensitive(True)
        else:
            self.btn_edit.set_sensitive(False)
            self.btn_delete.set_sensitive(False)

    def get_profiles(self):
        import homealumno.profiler
        obj=homealumno.profiler.Profiler()
        return obj.get_allprofiles()

    def on_btn_delete(self, *args):
        import homealumno
        if homealumno.selected_profile:
            import homealumno.profiler
            obj=homealumno.profiler.Profiler()
            obj.delete_profile(homealumno.selected_profile)
        self.render_profiles()

    def render_profiles(self):
        profiles=self.get_profiles()
        print_debug("render_profiles() %s"%profiles)
        self.profiles_model.clear()
        for profile in profiles:
            model=self.tree_profiles.get_model()
            print_debug("render_profiles() adding profile=%s" %profile)
            prof_iter = model.append (None)
            model.set_value (prof_iter, 0, profile )

    def exe_cmd(self, cmd, verbose=1):
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

    def quitapp(self, *args):
        print_debug ( "Exiting" )
        self.mainloop.quit()

    def run (self):
        self.mainloop = gobject.MainLoop()
        try:
            self.mainloop.run()
        except KeyboardInterrupt: # Press Ctrl+C
            self.quitapp()
