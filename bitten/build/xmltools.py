# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Recipe commands for XML processing."""

import logging
import os

try:
    import libxml2
    import libxslt
    have_libxslt = True
except ImportError:
    have_libxslt = False

if not have_libxslt and os.name == 'nt':
    try:
        import win32com.client
        have_msxml = True
    except ImportError:
        have_msxml = False
else:
    have_msxml = False

log = logging.getLogger('bitten.build.xmltools')

__docformat__ = 'restructuredtext en'

def transform(ctxt, src=None, dest=None, stylesheet=None):
    """Apply an XSLT stylesheet to a source XML document.
    
    This command requires either libxslt (with Python bindings), or MSXML to
    be installed.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param src: name of the XML input file
    :param dest: name of the XML output file
    :param stylesheet: name of the file containing the XSLT stylesheet
    """
    assert src, 'Missing required attribute "src"'
    assert dest, 'Missing required attribute "dest"'
    assert stylesheet, 'Missing required attribute "stylesheet"'

    if have_libxslt:
        log.debug('Using libxslt for XSLT transformation')
        srcdoc, styledoc, result = None, None, None
        try:
            srcdoc = libxml2.parseFile(ctxt.resolve(src))
            styledoc = libxslt.parseStylesheetFile(ctxt.resolve(stylesheet))
            result = styledoc.applyStylesheet(srcdoc, None)
            styledoc.saveResultToFilename(ctxt.resolve(dest), result, 0)
        finally:
            if styledoc:
                styledoc.freeStylesheet()
            if srcdoc:
                srcdoc.freeDoc()
            if result:
                result.freeDoc()

    elif have_msxml:
        log.debug('Using MSXML for XSLT transformation')
        srcdoc = win32com.client.Dispatch('MSXML2.DOMDocument.3.0')
        if not srcdoc.load(ctxt.resolve(src)):
            err = srcdoc.parseError
            ctxt.error('Failed to parse XML source %s: %s', src, err.reason)
            return
        styledoc = win32com.client.Dispatch('MSXML2.DOMDocument.3.0')
        if not styledoc.load(ctxt.resolve(stylesheet)):
            err = styledoc.parseError
            ctxt.error('Failed to parse XSLT stylesheet %s: %s', stylesheet,
                       err.reason)
            return
        result = srcdoc.transformNode(styledoc)

        # MSXML seems to always write produce the resulting XML document using
        # UTF-16 encoding, regardless of the encoding specified in the
        # stylesheet. For better interoperability, recode to UTF-8 here.
        result = result.encode('utf-8').replace(' encoding="UTF-16"?>', '?>')

        dest_file = file(ctxt.resolve(dest), 'w')
        try:
            dest_file.write(result)
        finally:
            dest_file.close()

    else:
        ctxt.error('No usable XSLT implementation found')

        # TODO: as a last resort, try to invoke 'xsltproc' to do the
        #       transformation?
