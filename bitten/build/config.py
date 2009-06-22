# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Support for build slave configuration."""

from ConfigParser import SafeConfigParser
import logging
import os
import platform
import re

log = logging.getLogger('bitten.config')

__docformat__ = 'restructuredtext en'


class Configuration(object):
    """Encapsulates the configuration of a build machine.
    
    Configuration values can be provided through a configuration file (in INI
    format) or through command-line parameters (properties). In addition to
    explicitly defined properties, this class automatically collects platform
    information and stores them as properties. These defaults can be
    overridden (useful for cross-compilation).
    """
    # TODO: document mapping from config file to property names

    def __init__(self, filename=None, properties=None):
        """Create the configuration object.
        
        :param filename: the path to the configuration file, if any
        :param properties: a dictionary of the configuration properties
                           provided on the command-line
        """
        self.properties = {}
        self.packages = {}
        parser = SafeConfigParser()
        if filename:
            parser.read(filename)
        self._merge_sysinfo(parser, properties)
        self._merge_packages(parser, properties)

    def _merge_sysinfo(self, parser, properties):
        """Merge the platform information properties into the configuration."""
        system, _, release, version, machine, processor = platform.uname()
        system, release, version = platform.system_alias(system, release,
                                                         version)
        self.properties['machine'] = machine
        self.properties['processor'] = processor
        self.properties['os'] = system
        self.properties['family'] = os.name
        self.properties['version'] = release

        mapping = {'machine': ('machine', 'name'),
                   'processor': ('machine', 'processor'),
                   'os': ('os', 'name'),
                   'family': ('os', 'family'),
                   'version': ('os', 'version')}
        for key, (section, option) in mapping.items():
            if parser.has_section(section):
                value = parser.get(section, option)
                if value is not None:
                    self.properties[key] = value

        if properties:
            for key, value in properties.items():
                if key in mapping:
                    self.properties[key] = value

    def _merge_packages(self, parser, properties):
        """Merge package information into the configuration."""
        for section in parser.sections():
            if section in ('os', 'machine', 'maintainer'):
                continue
            package = {}
            for option in parser.options(section):
                package[option] = parser.get(section, option)
            self.packages[section] = package

        if properties:
            for key, value in properties.items():
                if '.' in key:
                    package, propname = key.split('.', 1)
                    if package not in self.packages:
                        self.packages[package] = {}
                    self.packages[package][propname] = value

    def __contains__(self, key):
        """Return whether the configuration contains a value for the specified
        key.
        
        :param key: name of the configuration option using dotted notation
                    (for example, "python.path")
        """
        if '.' in key:
            package, propname = key.split('.', 1)
            return propname in self.packages.get(package, {})
        return key in self.properties

    def __getitem__(self, key):
        """Return the value for the specified configuration key.
        
        :param key: name of the configuration option using dotted notation
                    (for example, "python.path")
        """
        if '.' in key:
            package, propname = key.split('.', 1)
            return self.packages.get(package, {}).get(propname)
        return self.properties.get(key)

    def __str__(self):
        return str({'properties': self.properties, 'packages': self.packages})

    def get_dirpath(self, key):
        """Return the value of the specified configuration key, but verify that
        the value refers to the path of an existing directory.
        
        If the value does not exist, or is not a directory path, return `None`.

        :param key: name of the configuration option using dotted notation
                    (for example, "ant.home")
        """
        dirpath = self[key]
        if dirpath:
            if os.path.isdir(dirpath):
                return dirpath
            log.warning('Invalid %s: %s is not a directory', key, dirpath)
        return None

    def get_filepath(self, key):
        """Return the value of the specified configuration key, but verify that
        the value refers to the path of an existing file.
        
        If the value does not exist, or is not a file path, return `None`.

        :param key: name of the configuration option using dotted notation
                    (for example, "python.path")
        """
        filepath = self[key]
        if filepath:
            if os.path.isfile(filepath):
                return filepath
            log.warning('Invalid %s: %s is not a file', key, filepath)
        return None

    _VAR_RE = re.compile(r'\$\{(?P<ref>\w[\w.]*?\w)(?:\:(?P<def>.+))?\}')

    def interpolate(self, text, **vars):
        """Interpolate configuration properties into a string.
        
        Properties can be referenced in the text using the notation
        ``${property.name}``. A default value can be provided by appending it to
        the property name separated by a colon, for example
        ``${property.name:defaultvalue}``. This value will be used when there's
        no such property in the configuration. Otherwise, if no default is
        provided, the reference is not replaced at all.

        :param text: the string containing variable references
        :param vars: extra variables to use for the interpolation
        """
        def _replace(m):
            refname = m.group('ref')
            if refname in self:
                return self[refname]
            elif refname in vars:
                return vars[refname]
            elif m.group('def'):
                return m.group('def')
            else:
                return m.group(0)
        return self._VAR_RE.sub(_replace, text)
