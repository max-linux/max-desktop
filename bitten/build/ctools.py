# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Recipe commands for build tasks commonly used for C/C++ projects."""

import logging
import re
import os
import posixpath
import shlex

from bitten.build import CommandLine, FileSet
from bitten.util import xmlio

log = logging.getLogger('bitten.build.ctools')

__docformat__ = 'restructuredtext en'

def configure(ctxt, file_='configure', enable=None, disable=None, with_=None,
              without=None, cflags=None, cxxflags=None, prefix=None, **kw):
    """Run a ``configure`` script.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param file\_: name of the configure script
    :param enable: names of the features to enable, seperated by spaces
    :param disable: names of the features to disable, separated by spaces
    :param with_: names of external packages to include
    :param without: names of external packages to exclude
    :param cflags: ``CFLAGS`` to pass to the configure script
    :param cxxflags: ``CXXFLAGS`` to pass to the configure script
    :param prefix: install prefix to pass to the configure script, will be postfixed by the machine name from the build
    """
    args = []
    if enable:
        args += ['--enable-%s' % feature for feature in enable.split()]
    if disable:
        args += ['--disable-%s' % feature for feature in disable.split()]
    # since 'with' is a reserved word in python, we need to handle the argument carefully
    with_ = kw.pop('with', with_)
    for key in kw:
        raise TypeError("configure() got an unexpected keyword argument '%s'" % key)
    if with_:
        for pkg in with_.split():
            pkg_path = pkg + '.path'
            if pkg_path in ctxt.config:
                args.append('--with-%s=%s' % (pkg, ctxt.config[pkg_path]))
            else:
                args.append('--with-%s' % pkg)
    if without:
        args += ['--without-%s' % pkg for pkg in without.split()]
    if cflags:
        args.append('CFLAGS=%s' % cflags)
    if cxxflags:
        args.append('CXXFLAGS=%s' % cxxflags)
    if prefix:
        args.append('--prefix=%ss' % prefix)

    from bitten.build import shtools
    returncode = shtools.execute(ctxt, file_=file_, args=args)
    if returncode != 0:
        ctxt.error('configure failed (%s)' % returncode)

def autoreconf(ctxt, file_='configure', force=None, install=None, symlink=None,
              warnings=None, prepend_include=None, include =None):
    """Run the autotoll ``autoreconf``.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param force: consider all files obsolete
    :param install: copy missing auxiliary files
    :param symlink: install symbolic links instead of copies
    :param warnings: report the warnings falling in CATEGORY
    :prepend_include: prepend directories to search path
    :include: append directories to search path

    """
    args = []
    if install:
        args.append('--install')
        if symlink:
            args.append('--symlink')
    if force:
        args.append('--force')
    if warnings:
        args.append('--warnings=%s' % warnings)
        
    if include:
        args += ['--include=%s' % inc for inc in include.split()]
    if prepend_include:
        args += ['--prepend-include=%s' % pinc for pinc in prepend_include.split()]
            
    from bitten.build import shtools
    returncode = shtools.execute(ctxt, 'autoreconf', args=args)
    if returncode != 0:
        ctxt.error('autoreconf failed (%s)' % returncode)

def make(ctxt, target=None, file_=None, keep_going=False, directory=None, jobs=None, args=None):
    """Execute a Makefile target.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param file\_: name of the Makefile
    :param keep_going: whether make should keep going when errors are
                       encountered
    :param directory: directory in which to build; defaults to project source directory
    :param jobs: number of concurrent jobs to run
    :param args: command-line arguments to pass to the script
    """
    executable = ctxt.config.get_filepath('make.path') or 'make'

    if directory is None:
        directory = ctxt.basedir

    margs = ['--directory', directory]

    if file_:
        margs += ['--file', ctxt.resolve(file_)]
    if keep_going:
        margs.append('--keep-going')
    if target:
        margs.append(target)
    if jobs:
        margs += ['--jobs', jobs]

    if args:
        if isinstance(args, basestring):
            margs += shlex.split(args)

    from bitten.build import shtools
    returncode = shtools.execute(ctxt, executable=executable, args=margs)
    if returncode != 0:
        ctxt.error('make failed (%s)' % returncode)

def cppunit(ctxt, file_=None, srcdir=None):
    """Collect CppUnit XML data.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param file\_: path of the file containing the CppUnit results; may contain
                  globbing wildcards to match multiple files
    :param srcdir: name of the directory containing the source files, used to
                   link the test results to the corresponding files
    """
    assert file_, 'Missing required attribute "file"'

    try:
        fileobj = file(ctxt.resolve(file_), 'r')
        try:
            total, failed = 0, 0
            results = xmlio.Fragment()
            for group in xmlio.parse(fileobj):
                if group.name not in ('FailedTests', 'SuccessfulTests'):
                    continue
                for child in group.children():
                    test = xmlio.Element('test')
                    name = child.children('Name').next().gettext()
                    if '::' in name:
                        parts = name.split('::')
                        test.attr['fixture'] = '::'.join(parts[:-1])
                        name = parts[-1]
                    test.attr['name'] = name

                    for location in child.children('Location'):
                        for file_elem in location.children('File'):
                            filepath = file_elem.gettext()
                            if srcdir is not None:
                                filepath = posixpath.join(srcdir, filepath)
                            test.attr['file'] = filepath
                            break
                        for line_elem in location.children('Line'):
                            test.attr['line'] = line_elem.gettext()
                            break
                        break

                    if child.name == 'FailedTest':
                        for message in child.children('Message'):
                            test.append(xmlio.Element('traceback')[
                                message.gettext()
                            ])
                        test.attr['status'] = 'failure'
                        failed += 1
                    else:
                        test.attr['status'] = 'success'

                    results.append(test)
                    total += 1

            if failed:
                ctxt.error('%d of %d test%s failed' % (failed, total,
                           total != 1 and 's' or ''))

            ctxt.report('test', results)

        finally:
            fileobj.close()

    except IOError, e:
        log.warning('Error opening CppUnit results file (%s)', e)
    except xmlio.ParseError, e:
        print e
        log.warning('Error parsing CppUnit results file (%s)', e)

def cunit (ctxt, file_=None, srcdir=None):
    """Collect CUnit XML data.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param file\_: path of the file containing the CUnit results; may contain
                  globbing wildcards to match multiple files
    :param srcdir: name of the directory containing the source files, used to
                   link the test results to the corresponding files
    """
    assert file_, 'Missing required attribute "file"'

    try:
        fileobj = file(ctxt.resolve(file_), 'r')
        try:
            total, failed = 0, 0
            results = xmlio.Fragment()
            log_elem = xmlio.Fragment()
            def info (msg):
                log.info (msg)
                log_elem.append (xmlio.Element ('message', level='info')[msg])
            def warning (msg):
                log.warning (msg)
                log_elem.append (xmlio.Element ('message', level='warning')[msg])
            def error (msg):
                log.error (msg)
                log_elem.append (xmlio.Element ('message', level='error')[msg])
            for node in xmlio.parse(fileobj):
                if node.name != 'CUNIT_RESULT_LISTING':
                    continue
                for suiteRun in node.children ('CUNIT_RUN_SUITE'):
                    for suite in suiteRun.children():
                        if suite.name not in ('CUNIT_RUN_SUITE_SUCCESS', 'CUNIT_RUN_SUITE_FAILURE'):
                            warning ("Unknown node: %s" % suite.name)
                            continue
                        suiteName = suite.children ('SUITE_NAME').next().gettext()
                        info ("%s [%s]" % ("*" * (57 - len (suiteName)), suiteName))
                        for record in suite.children ('CUNIT_RUN_TEST_RECORD'):
                            for result in record.children():
                                if result.name not in ('CUNIT_RUN_TEST_SUCCESS', 'CUNIT_RUN_TEST_FAILURE'):
                                    continue
                                testName = result.children ('TEST_NAME').next().gettext()
                                info ("Running %s..." % testName);
                                test = xmlio.Element('test')
                                test.attr['fixture'] = suiteName
                                test.attr['name'] = testName
                                if result.name == 'CUNIT_RUN_TEST_FAILURE':
                                    error ("%s(%d): %s"
                                               % (result.children ('FILE_NAME').next().gettext(),
                                                  int (result.children ('LINE_NUMBER').next().gettext()),
                                                  result.children ('CONDITION').next().gettext()))
                                    test.attr['status'] = 'failure'
                                    failed += 1
                                else:
                                    test.attr['status'] = 'success'

                                results.append(test)
                                total += 1

            if failed:
                ctxt.error('%d of %d test%s failed' % (failed, total,
                           total != 1 and 's' or ''))

            ctxt.report('test', results)
            ctxt.log (log_elem)

        finally:
            fileobj.close()

    except IOError, e:
        log.warning('Error opening CUnit results file (%s)', e)
    except xmlio.ParseError, e:
        print e
        log.warning('Error parsing CUnit results file (%s)', e)

def gcov(ctxt, include=None, exclude=None, prefix=None, root=""):
    """Run ``gcov`` to extract coverage data where available.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param include: patterns of files and directories to include
    :param exclude: patterns of files and directories that should be excluded
    :param prefix: optional prefix name that is added to object files by the
                   build system
    :param root: optional root path in which the build system puts the object
                 files
    """
    file_re = re.compile(r'^File (?:\'|\`)(?P<file>[^\']+)\'\s*$')
    lines_re = re.compile(r'^Lines executed:(?P<cov>\d+\.\d+)\% of (?P<num>\d+)\s*$')

    files = []
    for filename in FileSet(ctxt.basedir, include, exclude):
        if os.path.splitext(filename)[1] in ('.c', '.cpp', '.cc', '.cxx'):
            files.append(filename)

    coverage = xmlio.Fragment()
    log_elem = xmlio.Fragment()
    def info (msg):
        log.info (msg)
        log_elem.append (xmlio.Element ('message', level='info')[msg])
    def warning (msg):
        log.warning (msg)
        log_elem.append (xmlio.Element ('message', level='warning')[msg])
    def error (msg):
        log.error (msg)
        log_elem.append (xmlio.Element ('message', level='error')[msg])

    for srcfile in files:
        # Determine the coverage for each source file by looking for a .gcno
        # and .gcda pair
        info ("Getting coverage info for %s" % srcfile)
        filepath, filename = os.path.split(srcfile)
        stem = os.path.splitext(filename)[0]
        if prefix is not None:
            stem = prefix + '-' + stem

        objfile = os.path.join (root, filepath, stem + '.o')
        if not os.path.isfile(ctxt.resolve(objfile)):
            warning ('No object file found for %s at %s' % (srcfile, objfile))
            continue
        if not os.path.isfile (ctxt.resolve (os.path.join (root, filepath, stem + '.gcno'))):
            warning ('No .gcno file found for %s at %s' % (srcfile, os.path.join (root, filepath, stem + '.gcno')))
            continue
        if not os.path.isfile (ctxt.resolve (os.path.join (root, filepath, stem + '.gcda'))):
            warning ('No .gcda file found for %s at %s' % (srcfile, os.path.join (root, filepath, stem + '.gcda')))
            continue

        num_lines, num_covered = 0, 0
        skip_block = False
        cmd = CommandLine('gcov', ['-b', '-n', '-o', objfile, srcfile],
                          cwd=ctxt.basedir)
        for out, err in cmd.execute():
            if out == '': # catch blank lines, reset the block state...
                skip_block = False
            elif out and not skip_block:
                # Check for a file name
                match = file_re.match(out)
                if match:
                    if os.path.isabs(match.group('file')):
                        skip_block = True
                        continue
                else:
                    # check for a "Lines executed" message
                    match = lines_re.match(out)
                    if match:
                        lines = float(match.group('num'))
                        cov = float(match.group('cov'))
                        num_covered += int(lines * cov / 100)
                        num_lines += int(lines)
        if cmd.returncode != 0:
            continue

        module = xmlio.Element('coverage', name=os.path.basename(srcfile),
                                file=srcfile.replace(os.sep, '/'),
                                lines=num_lines, percentage=0)
        if num_lines:
            percent = int(round(num_covered * 100 / num_lines))
            module.attr['percentage'] = percent
        coverage.append(module)

    ctxt.report('coverage', coverage)
    ctxt.log (log_elem)
