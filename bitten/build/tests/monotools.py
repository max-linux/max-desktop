# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2008 Matt Good <matt@matt-good.net>
# Copyright (C) 2008 Edgewall Software
# Copyright (C) 2009 Grzegorz Sobanski <silk@boktor.net>
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

import os
import cPickle as pickle
import shutil
import tempfile
import unittest

from bitten.build import monotools
from bitten.build import FileSet
from bitten.recipe import Context, Recipe



class NUnitTestCase(unittest.TestCase):

    def setUp(self):
        self.basedir = os.path.realpath(tempfile.mkdtemp())
        self.ctxt = Context(self.basedir)
        self.results_xml = open(os.path.join(self.basedir, 'test-results.xml'), 'w')

    def tearDown(self):
        shutil.rmtree(self.basedir)

    def test_missing_file_param(self):
        self.results_xml.close()
        self.assertRaises(AssertionError, monotools.nunit, self.ctxt)

    def test_empty_results(self):
        self.results_xml.write('<?xml version="1.0"?>'
                              '<test-results>'
                              '</test-results>')
        self.results_xml.close()
        monotools.nunit(self.ctxt, self.results_xml.name)
        type, category, generator, xml = self.ctxt.output.pop()
        self.assertEqual(Recipe.REPORT, type)
        self.assertEqual('test', category)
        self.assertEqual(0, len(xml.children))

    def test_successful_test_simple(self):
        self.results_xml.write(
'<?xml version="1.0" encoding="utf-8" standalone="no"?>'
'<!--This file represents the results of running a test suite-->'
'<test-results name="Test.dll" total="3" failures="1" not-run="0" date="2009-01-05" time="16:32:46">'
'  <environment nunit-version="2.4.7.0" clr-version="2.0.50727.42" os-version="Unix 2.6.26.1" platform="Unix" cwd="/home/silk/devel/trac/testcheckout" machine-name="tango" user="silk" user-domain="tango" />'
'  <culture-info current-culture="en-US" current-uiculture="en-US" />'
'  <test-suite name="Test.dll" success="False" time="0.081" asserts="0">'
'    <results>'
'      <test-case name="Lib.Test.KlasaTests.Test2" executed="True" success="True" time="0.001" asserts="1" />'
'    </results>'
'  </test-suite>'
'</test-results>'
)
        self.results_xml.close()
        monotools.nunit(self.ctxt, self.results_xml.name)
        type, category, generator, xml = self.ctxt.output.pop()
        self.assertEqual(1, len(xml.children))
        test_elem = xml.children[0]
        self.assertEqual('test', test_elem.name)
        self.assertEqual('0.001', test_elem.attr['duration'])
        self.assertEqual('success', test_elem.attr['status'])
        self.assertEqual('Test.dll', test_elem.attr['fixture'])


    def test_failure_test_simple(self):
        self.results_xml.write(
'<?xml version="1.0" encoding="utf-8" standalone="no"?>'
'<!--This file represents the results of running a test suite-->'
'<test-results name="Test.dll" total="3" failures="1" not-run="0" date="2009-01-05" time="16:32:46">'
'  <environment nunit-version="2.4.7.0" clr-version="2.0.50727.42" os-version="Unix 2.6.26.1" platform="Unix" cwd="/home/silk/devel/trac/testcheckout" machine-name="tango" user="silk" user-domain="tango" />'
'  <culture-info current-culture="en-US" current-uiculture="en-US" />'
'  <test-suite name="Test.dll" success="False" time="0.081" asserts="0">'
'    <results>'
'      <test-case name="Lib.Test.KlasaTests.Test2" executed="True" success="False" time="0.001" asserts="1">'
'                    <failure>'
'                      <message><![CDATA[  Expected: 2'
'  But was:  3'
']]></message>'
'                      <stack-trace><![CDATA[at Lib.Test.KlasaTests.Test1 () [0x00000]'
'at (wrapper managed-to-native) System.Reflection.MonoMethod:InternalInvoke (object,object[],System.Exception&)'
'at System.Reflection.MonoMethod.Invoke (System.Object obj, BindingFlags invokeAttr, System.Reflection.Binder binder, System.Object[] parameters, System.Globalization.CultureInfo culture) [0x00000]'
']]></stack-trace>'
'                    </failure>'
'      </test-case>'
'    </results>'
'  </test-suite>'
'</test-results>'
)
        self.results_xml.close()
        monotools.nunit(self.ctxt, self.results_xml.name)
        type, category, generator, xml = self.ctxt.output.pop()
        self.assertEqual(1, len(xml.children))
        test_elem = xml.children[0]
        self.assertEqual('test', test_elem.name)
        self.assertEqual('0.001', test_elem.attr['duration'])
        self.assertEqual('failure', test_elem.attr['status'])
        self.assertEqual('Test.dll', test_elem.attr['fixture'])
        self.assertEqual(1, len(test_elem.children))


    def test_successful_test_recursive(self):
        self.results_xml.write(
'<?xml version="1.0" encoding="utf-8" standalone="no"?>'
'<!--This file represents the results of running a test suite-->'
'<test-results name="Test.dll" total="3" failures="1" not-run="0" date="2009-01-05" time="16:32:46">'
'  <environment nunit-version="2.4.7.0" clr-version="2.0.50727.42" os-version="Unix 2.6.26.1" platform="Unix" cwd="/home/silk/devel/trac/testcheckout" machine-name="tango" user="silk" user-domain="tango" />'
'  <culture-info current-culture="en-US" current-uiculture="en-US" />'
'  <test-suite name="Test.dll" success="False" time="0.081" asserts="0">'
'    <results>'
    '  <test-suite name="Lib" success="False" time="0.078" asserts="0">'
    '    <results>'
        '  <test-suite name="Test" success="False" time="0.078" asserts="0">'
        '    <results>'
        '      <test-case name="Lib.Test.KlasaTests.Test2" executed="True" success="True" time="0.001" asserts="1" />'
    '    </results>'
    '  </test-suite>'
    '    </results>'
    '  </test-suite>'
'    </results>'
'  </test-suite>'
'</test-results>'
)
        self.results_xml.close()
        monotools.nunit(self.ctxt, self.results_xml.name)
        type, category, generator, xml = self.ctxt.output.pop()
        self.assertEqual(1, len(xml.children))
        test_elem = xml.children[0]
        self.assertEqual('test', test_elem.name)
        self.assertEqual('0.001', test_elem.attr['duration'])
        self.assertEqual('success', test_elem.attr['status'])
        self.assertEqual('Test', test_elem.attr['fixture'])


def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(NUnitTestCase, 'test'))
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
