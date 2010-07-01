#!/usr/bin/python
# -*- coding: UTF-8 -*-
# 
# This script is inspired by the debian package python-chardet
import os
import glob
from distutils.core import setup

data_files = []

def get_debian_version():
    f=open('debian/changelog', 'r')
    line=f.readline()
    f.close()
    version=line.split()[1].replace('(','').replace(')','')
    return version


def get_images(ipath):
    images = []
    for afile in glob.glob('%s/*'%(ipath) ):
        if os.path.isfile(afile):
            images.append(afile)
    return images
        
data_files.append(('share/max-ldap/images', get_images("images") ))

data_files.append(('share/max-ldap', ['max-ldap-main.ui'] ))

data_files.append(('share/applications', ['max-ldap.desktop'] ))

data_files.append(('/etc/X11/Xsession.d', ['80_configure_ldap_session'] ))

data_files.append(('share/gnome/shutdown', ['ldap_logout.sh'] ))

data_files.append(('/etc/grub.d', ['50_extlinux'] ))

setup(name='MAX-LDAP',
      description = 'Configure MAX as LDAP client',
      version=get_debian_version(),
      author = 'Mario Izquierdo',
      author_email = 'mariodebian@gmail.com',
      url = 'http://max.educa.madrid.org',
      license = 'GPLv2',
      platforms = ['linux'],
      keywords = ['ldap'],
      scripts=['max-ldap', 'max-control'],
      data_files=data_files
      )

