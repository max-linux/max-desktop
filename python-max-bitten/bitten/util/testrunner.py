# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2008 Matt Good <matt@matt-good.net>
# Copyright (C) 2008 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

from distutils import log
from distutils.errors import DistutilsOptionError
import os
import re
from StringIO import StringIO
import sys
import time
from pkg_resources import Distribution, EntryPoint, PathMetadata, \
                          normalize_path, require, working_set
from setuptools.command.test import test
from unittest import _TextTestResult, TextTestRunner

from bitten import __version__ as VERSION
from bitten.util import xmlio

__docformat__ = 'restructuredtext en'


class XMLTestResult(_TextTestResult):

    def __init__(self, stream, descriptions, verbosity):
        _TextTestResult.__init__(self, stream, descriptions, verbosity)
        self.tests = []

    def startTest(self, test):
        _TextTestResult.startTest(self, test)
        filename = sys.modules[test.__module__].__file__
        if filename.endswith('.pyc') or filename.endswith('.pyo'):
            filename = filename[:-1]
        self.tests.append([test, filename, time.time(), None, None])

    def stopTest(self, test):
        self.tests[-1][2] = time.time() - self.tests[-1][2]
        _TextTestResult.stopTest(self, test)


class XMLTestRunner(TextTestRunner):

    def __init__(self, stream=sys.stdout, xml_stream=None):
        TextTestRunner.__init__(self, stream, descriptions=0, verbosity=2)
        self.xml_stream = xml_stream

    def _makeResult(self):
        return XMLTestResult(self.stream, self.descriptions, self.verbosity)

    def run(self, test):
        result = TextTestRunner.run(self, test)
        if not self.xml_stream:
            return result

        root = xmlio.Element('unittest-results')
        for testcase, filename, timetaken, stdout, stderr in result.tests:
            status = 'success'
            tb = None

            if testcase in [e[0] for e in result.errors]:
                status = 'error'
                tb = [e[1] for e in result.errors if e[0] is testcase][0]
            elif testcase in [f[0] for f in result.failures]:
                status = 'failure'
                tb = [f[1] for f in result.failures if f[0] is testcase][0]

            name = str(testcase)
            fixture = None
            description = testcase.shortDescription() or ''
            if description.startswith('doctest of '):
                name = 'doctest'
                fixture = description[11:]
                description = None
            else:
                match = re.match('(\w+)\s+\(([\w.]+)\)', name)
                if match:
                    name = match.group(1)
                    fixture = match.group(2)

            test_elem = xmlio.Element('test', file=filename, name=name,
                                      fixture=fixture, status=status,
                                      duration=timetaken)
            if description:
                test_elem.append(xmlio.Element('description')[description])
            if stdout:
                test_elem.append(xmlio.Element('stdout')[stdout])
            if stderr:
                test_elem.append(xmlio.Element('stdout')[stderr])
            if tb:         
                test_elem.append(xmlio.Element('traceback')[tb])
            root.append(test_elem)

        root.write(self.xml_stream, newlines=True)
        return result


class unittest(test):
    description = test.description + ', and optionally record code coverage'

    user_options = test.user_options + [
        ('xml-output=', None,
            "Path to the XML file where test results are written to"),
        ('coverage-dir=', None,
            "Directory where coverage files are to be stored"),
        ('coverage-summary=', None,
            "Path to the file where the coverage summary should be stored"),
        ('coverage-method=', None,
            "Whether to use trace.py or coverage.py to collect code coverage. "
            "Valid options are 'trace' (the default) or 'coverage'.")
    ]

    def initialize_options(self):
        test.initialize_options(self)
        self.xml_output = None
        self.xml_output_file = None
        self.coverage_summary = None
        self.coverage_dir = None
        self.coverage_method = 'trace'

    def finalize_options(self):
        test.finalize_options(self)

        if self.xml_output is not None:
            output_dir = os.path.dirname(self.xml_output) or '.'
            if not os.path.exists(output_dir):
                os.makedirs(output_dir)
            self.xml_output_file = open(self.xml_output, 'w')

        if self.coverage_method not in ('trace', 'coverage', 'figleaf'):
            raise DistutilsOptionError('Unknown coverage method %r' %
                                       self.coverage_method)

    def run_tests(self):
        if self.coverage_summary:
            if self.coverage_method == 'coverage':
                self._run_with_coverage()
            elif self.coverage_method == 'figleaf':
                self._run_with_figleaf()
            else:
                self._run_with_trace()
        else:
            self._run_tests()

    def _run_with_figleaf(self):
        import figleaf
        figleaf.start()
        try:
            self._run_tests()
        finally:
            figleaf.stop()
            figleaf.write_coverage(self.coverage_summary)

    def _run_with_coverage(self):
        import coverage
        coverage.use_cache(False)
        coverage.start()
        try:
            self._run_tests()
        finally:
            coverage.stop()

            modules = [m for _, m in sys.modules.items()
                       if m is not None and hasattr(m, '__file__')
                       and os.path.splitext(m.__file__)[-1] in ('.py', '.pyc')]

            # Generate summary file
            buf = StringIO()
            coverage.report(modules, file=buf)
            buf.seek(0)
            fileobj = open(self.coverage_summary, 'w')
            try:
                filter_coverage(buf, fileobj)
            finally:
                fileobj.close()

            if self.coverage_dir:
                if not os.path.exists(self.coverage_dir):
                    os.makedirs(self.coverage_dir)
                coverage.annotate(modules, directory=self.coverage_dir,
                                  ignore_errors=True)

    def _run_with_trace(self):
        from trace import Trace
        trace = Trace(ignoredirs=[sys.prefix, sys.exec_prefix], trace=False,
                      count=True)
        try:
            trace.runfunc(self._run_tests)
        finally:
            results = trace.results()
            real_stdout = sys.stdout
            sys.stdout = open(self.coverage_summary, 'w')
            try:
                results.write_results(show_missing=True, summary=True,
                                      coverdir=self.coverage_dir)
            finally:
                sys.stdout.close()
                sys.stdout = real_stdout

    def _run_tests(self):
        old_path = sys.path[:]
        ei_cmd = self.get_finalized_command("egg_info")
        path_item = normalize_path(ei_cmd.egg_base)
        metadata = PathMetadata(
            path_item, normalize_path(ei_cmd.egg_info)
        )
        dist = Distribution(path_item, metadata, project_name=ei_cmd.egg_name)
        working_set.add(dist)
        require(str(dist.as_requirement()))
        loader_ep = EntryPoint.parse("x=" + self.test_loader)
        loader_class = loader_ep.load(require=False)

        try:
            import unittest
            unittest.main(
                None, None, [unittest.__file__] + self.test_args,
                testRunner=XMLTestRunner(stream=sys.stdout,
                                         xml_stream=self.xml_output_file),
                testLoader=loader_class()
            )
        except SystemExit, e:
            return e.code


def filter_coverage(infile, outfile):
    for idx, line in enumerate(infile):
        if idx < 2 or line.startswith('--'):
            outfile.write(line)
            continue
        parts = line.split()
        name = parts[0]
        if name == 'TOTAL':
            continue
        if name not in sys.modules:
            outfile.write(line)
            continue
        filename = os.path.normpath(sys.modules[name].__file__)
        if filename.endswith('.pyc') or filename.endswith('.pyo'):
            filename = filename[:-1]
        outfile.write(line.rstrip() + ' ' + filename + '\n')


def main():
    from distutils.dist import Distribution
    from optparse import OptionParser

    parser = OptionParser(usage='usage: %prog [options] test_suite ...',
                          version='%%prog %s' % VERSION)
    parser.add_option('-o', '--xml-output', action='store', dest='xml_output',
                      metavar='FILE', help='write XML test results to FILE')
    parser.add_option('-d', '--coverage-dir', action='store',
                      dest='coverage_dir', metavar='DIR',
                      help='store coverage results in DIR')
    parser.add_option('-s', '--coverage-summary', action='store',
                      dest='coverage_summary', metavar='FILE',
                      help='write coverage summary to FILE')
    options, args = parser.parse_args()
    if len(args) < 1:
        parser.error('incorrect number of arguments')

    cmd = unittest(Distribution())
    cmd.initialize_options()
    cmd.test_suite = args[0]
    if hasattr(options, 'xml_output'):
        cmd.xml_output = options.xml_output
    if hasattr(options, 'coverage_summary'):
        cmd.coverage_summary = options.coverage_summary
    if hasattr(options, 'coverage_dir'):
        cmd.coverage_dir = options.coverage_dir
    cmd.finalize_options()
    cmd.run()

if __name__ == '__main__':
    main()
