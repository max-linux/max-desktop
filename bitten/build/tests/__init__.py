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

from bitten.build.tests import api, config, ctools, \
                               monotools, phptools, pythontools, \
                               xmltools

def suite():
    suite = unittest.TestSuite()
    suite.addTest(api.suite())
    suite.addTest(config.suite())
    suite.addTest(ctools.suite())
    suite.addTest(monotools.suite())
    suite.addTest(phptools.suite())
    suite.addTest(pythontools.suite())
    suite.addTest(xmltools.suite())
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
