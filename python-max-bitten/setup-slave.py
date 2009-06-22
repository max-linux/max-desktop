#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (C) 2007 Edgewall Software
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2005-2007 David Fraser <davidf@sjsoft.com>
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

from setuptools import setup
from setuptools.command import egg_info

NS = 'http://bitten.cmlenz.net/tools/'

# TODO: there must be a way to pass this altered value in...
egg_info.manifest_maker.template = "MANIFEST-SLAVE.in"

setup(
    name = 'Bitten-Slave',
    version = '0.6',
    description = 'Continuous integration build slave for Trac',
    long_description = \
"""A slave for running builds and submitting them to Bitten, the continuous integration system for Trac""",
    author = 'Edgewall Software',
    author_email = 'info@edgewall.org',
    license = 'BSD',
    url = 'http://bitten.edgewall.org/',
    download_url = 'http://bitten.edgewall.org/wiki/Download',
    zip_safe = False,

    py_modules = ["bitten.__init__", "bitten.slave",
                 "bitten.build.__init__", "bitten.build.api", "bitten.build.config",
                 "bitten.recipe", "bitten.tests.slave",
                 "bitten.util.__init__", "bitten.util.testrunner", "bitten.util.xmlio",
                ],
    test_suite = 'bitten.tests.slave',
    entry_points = {
        'console_scripts': [
            'bitten-slave = bitten.slave:main'
        ],
        'distutils.commands': [
            'unittest = bitten.util.testrunner:unittest'
        ],
    },
)

