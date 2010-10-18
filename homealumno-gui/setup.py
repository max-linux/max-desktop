#!/usr/bin/python
# -*- coding: UTF-8 -*-
# 
# This script is inspired by the debian package python-chardet
import os
import glob
from distutils.core import setup
from distutils.command.build import build

data_files = []

def get_debian_version():
    f=open('debian/changelog', 'r')
    line=f.readline()
    f.close()
    version=line.split()[1].replace('(','').replace(')','')
    return version

class build_homealumno(build):
    def run(self):

        # parse __VERSION__ in build_scripts
        for pyfile in glob.glob( "%s/*.py" %self.build_scripts):
            process_version(pyfile)
        
        libdir=self.build_lib + '/homealumno'
        for pyfile in glob.glob( "%s/*.py" %libdir):
            process_version(pyfile)
            
        extdir=libdir+'/extensions'
        for pyfile in glob.glob( "%s/*.py" %extdir):
            process_version(pyfile)
        
        build.run(self)

def process_version(pyfile):
    version=get_debian_version()
    print("sed -i -e 's/__VERSION__/%s/g' %s" %(version, pyfile) )
    os.system("sed -i -e 's/__VERSION__/%s/g' %s" %(version, pyfile) )

def get_images(ipath):
    images = []
    for afile in glob.glob('%s/*'%(ipath) ):
        if os.path.isfile(afile):
            images.append(afile)
    return images
        
data_files.append(('share/homealumno/images', get_images("images") ))


data_files.append(('share/homealumno', ['homealumno-gui-edit.ui', 'homealumno-gui-main.ui'] ))

data_files.append(('share/applications', ['homealumno-gui.desktop'] ))


data_files.append(('/etc/X11/Xsession.d/', ['58homealumno-gui'] ))

setup(name='HomeAlumnoGui',
      description = 'Configure homealumno templates',
      version=get_debian_version(),
      author = 'Mario Izquierdo',
      author_email = 'mariodebian@gmail.com',
      url = 'http://max.educa.madrid.org',
      license = 'GPLv2',
      platforms = ['linux'],
      keywords = ['teacher monitor', 'rsync'],
      scripts=['homealumno-gui'],
      packages=['homealumno'],
      cmdclass = {'build': build_homealumno},
      data_files=data_files
      )

