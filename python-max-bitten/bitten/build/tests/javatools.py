# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2006 Matthew Good <matt@matt-good.net>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

import os.path
import shutil
import tempfile
import unittest

from bitten.build import javatools
from bitten.recipe import Context

class CoberturaTestCase(unittest.TestCase):
    xml_template="""<?xml version="1.0"?>
<!DOCTYPE coverage SYSTEM "http://cobertura.sourceforge.net/xml/coverage-02.dtd">

<coverage timestamp="1148533713840">
  <sources>
    <source>src</source>
  </sources>
  <packages>
    <package name="test">
      <classes>%s
      </classes>
    </package>
  </packages>
</coverage>"""

    def setUp(self):
        self.basedir = os.path.realpath(tempfile.mkdtemp())
        self.ctxt = Context(self.basedir)

    def tearDown(self):
        shutil.rmtree(self.basedir)

    def _create_file(self, *path, **kw):
        filename = os.path.join(self.basedir, *path)
        dirname = os.path.dirname(filename)
        if not os.path.exists(dirname):
            os.makedirs(dirname)
        fd = file(filename, 'w')
        content = kw.get('content')
        if content is not None:
            fd.write(content)
        fd.close()
        return filename[len(self.basedir) + 1:]

    def test_basic(self):
        filename = self._create_file('coverage.xml', content=self.xml_template % """
        <class name="test.TestClass" filename="test/TestClass.java">
          <lines>
            <line number="1" hits="0" branch="false"/>
            <line number="2" hits="1" branch="false"/>
            <line number="3" hits="0" branch="false"/>
            <line number="4" hits="2" branch="false"/>
          </lines>
        </class>""")
        javatools.cobertura(self.ctxt, file_=filename)
        type, category, generator, xml = self.ctxt.output.pop()
        self.assertEqual('report', type)
        self.assertEqual('coverage', category)
        self.assertEqual(1, len(xml.children))

        elem = xml.children[0]
        self.assertEqual('coverage', elem.name)
        self.assertEqual('src/test/TestClass.java', elem.attr['file'])
        self.assertEqual('test.TestClass', elem.attr['name'])
        self.assertEqual(4, elem.attr['lines'])
        self.assertEqual(50, elem.attr['percentage'])

    def test_skipped_lines(self):
        filename = self._create_file('coverage.xml', content=self.xml_template % """
        <class name="test.TestClass" filename="test/TestClass.java">
          <lines>
            <line number="1" hits="0" branch="false"/>
            <line number="3" hits="1" branch="false"/>
          </lines>
        </class>""")
        javatools.cobertura(self.ctxt, file_=filename)
        type, category, generator, xml = self.ctxt.output.pop()
        self.assertEqual('report', type)
        self.assertEqual('coverage', category)
        self.assertEqual(1, len(xml.children))

        elem = xml.children[0]
        self.assertEqual('coverage', elem.name)
        self.assertEqual('src/test/TestClass.java', elem.attr['file'])
        self.assertEqual('test.TestClass', elem.attr['name'])
        self.assertEqual(2, elem.attr['lines'])
        self.assertEqual(50, elem.attr['percentage'])

        line_hits = elem.children[0]
        self.assertEqual('line_hits', line_hits.name)
        self.assertEqual('0 - 1', line_hits.children[0])

    def test_interface(self):
        filename = self._create_file('coverage.xml', content=self.xml_template % """
        <class name="test.TestInterface" filename="test/TestInterface.java">
          <lines>
          </lines>
        </class>""")
        javatools.cobertura(self.ctxt, file_=filename)
        type, category, generator, xml = self.ctxt.output.pop()
        self.assertEqual('report', type)
        self.assertEqual('coverage', category)
        self.assertEqual(1, len(xml.children))

        elem = xml.children[0]
        self.assertEqual('coverage', elem.name)
        self.assertEqual('src/test/TestInterface.java', elem.attr['file'])
        self.assertEqual('test.TestInterface', elem.attr['name'])
        self.assertEqual(0, elem.attr['lines'])
        self.assertEqual(0, elem.attr['percentage'])


def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(CoberturaTestCase, 'test'))
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
