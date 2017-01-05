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
import pwd
import grp

#sys.path.append('./')
import homealumno
import homealumno.gconfprofile

from configobj import ConfigObj
import subprocess
import tempfile

def print_debug(txt):
    if homealumno.debug:
        print >> sys.stderr, "%s::%s" % ("homealumno-gui::profiler", txt)

DEFAULT_CONF={'main':{'profiles':['alumno']},
              'alumno':{'users':['alumno'], 'exceptions':['.mozilla'], 
                        'compiz':0, 'screensaver':0, 'wallpaper':''},
             }

ALWAYS_EXCLUDE=['.xsession-errors', '.Xauthority', '.ICEauthority', '.dmrc', '.gvfs', '.kompozer', '.kompozer.net', 'Escritorio/Profesor']

class Profiler(object):
    def __init__(self):
        self.config=ConfigObj( homealumno.PROFILES_CONF_FILE )
        self.reload_function=None
        if len(self.config) < 1:
            self.set_default()
            print_debug("profiles.ini not found, creating it at %s"%(homealumno.PROFILES_CONF_FILE))
            self.save()

    def set_default(self):
        for key in DEFAULT_CONF:
            self.config[key]=DEFAULT_CONF[key]

    def new_profile(self, profilename):
        #self.config.reload()
        newprofiles=[]
        for profile in self.config['main']['profiles']:
            newprofiles.append(profile)
        newprofiles.append(profilename)
        self.config['main']['profiles']=newprofiles
        self.config[profilename]={'users':[], 'exceptions':[],
                                  'compiz':0, 'screensaver':0, 'wallpaper':''}
        self.save()
        path=os.path.join(homealumno.PROFILES_PATH, profilename)
        try:
            os.mkdir(path)
        except:
            pass

    def get_allprofiles(self):
        return self.config['main']['profiles']

    def delete_profile(self, profilename):
        if profilename in self.config:
            self.config.pop(profilename)
        print_debug("delete_profile(%s) profiles=%s"%(profilename, self.config['main']['profiles']))
        newprofiles=[]
        for profile in self.config['main']['profiles']:
            if profilename != profile:
                newprofiles.append(profile)
        self.config['main']['profiles']=newprofiles
        print_debug("delete_profile(%s) profiles=%s"%(profilename, self.config['main']['profiles']))
        self.save()

    def get_profile(self, profile):
        if not profile in self.config:
            return None
        return self.config[profile]

    def set_profile(self, profile, key, value):
        self.config[profile][key]=value

    def save(self):
        print_debug("save() CONFIG")
        self.config.write()
        self.config.reload()
        if self.reload_function:
            print_debug("save() calling %s"%self.reload_function)
            self.reload_function()

    def doapply(self):
        currentusername=pwd.getpwuid(os.getuid()).pw_name
        currenthome=pwd.getpwuid(os.getuid()).pw_dir
        (fd, tmpexc)=tempfile.mkstemp()
        
        print_debug("applying profiles user=%s home=%s..."%(currentusername, currenthome))
        for profilename in self.get_allprofiles():
            profile=self.get_profile(profilename)
            if currentusername in profile['users']:
                print_debug("found %s in profile[%s]=%s"%(currentusername, profilename, profile))
                excludes=""
                for exc in ALWAYS_EXCLUDE:
                    os.write(fd, "%s\n"%exc)
                for exc in profile['exceptions']:
                        os.write(fd, "%s\n"%exc)

                # close fd and append to rsync commands
                os.close(fd)
                excludes="--exclude-from=%s"%tmpexc
                if homealumno.debug:
                    self.exe("cat %s"%tmpexc)

                # ejecutar pre-run scripts
                post_run=os.path.abspath(homealumno.PRERUN_PATH + profilename)
                if os.path.isfile(post_run):
                    self.exe(post_run)
                
                # hacer un rsync desde /etc/skel con exclude
                #print_debug("rsync %s -Pav /etc/skel/ --delete %s/"%(excludes, currenthome))
                self.exe("rsync %s -Pav /etc/skel/ --delete %s/"%(excludes, currenthome))

                self.exe("xdg-user-dirs-update --force")
                # support ATNAG
                try:
                    if grp.getgrnam('atnag'):
                        self.exe("mkdir -p %s/Escritorio"%currenthome)
                        self.exe("chgrp atnag %s/Escritorio"%currenthome)
                        self.exe("chmod 775 %s/Escritorio"%currenthome)
                except:
                    pass
                
                # hacer un rsync desde PROFILES_PATH + profilename con exclude
                absprof=os.path.abspath(homealumno.PROFILES_PATH + profilename)
                if os.path.isdir(absprof):
                    #print_debug("rsync %s -Pav %s/ %s/"%(excludes, absprof, currenthome))
                    self.exe("rsync %s -Pav %s/ %s/"%(excludes, absprof, currenthome))

                # delete filetemp file
                if os.path.isfile(tmpexc):
                    os.unlink(tmpexc)

                # ejecutar post-run scripts
                post_run=os.path.abspath(homealumno.POSTRUN_PATH + profilename)
                if os.path.isfile(post_run):
                    self.exe(post_run)
                
                # aplicar cambios de salvapantallas y fondo
                if 'compiz' in profile and int(profile['compiz']) == 1:
                    print_debug("desactivar compiz")
                    app=homealumno.gconfprofile.GconfProfile(os.getuid(), os.getgid(), direct=False)
                    data=[{'key':'/desktop/gnome/applications/window_manager/current',
                             'type':'string',
                             'value':'/usr/bin/metacity'},
                             {'key':'/desktop/gnome/applications/window_manager/default',
                             'type':'string',
                             'value':'/usr/bin/metacity'},
                             {'key':'/desktop/gnome/session/required_components/windowmanager',
                             'type':'string',
                             'value':'metacity'}]
                    app.do(data)
                
                if 'screensaver' in profile and int(profile['screensaver']) == 1:
                    print_debug("desactivar screensaver")
                    app=homealumno.gconfprofile.GconfProfile(os.getuid(), os.getgid(), direct=False)
                    # mate screensaver
                    data=[{'key':'/org/mate/screensaver/mode',
                           'type':'string',
                           'value':'blank-only'},
                          {'key':'/org/mate/screensaver/lock_enabled',
                           'type':'bool',
                           'value':'false'}]
                    app.do(data)
                    # gnome screensaver
                    data=[{'key':'/apps/gnome-screensaver/mode',
                           'type':'string',
                           'value':'blank-only'},
                          {'key':'/apps/gnome-screensaver/lock_enabled',
                           'type':'bool',
                           'value':'false'}]
                    app.do(data)
                
                if 'wallpaper' in profile and profile['wallpaper'] != "" :
                    print_debug("aplicar wallpaper '%s'"%profile['wallpaper'])
                    app=homealumno.gconfprofile.GconfProfile(os.getuid(), os.getgid(), direct=False)
                    # mate wallpaper
                    data=[{'key':'/org/mate/desktop/background/picture-filename',
                           'type':'string',
                           'value':profile['wallpaper']}]
                    app.do(data)
                    # gnome wallpaper
                    data=[{'key':'/desktop/gnome/background/picture_filename',
                           'type':'string',
                           'value':profile['wallpaper']}]
                    app.do(data)

    def exe(self, cmd):
        #print_debug(cmd)
        _cmd=cmd.split()
        print_debug(_cmd)
        
        subp = subprocess.Popen(_cmd,
                  stdout=subprocess.PIPE,
                  stderr=subprocess.PIPE)

        stdout,stderr=subp.communicate()
        if int(subp.returncode) != 0:
            print_debug("WARNING STDERR=%s"%stderr.strip('\n'))
        print_debug("exe output:%s"%stdout)
        return stdout.rstrip('\n')


if __name__ == '__main__':
    homealumno.debug=True
    app=Profiler()
    for profile in app.get_allprofiles():
        print app.get_profile(profile)
