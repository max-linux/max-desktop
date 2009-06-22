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

"""Recipe commands for tools commonly used by Python projects."""

from __future__ import division

import logging
import os
import cPickle as pickle
import re
try:
    set
except NameError:
    from sets import Set as set
import shlex
import sys

from bitten.build import CommandLine, FileSet
from bitten.util import loc, xmlio

log = logging.getLogger('bitten.build.pythontools')

__docformat__ = 'restructuredtext en'

def _python_path(ctxt):
    """Return the path to the Python interpreter.
    
    If the configuration has a ``python.path`` property, the value of that
    option is returned; otherwise the path to the current Python interpreter is
    returned.
    """
    python_path = ctxt.config.get_filepath('python.path')
    if python_path:
        return python_path
    return sys.executable

def distutils(ctxt, file_='setup.py', command='build', options=None):
    """Execute a ``distutils`` command.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param file\_: name of the file defining the distutils setup
    :param command: the setup command to execute
    :param options: additional options to pass to the command
    """
    if options:
        if isinstance(options, basestring):
            options = shlex.split(options)
    else:
        options = []

    cmdline = CommandLine(_python_path(ctxt),
                          [ctxt.resolve(file_), command] + options,
                          cwd=ctxt.basedir)
    log_elem = xmlio.Fragment()
    error_logged = False
    for out, err in cmdline.execute():
        if out is not None:
            log.info(out)
            log_elem.append(xmlio.Element('message', level='info')[out])
        if err is not None:
            level = 'error'
            if err.startswith('warning: '):
                err = err[9:]
                level = 'warning'
                log.warning(err)
            elif err.startswith('error: '):
                ctxt.error(err[7:])
                error_logged = True
            else:
                log.error(err)
            log_elem.append(xmlio.Element('message', level=level)[err])
    ctxt.log(log_elem)

    if not error_logged and cmdline.returncode != 0:
        ctxt.error('distutils failed (%s)' % cmdline.returncode)

def exec_(ctxt, file_=None, module=None, function=None, output=None, args=None):
    """Execute a Python script.
    
    Either the `file_` or the `module` parameter must be provided. If
    specified using the `file_` parameter, the file must be inside the project
    directory. If specified as a module, the module must either be resolvable
    to a file, or the `function` parameter must be provided
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param file\_: name of the script file to execute
    :param module: name of the Python module to execute
    :param function: name of the Python function to run
    :param output: name of the file to which output should be written
    :param args: extra arguments to pass to the script
    """
    assert file_ or module, 'Either "file" or "module" attribute required'
    if function:
        assert module and not file_, '"module" attribute required for use of ' \
                                     '"function" attribute'

    if module:
        # Script specified as module name, need to resolve that to a file,
        # or use the function name if provided
        if function:
            args = '-c "import sys; from %s import %s; %s(sys.argv)" %s' % (
                   module, function, function, args)
        else:
            try:
                mod = __import__(module, globals(), locals(), [])
                components = module.split('.')
                for comp in components[1:]:
                    mod = getattr(mod, comp)
                file_ = mod.__file__.replace('\\', '/')
            except ImportError, e:
                ctxt.error('Cannot execute Python module %s: %s' % (module, e))
                return

    from bitten.build import shtools
    returncode = shtools.execute(ctxt, executable=_python_path(ctxt),
                                 file_=file_, output=output, args=args)
    if returncode != 0:
        ctxt.error('Executing %s failed (error code %s)' % (file_, returncode))

def pylint(ctxt, file_=None):
    """Extract data from a ``pylint`` run written to a file.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param file\_: name of the file containing the Pylint output
    """
    assert file_, 'Missing required attribute "file"'
    msg_re = re.compile(r'^(?P<file>.+):(?P<line>\d+): '
                        r'\[(?P<type>[A-Z]\d*)(?:, (?P<tag>[\w\.]+))?\] '
                        r'(?P<msg>.*)$')
    msg_categories = dict(W='warning', E='error', C='convention', R='refactor')

    problems = xmlio.Fragment()
    try:
        fd = open(ctxt.resolve(file_), 'r')
        try:
            for line in fd:
                match = msg_re.search(line)
                if match:
                    msg_type = match.group('type')
                    category = msg_categories.get(msg_type[0])
                    if len(msg_type) == 1:
                        msg_type = None
                    filename = os.path.realpath(match.group('file'))
                    if filename.startswith(ctxt.basedir):
                        filename = filename[len(ctxt.basedir) + 1:]
                    filename = filename.replace(os.sep, '/')
                    lineno = int(match.group('line'))
                    tag = match.group('tag')
                    problems.append(xmlio.Element('problem', category=category,
                                                  type=msg_type, tag=tag,
                                                  line=lineno, file=filename)[
                        match.group('msg') or ''
                    ])
            ctxt.report('lint', problems)
        finally:
            fd.close()
    except IOError, e:
        log.warning('Error opening pylint results file (%s)', e)

def coverage(ctxt, summary=None, coverdir=None, include=None, exclude=None):
    """Extract data from a ``coverage.py`` run.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param summary: path to the file containing the coverage summary
    :param coverdir: name of the directory containing the per-module coverage
                     details
    :param include: patterns of files or directories to include in the report
    :param exclude: patterns of files or directories to exclude from the report
    """
    assert summary, 'Missing required attribute "summary"'

    summary_line_re = re.compile(r'^(?P<module>.*?)\s+(?P<stmts>\d+)\s+'
                                 r'(?P<exec>\d+)\s+(?P<cov>\d+)%\s+'
                                 r'(?:(?P<missing>(?:\d+(?:-\d+)?(?:, )?)*)\s+)?'
                                 r'(?P<file>.+)$')

    fileset = FileSet(ctxt.basedir, include, exclude)
    missing_files = []
    for filename in fileset:
        if os.path.splitext(filename)[1] != '.py':
            continue
        missing_files.append(filename)
    covered_modules = set()

    try:
        summary_file = open(ctxt.resolve(summary), 'r')
        try:
            coverage = xmlio.Fragment()
            for summary_line in summary_file:
                match = summary_line_re.search(summary_line)
                if match:
                    modname = match.group(1)
                    filename = match.group(6)
                    if not os.path.isabs(filename):
                        filename = os.path.normpath(os.path.join(ctxt.basedir,
                                                                 filename))
                    else:
                        filename = os.path.realpath(filename)
                    if not filename.startswith(ctxt.basedir):
                        continue
                    filename = filename[len(ctxt.basedir) + 1:]
                    if not filename in fileset:
                        continue

                    percentage = int(match.group(4).rstrip('%'))
                    num_lines = int(match.group(2))

                    missing_files.remove(filename)
                    covered_modules.add(modname)
                    module = xmlio.Element('coverage', name=modname,
                                           file=filename.replace(os.sep, '/'),
                                           percentage=percentage,
                                           lines=num_lines)
                    coverage.append(module)

            for filename in missing_files:
                modname = os.path.splitext(filename.replace(os.sep, '.'))[0]
                if modname in covered_modules:
                    continue
                covered_modules.add(modname)
                module = xmlio.Element('coverage', name=modname,
                                       file=filename.replace(os.sep, '/'),
                                       percentage=0)
                coverage.append(module)

            ctxt.report('coverage', coverage)
        finally:
            summary_file.close()
    except IOError, e:
        log.warning('Error opening coverage summary file (%s)', e)

def trace(ctxt, summary=None, coverdir=None, include=None, exclude=None):
    """Extract data from a ``trace.py`` run.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param summary: path to the file containing the coverage summary
    :param coverdir: name of the directory containing the per-module coverage
                     details
    :param include: patterns of files or directories to include in the report
    :param exclude: patterns of files or directories to exclude from the report
    """
    assert summary, 'Missing required attribute "summary"'
    assert coverdir, 'Missing required attribute "coverdir"'

    summary_line_re = re.compile(r'^\s*(?P<lines>\d+)\s+(?P<cov>\d+)%\s+'
                                 r'(?P<module>.*?)\s+\((?P<filename>.*?)\)')
    coverage_line_re = re.compile(r'\s*(?:(?P<hits>\d+): )?(?P<line>.*)')

    fileset = FileSet(ctxt.basedir, include, exclude)
    missing_files = []
    for filename in fileset:
        if os.path.splitext(filename)[1] != '.py':
            continue
        missing_files.append(filename)
    covered_modules = set()

    def handle_file(elem, sourcefile, coverfile=None):
        code_lines = set()
        for lineno, linetype, line in loc.count(sourcefile):
            if linetype == loc.CODE:
                code_lines.add(lineno)
        num_covered = 0
        lines = []

        if coverfile:
            prev_hits = '0'
            for idx, coverline in enumerate(coverfile):
                match = coverage_line_re.search(coverline)
                if match:
                    hits = match.group(1)
                    if hits: # Line covered
                        if hits != '0':
                            num_covered += 1
                        lines.append(hits)
                        prev_hits = hits
                    elif coverline.startswith('>'): # Line not covered
                        lines.append('0')
                        prev_hits = '0'
                    elif idx not in code_lines: # Not a code line
                        lines.append('-')
                        prev_hits = '0'
                    else: # A code line not flagged by trace.py
                        if prev_hits != '0':
                            num_covered += 1
                        lines.append(prev_hits)

            elem.append(xmlio.Element('line_hits')[' '.join(lines)])

        num_lines = len(code_lines)
        if num_lines:
            percentage = int(round(num_covered * 100 / num_lines))
        else:
            percentage = 0
        elem.attr['percentage'] = percentage
        elem.attr['lines'] = num_lines

    try:
        summary_file = open(ctxt.resolve(summary), 'r')
        try:
            coverage = xmlio.Fragment()
            for summary_line in summary_file:
                match = summary_line_re.search(summary_line)
                if match:
                    modname = match.group(3)
                    filename = match.group(4)
                    if not os.path.isabs(filename):
                        filename = os.path.normpath(os.path.join(ctxt.basedir,
                                                                 filename))
                    else:
                        filename = os.path.realpath(filename)
                    if not filename.startswith(ctxt.basedir):
                        continue
                    filename = filename[len(ctxt.basedir) + 1:]
                    if not filename in fileset:
                        continue

                    missing_files.remove(filename)
                    covered_modules.add(modname)
                    module = xmlio.Element('coverage', name=modname,
                                           file=filename.replace(os.sep, '/'))
                    sourcefile = file(ctxt.resolve(filename))
                    try:
                        coverpath = ctxt.resolve(coverdir, modname + '.cover')
                        if os.path.isfile(coverpath):
                            coverfile = file(coverpath, 'r')
                        else:
                            log.warning('No coverage file for module %s at %s',
                                        modname, coverpath)
                            coverfile = None
                        try:
                            handle_file(module, sourcefile, coverfile)
                        finally:
                            if coverfile:
                                coverfile.close()
                    finally:
                        sourcefile.close()
                    coverage.append(module)

            for filename in missing_files:
                modname = os.path.splitext(filename.replace(os.sep, '.'))[0]
                if modname in covered_modules:
                    continue
                covered_modules.add(modname)
                module = xmlio.Element('coverage', name=modname,
                                       file=filename.replace(os.sep, '/'),
                                       percentage=0)
                filepath = ctxt.resolve(filename)
                fileobj = file(filepath, 'r')
                try:
                    handle_file(module, fileobj)
                finally:
                    fileobj.close()
                coverage.append(module)

            ctxt.report('coverage', coverage)
        finally:
            summary_file.close()
    except IOError, e:
        log.warning('Error opening coverage summary file (%s)', e)

def figleaf(ctxt, summary=None, include=None, exclude=None):
    from figleaf import get_lines
    coverage = xmlio.Fragment()
    try:
        fileobj = open(ctxt.resolve(summary))
    except IOError, e:
        log.warning('Error opening coverage summary file (%s)', e)
        return
    coverage_data = pickle.load(fileobj)
    fileset = FileSet(ctxt.basedir, include, exclude)
    for filename in fileset:
        base, ext = os.path.splitext(filename)
        if ext != '.py':
            continue
        modname = base.replace(os.path.sep, '.')
        realfilename = ctxt.resolve(filename)
        interesting_lines = get_lines(open(realfilename))
        covered_lines = coverage_data.get(realfilename, set())
        percentage = int(round(len(covered_lines) * 100 / len(interesting_lines)))
        line_hits = []
        for lineno in xrange(1, max(interesting_lines)+1):
            if lineno not in interesting_lines:
                line_hits.append('-')
            elif lineno in covered_lines:
                line_hits.append('1')
            else:
                line_hits.append('0')
        module = xmlio.Element('coverage', name=modname,
                               file=filename,
                               percentage=percentage,
                               lines=len(interesting_lines),
                               line_hits=' '.join(line_hits))
        coverage.append(module)
    ctxt.report('coverage', coverage)

def _normalize_filenames(ctxt, filenames, fileset):
    for filename in filenames:
        if not os.path.isabs(filename):
            filename = os.path.normpath(os.path.join(ctxt.basedir,
                                                     filename))
        else:
            filename = os.path.realpath(filename)
        if not filename.startswith(ctxt.basedir):
            continue
        filename = filename[len(ctxt.basedir) + 1:]
        if filename not in fileset:
            continue
        yield filename.replace(os.sep, '/')

def unittest(ctxt, file_=None):
    """Extract data from a unittest results file in XML format.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param file\_: name of the file containing the test results
    """
    assert file_, 'Missing required attribute "file"'

    try:
        fileobj = file(ctxt.resolve(file_), 'r')
        try:
            total, failed = 0, 0
            results = xmlio.Fragment()
            for child in xmlio.parse(fileobj).children():
                test = xmlio.Element('test')
                for name, value in child.attr.items():
                    if name == 'file':
                        value = os.path.realpath(value)
                        if value.startswith(ctxt.basedir):
                            value = value[len(ctxt.basedir) + 1:]
                            value = value.replace(os.sep, '/')
                        else:
                            continue
                    test.attr[name] = value
                    if name == 'status' and value in ('error', 'failure'):
                        failed += 1
                for grandchild in child.children():
                    test.append(xmlio.Element(grandchild.name)[
                        grandchild.gettext()
                    ])
                results.append(test)
                total += 1
            if failed:
                ctxt.error('%d of %d test%s failed' % (failed, total,
                           total != 1 and 's' or ''))
            ctxt.report('test', results)
        finally:
            fileobj.close()
    except IOError, e:
        log.warning('Error opening unittest results file (%s)', e)
    except xmlio.ParseError, e:
        log.warning('Error parsing unittest results file (%s)', e)
