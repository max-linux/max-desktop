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

"""Recipe commands for tools commonly used in Java projects."""

from glob import glob
import logging
import os
import posixpath
import shlex
import tempfile

from bitten.build import CommandLine
from bitten.util import xmlio

log = logging.getLogger('bitten.build.javatools')

__docformat__ = 'restructuredtext en'

def ant(ctxt, file_=None, target=None, keep_going=False, args=None):
    """Run an Ant build.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param file\_: name of the Ant build file
    :param target: name of the target that should be executed (optional)
    :param keep_going: whether Ant should keep going when errors are encountered
    :param args: additional arguments to pass to Ant
    """
    executable = 'ant'
    ant_home = ctxt.config.get_dirpath('ant.home')
    if ant_home:
        executable = os.path.join(ant_home, 'bin', 'ant')

    java_home = ctxt.config.get_dirpath('java.home')
    if java_home:
        os.environ['JAVA_HOME'] = java_home

    logfile = tempfile.NamedTemporaryFile(prefix='ant_log', suffix='.xml')
    logfile.close()
    if args:
        args = shlex.split(args)
    else:
        args = []
    args += ['-noinput', '-listener', 'org.apache.tools.ant.XmlLogger',
             '-Dant.XmlLogger.stylesheet.uri', '""',
             '-DXmlLogger.file', logfile.name]
    if file_:
        args += ['-buildfile', ctxt.resolve(file_)]
    if keep_going:
        args.append('-keep-going')
    if target:
        args.append(target)

    cmdline = CommandLine(executable, args, cwd=ctxt.basedir)
    for out, err in cmdline.execute():
        if out is not None:
            log.info(out)
        if err is not None:
            log.error(err)

    error_logged = False
    log_elem = xmlio.Fragment()
    try:
        xml_log = xmlio.parse(file(logfile.name, 'r'))
        def collect_log_messages(node):
            for child in node.children():
                if child.name == 'message':
                    if child.attr['priority'] == 'debug':
                        continue
                    log_elem.append(xmlio.Element('message',
                                                  level=child.attr['priority'])[
                        child.gettext().replace(ctxt.basedir + os.sep, '')
                                       .replace(ctxt.basedir, '')
                    ])
                else:
                    collect_log_messages(child)
        collect_log_messages(xml_log)

        if 'error' in xml_log.attr:
            ctxt.error(xml_log.attr['error'])
            error_logged = True

    except xmlio.ParseError, e:
        log.warning('Error parsing Ant XML log file (%s)', e)
    ctxt.log(log_elem)

    if not error_logged and cmdline.returncode != 0:
        ctxt.error('Ant failed (%s)' % cmdline.returncode)

def junit(ctxt, file_=None, srcdir=None):
    """Extract test results from a JUnit XML report.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param file\_: path to the JUnit XML test results; may contain globbing
                  wildcards for matching multiple results files
    :param srcdir: name of the directory containing the test sources, used to
                   link test results to the corresponding source files
    """
    assert file_, 'Missing required attribute "file"'
    try:
        total, failed = 0, 0
        results = xmlio.Fragment()
        for path in glob(ctxt.resolve(file_)):
            fileobj = file(path, 'r')
            try:
                for testcase in xmlio.parse(fileobj).children('testcase'):
                    test = xmlio.Element('test')
                    test.attr['fixture'] = testcase.attr['classname']
                    if 'time' in testcase.attr:
                        test.attr['duration'] = testcase.attr['time']
                    if srcdir is not None:
                        cls = testcase.attr['classname'].split('.')
                        test.attr['file'] = posixpath.join(srcdir, *cls) + \
                                            '.java'

                    result = list(testcase.children())
                    if result:
                        test.attr['status'] = result[0].name
                        test.append(xmlio.Element('traceback')[
                            result[0].gettext()
                        ])
                        failed += 1
                    else:
                        test.attr['status'] = 'success'

                    results.append(test)
                    total += 1
            finally:
                fileobj.close()
        if failed:
            ctxt.error('%d of %d test%s failed' % (failed, total,
                       total != 1 and 's' or ''))
        ctxt.report('test', results)
    except IOError, e:
        log.warning('Error opening JUnit results file (%s)', e)
    except xmlio.ParseError, e:
        log.warning('Error parsing JUnit results file (%s)', e)


class _LineCounter(object):
    def __init__(self):
        self.lines = []
        self.covered = 0
        self.num_lines = 0

    def __getitem__(self, idx):
        if idx >= len(self.lines):
            return 0
        return self.lines[idx]

    def __setitem__(self, idx, val):
        idx = int(idx) - 1 # 1-indexed to 0-indexed
        from itertools import repeat
        if idx >= len(self.lines):
            self.lines.extend(repeat('-', idx - len(self.lines) + 1))
        self.lines[idx] = val
        self.num_lines += 1
        if val != '0':
            self.covered += 1

    def line_hits(self):
        return ' '.join(self.lines)
    line_hits = property(line_hits)

    def percentage(self):
        if self.num_lines == 0:
            return 0
        return int(round(self.covered * 100. / self.num_lines))
    percentage = property(percentage)


def cobertura(ctxt, file_=None):
    """Extract test coverage information from a Cobertura XML report.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param file\_: path to the Cobertura XML output
    """
    assert file_, 'Missing required attribute "file"'

    coverage = xmlio.Fragment()
    doc = xmlio.parse(open(ctxt.resolve(file_)))
    srcdir = [s.gettext().strip() for ss in doc.children('sources')
                                  for s in ss.children('source')][0]

    classes = [cls for pkgs in doc.children('packages')
                   for pkg in pkgs.children('package')
                   for clss in pkg.children('classes')
                   for cls in clss.children('class')]

    counters = {}
    class_names = {}

    for cls in classes:
        filename = cls.attr['filename'].replace(os.sep, '/')
        name = cls.attr['name']
        if not '$' in name: # ignore internal classes
            class_names[filename] = name
        counter = counters.get(filename)
        if counter is None:
            counter = counters[filename] = _LineCounter()
        lines = [l for ls in cls.children('lines')
                   for l in ls.children('line')]
        for line in lines:
            counter[line.attr['number']] = line.attr['hits']

    for filename, name in class_names.iteritems():
        counter = counters[filename]
        module = xmlio.Element('coverage', name=name,
                               file=posixpath.join(srcdir, filename),
                               lines=counter.num_lines,
                               percentage=counter.percentage)
        module.append(xmlio.Element('line_hits')[counter.line_hits])
        coverage.append(module)
    ctxt.report('coverage', coverage)
