# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

import unittest
from bitten.tests import admin, master, model, recipe, queue, slave, web_ui, notify
from bitten.build import tests as build
from bitten.report import tests as report
from bitten.util import tests as util

def suite():
    suite = unittest.TestSuite()
    suite.addTest(admin.suite())
    suite.addTest(master.suite())
    suite.addTest(model.suite())
    suite.addTest(recipe.suite())
    suite.addTest(queue.suite())
    suite.addTest(slave.suite())
    suite.addTest(web_ui.suite())
    suite.addTest(build.suite())
    suite.addTest(report.suite())
    suite.addTest(util.suite())
    suite.addTest(notify.suite())
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
