# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

import os
import shutil
import tempfile
import unittest

from bitten.slave import BuildSlave, ExitSlave

class BuildSlaveTestCase(unittest.TestCase):

    def setUp(self):
        self.work_dir = tempfile.mkdtemp(prefix='bitten_test')
        self.slave = BuildSlave([], work_dir=self.work_dir)

    def tearDown(self):
        shutil.rmtree(self.work_dir)

    def _create_file(self, *path):
        filename = os.path.join(self.work_dir, *path)
        fd = file(filename, 'w')
        fd.close()
        return filename

    def test_quit_raises(self):
        self.assertRaises(ExitSlave, self.slave.quit)

def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(BuildSlaveTestCase, 'test'))
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
