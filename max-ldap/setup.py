#!/usr/bin/python
# -*- coding: UTF-8 -*-
# 
# This script is inspired by the debian package python-chardet
import os
import glob
from distutils.core import setup
from distutils.command.build import build

data_files = []

#class build_locales(build):
#    os.system("cd po && make gmo")

#for (path, dirs, files) in os.walk("po"):
#    if "max-ldap.mo" in files:
#        target = path.replace("po", "share/locale", 1)
#        data_files.append((target, [os.path.join(path, "max-ldap.mo")]))

def get_images(ipath):
    images = []
    for afile in glob.glob('%s/*'%(ipath) ):
        if os.path.isfile(afile):
            images.append(afile)
    return images
        
data_files.append(('share/max-ldap/images', get_images("images") ))

data_files.append(('share/max-ldap', ['max-ldap-main.ui'] ))

data_files.append(('share/applications', ['max-ldap.desktop'] ))

setup(name='MAX-LDAP',
      description = 'Configure MAX as LDAP client',
      version='0.1',
      author = 'Mario Izquierdo',
      author_email = 'mariodebian@gmail.com',
      url = 'http://max.educa.madrid.org',
      license = 'GPLv2',
      platforms = ['linux'],
      keywords = ['ldap'],
      scripts=['max-ldap'],
      #cmdclass = {'build': build_locales},
      data_files=data_files
      )

