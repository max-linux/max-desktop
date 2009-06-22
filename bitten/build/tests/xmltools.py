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

from bitten.build import xmltools
from bitten.recipe import Context
from bitten.util import xmlio


class TransformTestCase(unittest.TestCase):

    def setUp(self):
        self.basedir = os.path.realpath(tempfile.mkdtemp())
        self.ctxt = Context(self.basedir)

    def tearDown(self):
        shutil.rmtree(self.basedir)

    def test_transform_no_src(self):
        self.assertRaises(AssertionError, xmltools.transform, self.ctxt)

    def test_transform_no_dest(self):
        self.assertRaises(AssertionError, xmltools.transform, self.ctxt,
                          src='src.xml')

    def test_transform_no_stylesheet(self):
        self.assertRaises(AssertionError, xmltools.transform, self.ctxt,
                          src='src.xml', dest='dest.xml')

    def test_transform(self):
        src_file = file(self.ctxt.resolve('src.xml'), 'w')
        try:
            src_file.write("""<doc>
<title>Document Title</title>
<section>
<title>Section Title</title>
<para>This is a test.</para>
<note>This is a note.</note>
</section>
</doc>
""")
        finally:
            src_file.close()

        style_file = file(self.ctxt.resolve('style.xsl'), 'w')
        try:
            style_file.write("""<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/TR/xhtml1/strict">
 <xsl:template match="doc">
  <html>
   <head>
    <title><xsl:value-of select="title"/></title>
   </head>
   <body>
    <xsl:apply-templates/>
   </body>
  </html>
 </xsl:template>
 <xsl:template match="doc/title">
  <h1><xsl:apply-templates/></h1>
 </xsl:template>
 <xsl:template match="section/title">
  <h2><xsl:apply-templates/></h2>
 </xsl:template>
 <xsl:template match="para">
  <p><xsl:apply-templates/></p>
 </xsl:template>
 <xsl:template match="note">
  <p class="note"><b>NOTE: </b><xsl:apply-templates/></p>
 </xsl:template>
</xsl:stylesheet>
""")
        finally:
            style_file.close()

        xmltools.transform(self.ctxt, src='src.xml', dest='dest.xml',
                           stylesheet='style.xsl')

        dest_file = file(self.ctxt.resolve('dest.xml'))
        try:
            dest = xmlio.parse(dest_file)
        finally:
            dest_file.close()

        self.assertEqual('html', dest.name)
        self.assertEqual('http://www.w3.org/TR/xhtml1/strict', dest.namespace)
        children = list(dest.children())
        self.assertEqual(2, len(children))
        self.assertEqual('head', children[0].name)
        head_children = list(children[0].children())
        self.assertEqual(1, len(head_children))
        self.assertEqual('title', head_children[0].name)
        self.assertEqual('Document Title', head_children[0].gettext())
        self.assertEqual('body', children[1].name)
        body_children = list(children[1].children())
        self.assertEqual(4, len(body_children))
        self.assertEqual('h1', body_children[0].name)
        self.assertEqual('Document Title', body_children[0].gettext())
        self.assertEqual('h2', body_children[1].name)
        self.assertEqual('Section Title', body_children[1].gettext())
        self.assertEqual('p', body_children[2].name)
        self.assertEqual('This is a test.', body_children[2].gettext())
        self.assertEqual('p', body_children[3].name)
        self.assertEqual('note', body_children[3].attr['class'])
        self.assertEqual('This is a note.', body_children[3].gettext())


def suite():
    suite = unittest.TestSuite()
    if xmltools.have_libxslt or xmltools.have_msxml:
        suite.addTest(unittest.makeSuite(TransformTestCase, 'test'))
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
