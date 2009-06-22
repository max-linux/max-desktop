# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2006 Matthew Good <matt@matt-good.net>
# Copyright (C) 2007 Edgewall Software
# Copyright (C) 2009 Grzegorz Sobanski <silk@boktor.net>
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Recipe commands for tools commonly used in Mono projects."""

from glob import glob
import logging
import os
import posixpath
import shlex
import tempfile

from bitten.build import CommandLine
from bitten.util import xmlio

log = logging.getLogger('bitten.build.monotools')

__docformat__ = 'restructuredtext en'


def _parse_suite(element):
    for child in element.children('results'):
        testcases = list(child.children('test-case'))
        if testcases:
            yield element, testcases

        for xmlsuite in child.children('test-suite'):
            for suite in _parse_suite(xmlsuite):
                yield suite


def _get_cases(fileobj):
    for testsuite in xmlio.parse(fileobj).children('test-suite'):
        for suite in _parse_suite(testsuite):
            yield suite


def nunit(ctxt, file_=None):
    """Extract test results from a NUnit XML report.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param file\_: path to the NUnit XML test results; may contain globbing
                  wildcards for matching multiple results files
    """
    assert file_, 'Missing required attribute "file"'
    try:
        total, failed = 0, 0
        results = xmlio.Fragment()
        for path in glob(ctxt.resolve(file_)):
            fileobj = file(path, 'r')
            try:
                for suite, testcases in _get_cases(fileobj):
                    for testcase in testcases:
                        test = xmlio.Element('test')
                        test.attr['fixture'] = suite.attr['name']
                        if 'time' in testcase.attr:
                            test.attr['duration'] = testcase.attr['time']
                        if testcase.attr['executed'] == 'True':
                            if testcase.attr['success'] != 'True':
                                test.attr['status'] = 'failure'
                                failure = list(testcase.children('failure'))
                                if failure:
                                    stacktraceNode = list(failure[0].children('stack-trace'))
                                    if stacktraceNode:
                                        test.append(xmlio.Element('traceback')[
                                            stacktraceNode[0].gettext()
                                        ])
                                    failed += 1
                            else:
                                test.attr['status'] = 'success'
                        else:
                            test.attr['status'] = 'ignore'

                        results.append(test)
                        total += 1
            finally:
                fileobj.close()
        if failed:
            ctxt.error('%d of %d test%s failed' % (failed, total,
                       total != 1 and 's' or ''))
        ctxt.report('test', results)
    except IOError, e:
        log.warning('Error opening NUnit results file (%s)', e)
    except xmlio.ParseError, e:
        log.warning('Error parsing NUnit results file (%s)', e)


