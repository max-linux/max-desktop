# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

from StringIO import StringIO

from bitten.build import api


class CommandLine(api.CommandLine):

    def __init__(self, returncode=0, stdout='', stderr=''):
        self.returncode = returncode
        self.stdout = StringIO(stdout)
        self.stderr = StringIO(stderr)

    def __call__(self, executable, args, input=None, cwd=None):
        return self

    def execute(self):
        return api._combine(self.stdout.readlines(), self.stderr.readlines())
