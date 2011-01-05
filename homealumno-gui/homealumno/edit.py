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
import os
import gtk
import pwd
import subprocess

import homealumno

def print_debug(txt):
    if homealumno.debug:
        print >> sys.stderr, "%s::%s" % ("homealumno-gui::edit", txt)


class EditProfile(object):
    def __init__(self):
        print_debug("__init__()")
        self.selected_exc=None
        self.all_exceptions=None
        self.all_users=None
        self.triggers=[]
        self.init_edit()
    
    def init_edit(self):
        self.ui = gtk.Builder()
        self.ui.set_translation_domain(homealumno.PACKAGE)
        print_debug("Loading ui file...")
        self.ui.add_from_file(homealumno.GLADE_DIR + 'homealumno-gui-edit.ui')
        self.editwindow = self.ui.get_object('editwindow')
        if os.path.isfile('/usr/share/pixmaps/max.png')
            self.editwindow.set_icon_from_file('/usr/share/pixmaps/max.png')
        self.editwindow.connect('show',  self.on_editwindow_show)
        
        self.editwindow.connect('destroy', self.hide_edit )
        self.editwindow.connect('delete_event', self.hide_edit )
        
        self.lbl_edit=self.ui.get_object('lbl_edit')
        # don't show by default
        self.editwindow.hide()
        
        # buttons
        self.btn_edit_cancel=self.ui.get_object('btn_edit_cancel')
        self.btn_edit_cancel.connect('clicked', self.hide_edit)
        self.btn_edit_apply=self.ui.get_object('btn_edit_apply')
        self.btn_edit_apply.connect('clicked', self.save)
        
        self.btn_openprofile=self.ui.get_object('btn_openprofile')
        self.btn_openprofile.connect('clicked', self.open_profile)
        
        #
        self.txt_edit_path=self.ui.get_object('txt_edit_path')
        self.box_profile=self.ui.get_object('box_profile')
        
        self.lbl_profilename=self.ui.get_object('lbl_profilename')
        self.txt_profilename=self.ui.get_object('txt_profilename')
        self.txt_profilename.connect('changed', self.on_write_profilename)
        
        # init user profiles list
        self.tree_userlist=self.ui.get_object('tree_userlist')
        # model
        self.users_model=gtk.TreeStore(str, gtk.CheckButton, bool)
        self.tree_userlist.set_model (self.users_model)
        
        renderer1 = gtk.CellRendererText()
        column1 = gtk.TreeViewColumn('Usuarios', renderer1, text=0)
        self.tree_userlist.append_column(column1)
        
        renderer2 = gtk.CellRendererToggle()
        renderer2.connect('toggled', self.on_sel_click, self.users_model, 2)
        column2 = gtk.TreeViewColumn('Activo', renderer2, active=2)
        self.tree_userlist.append_column(column2)
        
        self.btn_delete_exc=self.ui.get_object('btn_delete_exc')
        self.btn_delete_exc.connect('clicked',  self.on_btn_delete_exc)
        self.btn_add_exc=self.ui.get_object('btn_add_exc')
        self.btn_add_exc.connect('clicked',  self.on_btn_add_exc)
        
        # init exceptions profiles list
        self.tree_except=self.ui.get_object('tree_except')
        self.exc_row = self.tree_except.get_selection()
        self.exc_row.connect("changed", self.on_tree_exc_selected)
        # model
        self.exc_model=gtk.TreeStore(str)
        self.tree_except.set_model (self.exc_model)
        
        renderer1 = gtk.CellRendererText()
        renderer1.set_property('editable', True)
        renderer1.connect('edited',  self.on_tree_exc_edited)
        column1 = gtk.TreeViewColumn('Rutas', renderer1, text=0)
        self.tree_except.append_column(column1)
        
        
        self.ck_compiz=self.ui.get_object('ck_compiz')
        self.ck_screensaver=self.ui.get_object('ck_screensaver')
        self.txt_wallpaper=self.ui.get_object('txt_wallpaper')
        
        self.lbl_excludes=self.ui.get_object('lbl_excludes')
        
        self.render_users()
        self.render_exceptions()
        self.load_default_excludes()

    def open_profile(self, *args):
        path=self.txt_edit_path.get_text()
        pid = subprocess.Popen(["/usr/bin/nautilus", path]).pid

    def hide_edit(self, *args):
        self.editwindow.hide()
        return True

    def on_editwindow_show(self, *args):
        print_debug("on_editwindow_show re-init edit window")
        self.load()
        self.render_users()
        self.render_exceptions()

    def show_edit(self, *args):
        if args[1] == 'edit':
            self.lbl_edit.set_markup('<b>Editando perfil: «%s»</b>'%homealumno.selected_profile)
            self.lbl_profilename.hide()
            self.txt_profilename.hide()
            print_debug("ocultando profilename")
            self.load_profile(homealumno.selected_profile)
        elif args[1] == 'add':
            self.lbl_edit.set_markup('<b>Añadiendo</b>')
            self.lbl_profilename.show()
            self.txt_profilename.show()
            print_debug("mostrando profilename")
            self.load_profile(None)
            homealumno.selected_profile=None
        
        self.editwindow.show_all()

    def load_profile(self, profile):
        if profile is None:
            print_debug("load_profile() new !!!")
            self.txt_profilename.set_text('')
            self.txt_profilename.set_sensitive(True)
            return
        self.txt_edit_path.set_text( os.path.join(homealumno.PROFILES_PATH + profile))
        self.txt_profilename.set_text(profile)
        self.txt_profilename.set_sensitive(False)

    def render_users(self):
        self.users_model.clear()
        users=self.get_system_users()
        for user in users:
            model=self.tree_userlist.get_model()
            print_debug("render_users() adding user=%s" %user)
            user_iter = model.append (None)
            model.set_value (user_iter, 0, user )
            model.set_value (user_iter, 2, users[user] )

    def on_sel_click(self, cell, path, model, col=0):
        iter = model.get_iter(path)
        self.users_model.set_value(iter, col, not model[path][col])
        print_debug("on_sel_click() user=%s status=%s allusers=%s" %(model[path][0], model[path][col], self.get_selected_users()))
        return True

    def get_selected_users(self):
        allusers=[]
        rows = []
        self.users_model.foreach(lambda model, path, iter: rows.append(path))
        for user in rows:
            iter=self.users_model.get_iter(user)
            if self.users_model.get_value(iter, 2): # column 2 is bool
                allusers.append(self.users_model.get_value(iter, 0)) # column0 is username
        return allusers

    def get_system_users(self, cache=True):
        if cache and self.all_users != None:
            return self.all_users
        self.all_users={}
        for line in pwd.getpwall():
            #pw_name, pw_passwd, pw_uid, pw_gid, pw_gecos, pw_dir, pw_shell
            if line[2] > 999 and line[2] < 3000:
                self.all_users[line[0]]=False
        return self.all_users

    def on_write_profilename(self, *args):
        self.txt_edit_path.set_text( os.path.join(homealumno.PROFILES_PATH + self.txt_profilename.get_text()))

#####################  exceptions #########################

    def get_exceptions(self, cache=True):
        if cache and self.all_exceptions:
            return self.all_exceptions
        return []

    def render_exceptions(self):
        self.exc_model.clear()
        exceps=self.get_exceptions()
        for exc in exceps:
            model=self.tree_except.get_model()
            print_debug("render_exceptions() adding exc=%s" %exc)
            exc_iter = model.append (None)
            model.set_value (exc_iter, 0, exc )

    def on_tree_exc_selected(self, *args):
        (model, iter) = self.tree_except.get_selection().get_selected()
        if not iter:
            self.control_buttons(False)
            self.selected_exc=None
            return
        self.selected_exc=model.get_value(iter,0)
        print_debug("selected_except=%s" %(self.selected_exc))
        self.control_buttons(True)

    def control_buttons(self, seteditable):
        if seteditable:
            self.btn_delete_exc.set_sensitive(True)
        else:
            self.btn_delete_exc.set_sensitive(False)

    def on_tree_exc_edited(self, cellrender, path, newtext):
        iter=self.exc_model.get_iter(path)
        self.exc_model.set_value(iter, 0, newtext)

    def on_btn_delete_exc(self, args):
        # get selected and remove it
        exceptions=[]
        self.exc_model.foreach(lambda model, path, iter: exceptions.append(model.get_value(iter, 0)) )
        newexc=[]
        for exc in exceptions:
            if self.selected_exc != exc:
                newexc.append(exc)
        self.all_exceptions=newexc
        self.render_exceptions()

    def on_btn_add_exc(self, args):
        # get selected and remove it
        self.all_exceptions=self.get_exceptions()
        self.all_exceptions.append('editame')
        self.render_exceptions()

#############################################################################
    def save(self, *args):
        if self.txt_profilename.get_text() == '':
            self.error_msg("El nombre del perfil no puede estar vacío")
            return
        
        if " " in self.txt_profilename.get_text():
            self.error_msg("El nombre del perfil no puede contener espacios")
            return
        
        import homealumno.profiler
        if homealumno.selected_profile is None:
            # new profile
            print_debug("save() new profile")
            prof=homealumno.profiler.Profiler()
            prof.new_profile(self.txt_profilename.get_text())
            homealumno.selected_profile=self.txt_profilename.get_text()
            print_debug("save() new profile => %s"%homealumno.selected_profile)
        
        
        # read users
        users=self.get_selected_users()
        print_debug("SAVE users=%s"%users)
        
        # read exceptions
        exceptions=[]
        self.exc_model.foreach(lambda model, path, iter: exceptions.append(model.get_value(iter, 0)) )
        print_debug("SAVE exceptions=%s"%exceptions)
        
        compiz=0
        screensaver=0
        if self.ck_compiz.get_active():
            compiz=1
        
        if self.ck_screensaver.get_active():
            screensaver=1
        
        wallpaper=self.txt_wallpaper.get_text()
        print_debug("SAVE compiz=%s screensaver=%s wallpaper='%s'"%(compiz, screensaver, wallpaper))
        
        prof=homealumno.profiler.Profiler()
        prof.set_profile(homealumno.selected_profile, 'users', users)
        prof.set_profile(homealumno.selected_profile, 'exceptions', exceptions)
        prof.set_profile(homealumno.selected_profile, 'compiz', compiz)
        prof.set_profile(homealumno.selected_profile, 'screensaver', screensaver)
        prof.set_profile(homealumno.selected_profile, 'wallpaper', wallpaper)
        prof.save()
        self.process_triggers()
        self.hide_edit()

    def load(self):
        import homealumno.profiler
        # load profiles.ini
        obj=homealumno.profiler.Profiler()
        profile=obj.get_profile(homealumno.selected_profile)
        
        # no profile found
        if profile is None:
            self.all_users=self.get_system_users(cache=False)
            for user in self.all_users:
                self.all_users[user]=False
            self.all_exceptions=[]
            return
        
        # load users
        if 'users' in profile:
            self.all_users=self.get_system_users(cache=False)
            for user in self.all_users:
                if user in profile['users']:
                    self.all_users[user]=True
        
        # load exceptions
        if 'exceptions' in profile:
            self.all_exceptions=profile['exceptions']
        
        
        # checkbox
        if 'compiz' in profile:
            compiz=False
            if int(profile['compiz']) == 1:
                compiz=True
            self.ck_compiz.set_active(compiz)
            print_debug("LOAD compiz=%s "%(compiz))
        
        if 'screensaver' in profile:
            screensaver=False
            if int(profile['screensaver']) == 1:
                screensaver=True
            self.ck_screensaver.set_active(screensaver)
            print_debug("LOAD screensaver=%s "%(screensaver))
        
        # wallpaper
        if 'wallpaper' in profile:
            self.txt_wallpaper.set_text(profile['wallpaper'])
            print_debug("LOAD wallpaper='%s' "%(profile['wallpaper']))

    def load_default_excludes(self):
        text='<b><small>Excluidos por defecto:</small></b>'
        for exc in homealumno.profiler.ALWAYS_EXCLUDE:
            text=text+"\n<small>%s</small>"%exc
        
        self.lbl_excludes.set_markup(text)

    def process_triggers(self):
        for trigger in self.triggers:
            trigger()

    def error_msg(self, txt):
        d = gtk.MessageDialog(None,
                      gtk.DIALOG_MODAL |
                      gtk.DIALOG_DESTROY_WITH_PARENT,
                      gtk.MESSAGE_WARNING,
                      gtk.BUTTONS_OK,
                      txt)
        d.run()
        d.destroy()
