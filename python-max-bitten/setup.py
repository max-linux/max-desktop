#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (C) 2007 Edgewall Software
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

import os
from setuptools import setup, find_packages
import sys

sys.path.append(os.path.join('doc', 'common'))
try:
    from doctools import build_doc, test_doc
except ImportError:
    build_doc = test_doc = None

NS = 'http://bitten.cmlenz.net/tools/'

setup(
    name = 'Bitten',
    version = '0.6',
    description = 'Continuous integration for Trac',
    long_description = \
"""A Trac plugin for collecting software metrics via continuous integration.""",
    author = 'Edgewall Software',
    author_email = 'info@edgewall.org',
    license = 'BSD',
    url = 'http://bitten.edgewall.org/',
    download_url = 'http://bitten.edgewall.org/wiki/Download',
    zip_safe = False,

    packages = find_packages(exclude=['*.tests*']),
    package_data = {
        'bitten': ['htdocs/*.*',
                   'htdocs/charts_library/*.swf',
                   'templates/*.html',
                   'templates/*.txt']
    },
    test_suite = 'bitten.tests.suite',
    tests_require = [
        'figleaf',
    ],
    entry_points = {
        'console_scripts': [
            'bitten-slave = bitten.slave:main'
        ],
        'distutils.commands': [
            'unittest = bitten.util.testrunner:unittest'
        ],
        'trac.plugins': [
            'bitten.admin = bitten.admin',
            'bitten.main = bitten.main',
            'bitten.master = bitten.master',
            'bitten.web_ui = bitten.web_ui',
            'bitten.testing = bitten.report.testing',
            'bitten.coverage = bitten.report.coverage',
            'bitten.lint = bitten.report.lint',
            'bitten.notify = bitten.notify'
        ],
        'bitten.recipe_commands': [
            NS + 'sh#exec = bitten.build.shtools:exec_',
            NS + 'sh#pipe = bitten.build.shtools:pipe',
            NS + 'c#configure = bitten.build.ctools:configure',
            NS + 'c#autoreconf = bitten.build.ctools:autoreconf',
            NS + 'c#cppunit = bitten.build.ctools:cppunit',
            NS + 'c#cunit = bitten.build.ctools:cunit',
            NS + 'c#gcov = bitten.build.ctools:gcov',
            NS + 'c#make = bitten.build.ctools:make',
            NS + 'mono#nunit = bitten.build.monotools:nunit',
            NS + 'java#ant = bitten.build.javatools:ant',
            NS + 'java#junit = bitten.build.javatools:junit',
            NS + 'java#cobertura = bitten.build.javatools:cobertura',
            NS + 'php#phing = bitten.build.phptools:phing',
            NS + 'php#phpunit = bitten.build.phptools:phpunit',
            NS + 'php#coverage = bitten.build.phptools:coverage',
            NS + 'python#coverage = bitten.build.pythontools:coverage',
            NS + 'python#distutils = bitten.build.pythontools:distutils',
            NS + 'python#exec = bitten.build.pythontools:exec_',
            NS + 'python#figleaf = bitten.build.pythontools:figleaf',
            NS + 'python#pylint = bitten.build.pythontools:pylint',
            NS + 'python#trace = bitten.build.pythontools:trace',
            NS + 'python#unittest = bitten.build.pythontools:unittest',
            NS + 'svn#checkout = bitten.build.svntools:checkout',
            NS + 'svn#export = bitten.build.svntools:export',
            NS + 'svn#update = bitten.build.svntools:update',
            NS + 'hg#pull = bitten.build.hgtools:pull',
            NS + 'xml#transform = bitten.build.xmltools:transform'
        ]
    },

    cmdclass = {'build_doc': build_doc, 'test_doc': test_doc}
)
