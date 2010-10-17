#!/usr/bin/python
# -*- coding: UTF-8 -*-
# 
# This script is inspired by the debian package python-chardet
import os
import glob
from distutils.core import setup
from distutils.command.install_data import install_data as install_data


data_files = []

def get_debian_version():
    f=open('debian/changelog', 'r')
    line=f.readline()
    f.close()
    version=line.split()[1].replace('(','').replace(')','')
    return version


class max_install_data(install_data):
    def run(self):
        install_data.run(self)
        
        # rename scripts (delete .py extension)
        for pyfile in glob.glob(self.install_dir + '/bin/*.py'):
            new=pyfile.split('.py')[0]
            print(" * Renaming %s => %s" %(pyfile, new ) )
            os.rename( pyfile, new )


# Interface files
data_files.append( ('share/max-configure-proxy', ['max-configure-proxy.glade'] ) )

# Desktop files
data_files.append( ('share/applications/', ['max-configure-proxy.desktop']) )


setup(name='MaxConfigureProxy',
      description = 'Configure proxy in /etc/profile.d',
      version=get_debian_version(),
      author = 'Mario Izquierdo',
      author_email = 'mariodebian@gmail.com',
      url = 'http://max.educa.madrid.org',
      license = 'GPLv2',
      platforms = ['linux'],
      scripts=['max-configure-proxy.py'],
      cmdclass = {'install_data' : max_install_data},
      data_files=data_files
      )

