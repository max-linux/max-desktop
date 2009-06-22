# -*- coding: UTF-8 -*-
#
# Copyright (C) 2007 Edgewall Software
# Copyright (C) 2007 Wei Zhuo <weizhuo@gmail.com>
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.cmlenz.net/wiki/License.

import os
import shutil
import tempfile
import unittest

from bitten.build import phptools
from bitten.recipe import Context, Recipe

class PhpUnitTestCase(unittest.TestCase):

    def setUp(self):
        self.basedir = os.path.realpath(tempfile.mkdtemp())
        self.ctxt = Context(self.basedir)

    def tearDown(self):
        shutil.rmtree(self.basedir)

    def test_missing_param_file(self):
        self.assertRaises(AssertionError, phptools.phpunit, self.ctxt)

    def test_sample_unit_test_result(self):
        phpunit_xml = file(self.ctxt.resolve('phpunit.xml'), 'w')
        phpunit_xml.write("""<?xml version="1.0" encoding="UTF-8"?>
<testsuites>
  <testsuite name="FooTest" file="FooTest.php" tests="2" failures="1" errors="0" time="0.147397">
    <testcase name="testBar" class="FooTest" time="0.122265">
      <failure message="expected same: &lt;1&gt; was not: &lt;2&gt;" type="PHPUnit2_Framework_AssertionFailedError">
      ...
</failure>
    </testcase>
    <testcase name="testBar2" class="FooTest" time="0.025132"/>
  </testsuite>
  <testsuite name="BarTest" file="BarTest.php" tests="1" failures="0" errors="0" time="0.050713">
    <testcase name="testFoo" class="BarTest" time="0.026046"/>
  </testsuite>
</testsuites>""")
        phpunit_xml.close()
        phptools.phpunit(self.ctxt, file_='phpunit.xml')
        type, category, generator, xml = self.ctxt.output.pop()
        self.assertEqual(Recipe.REPORT, type)
        self.assertEqual('test', category)

        tests = list(xml.children)
        self.assertEqual(3, len(tests))
        self.assertEqual('FooTest', tests[0].attr['fixture'])
        self.assertEqual('testBar', tests[0].attr['name'])
        self.assertEqual('failure', tests[0].attr['status'])
        self.assert_('FooTest.php' in tests[0].attr['file'])

        self.assertEqual('FooTest', tests[1].attr['fixture'])
        self.assertEqual('testBar2', tests[1].attr['name'])
        self.assertEqual('success', tests[1].attr['status'])

        self.assertEqual('BarTest', tests[2].attr['fixture'])
        self.assertEqual('testFoo', tests[2].attr['name'])
        self.assertEqual('success', tests[2].attr['status'])
        
class PhpCodeCoverageTestCase(unittest.TestCase):

    def setUp(self):
        self.basedir = os.path.realpath(tempfile.mkdtemp())
        self.ctxt = Context(self.basedir)

    def tearDown(self):
        shutil.rmtree(self.basedir)

    def test_missing_param_file(self):
        self.assertRaises(AssertionError, phptools.coverage, self.ctxt)

    def test_sample_code_coverage(self):
        coverage_xml = file(self.ctxt.resolve('phpcoverage.xml'), 'w')
        coverage_xml.write("""<?xml version="1.0" encoding="UTF-8"?>
<snapshot methodcount="4" methodscovered="2" statementcount="11" statementscovered="5" totalcount="15" totalcovered="7">
  <package name="default" methodcount="4" methodscovered="2" statementcount="11" statementscovered="5" totalcount="15" totalcovered="7">
    <class name="Foo" methodcount="1" methodscovered="1" statementcount="7" statementscovered="3" totalcount="8" totalcovered="4">
      <sourcefile name="Foo.php" sourcefile="xxxx/Foo.php">
	  ...
      </sourcefile>
    </class>
    <class name="Foo2" methodcount="2" methodscovered="1" statementcount="4" statementscovered="2" totalcount="6" totalcovered="3">
      <sourcefile name="Foo.php" sourcefile="xxxx/Foo.php">
        ...
      </sourcefile>
    </class>
    <class name="Bar" methodcount="1" methodscovered="0" statementcount="0" statementscovered="0" totalcount="1" totalcovered="0">
      <sourcefile name="Bar.php" sourcefile="xxxx/Bar.php">
        ...
      </sourcefile>
    </class>
  </package>
</snapshot>""")
        coverage_xml.close()
        phptools.coverage(self.ctxt, file_='phpcoverage.xml')
        type, category, generator, xml = self.ctxt.output.pop()
        self.assertEqual(Recipe.REPORT, type)
        self.assertEqual('coverage', category)

        coverage = list(xml.children)
        self.assertEqual(3, len(coverage))
        self.assertEqual(7, coverage[0].attr['lines'])
        self.assertEqual('Foo', coverage[0].attr['name'])
        self.assert_('xxxx/Foo.php' in coverage[0].attr['file'])

        self.assertEqual(4, coverage[1].attr['lines'])
        self.assertEqual(50.0, coverage[1].attr['percentage'])
        self.assertEqual('Foo2', coverage[1].attr['name'])
        self.assert_('xxxx/Foo.php' in coverage[1].attr['file'])
        
        self.assertEqual(0, coverage[2].attr['lines'])
        self.assertEqual(100.0, coverage[2].attr['percentage'])
        self.assertEqual('Bar', coverage[2].attr['name'])
        self.assert_('xxxx/Bar.php' in coverage[2].attr['file'])

def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(PhpUnitTestCase, 'test'))
    suite.addTest(unittest.makeSuite(PhpCodeCoverageTestCase, 'test'))
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
