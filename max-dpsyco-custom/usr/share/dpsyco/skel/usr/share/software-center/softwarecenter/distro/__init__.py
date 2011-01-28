# Copyright (C) 2009 Canonical
#
# Authors:
#  Michael Vogt
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; version 3.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

import logging
import subprocess

class UnimplementedError(Exception):
    pass

class Distro(object):
    """ abstract base class for a distribution """
    
    # missing thumbnail
    IMAGE_THUMBNAIL_MISSING = "/usr/share/software-center/images/dummy-thumbnail-ubuntu.png"
    IMAGE_FULL_MISSING = "/usr/share/software-center/images/dummy-screenshot-ubuntu.png"

    def get_codename(self):
        """ The codename of the distro, e.g. lucid """
        if not hasattr(self, "_distro_code_name"):
            self._distro_code_name = subprocess.Popen(
                ["lsb_release","-c","-s"], 
                stdout=subprocess.PIPE).communicate()[0].strip()
        return self._distro_code_name

    def get_distro_channel_name(self):
        """ The name in the Release file """
        return "none"
 
    def get_distro_channel_description(self):
        """ The name in the Release file """
        return "none"

    def get_installation_status(self, pkg):
        raise UnimplementedError

    def get_maintenance_status(self, cache, appname, pkgname, component, channel):
        raise UnimplementedError

    def get_price(self, doc):
        """ get the price for the given software 

        :param doc: The xapian document that contains the information
        :return: None if the price should not be displayed, a string otherwise
        """
        return None

    def get_license_text(self, component):
        raise UnimplementedError

    def is_supported(self, cache, doc, pkgname):
        """ 
        return True if the given document and pkgname is supported by 
        the distribution
        """
        raise UnimplementError

def _get_distro():
    #distro_id = subprocess.Popen(["lsb_release","-i","-s"], 
    #                             stdout=subprocess.PIPE).communicate()[0].strip()
    distro_id = "Max"
    logging.debug("get_distro: '%s'" % distro_id)
    # start with a import, this gives us only a softwarecenter module
    module =  __import__(distro_id, globals(), locals(), [], -1)
    # get the right class and instanciate it
    distro_class = getattr(module, distro_id)
    instance = distro_class()
    return instance

def get_distro():
    """ factory to return the right Distro object """
    return distro_instance

# singelton
distro_instance=_get_distro()


if __name__ == "__main__":
    print get_distro()
