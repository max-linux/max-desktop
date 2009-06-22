# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Generic recipe commands for executing external processes."""

import logging
import os
import shlex

from bitten.build import CommandLine
from bitten.util import xmlio

log = logging.getLogger('bitten.build.shtools')

__docformat__ = 'restructuredtext en'

def exec_(ctxt, executable=None, file_=None, output=None, args=None, dir_=None):
    """Execute a program or shell script.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param executable: name of the executable to run
    :param file\_: name of the script file, relative to the project directory,
                  that should be run
    :param output: name of the file to which the output of the script should be
                   written
    :param args: command-line arguments to pass to the script
    """
    assert executable or file_, \
        'Either "executable" or "file" attribute required'

    returncode = execute(ctxt, executable=executable, file_=file_,
                         output=output, args=args, dir_=dir_)
    if returncode != 0:
        ctxt.error('Executing %s failed (error code %s)' % (executable or file_,
                                                            returncode))

def pipe(ctxt, executable=None, file_=None, input_=None, output=None,
         args=None, dir_=None):
    """Pipe the contents of a file through a program or shell script.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param executable: name of the executable to run
    :param file\_: name of the script file, relative to the project directory,
                  that should be run
    :param input\_: name of the file containing the data that should be passed
                   to the shell script on its standard input stream
    :param output: name of the file to which the output of the script should be
                   written
    :param args: command-line arguments to pass to the script
    """
    assert executable or file_, \
        'Either "executable" or "file" attribute required'
    assert input_, 'Missing required attribute "input"'

    returncode = execute(ctxt, executable=executable, file_=file_,
                         input_=input_, output=output, args=args, dir_=dir_)
    if returncode != 0:
        ctxt.error('Piping through %s failed (error code %s)'
                   % (executable or file_, returncode))

def execute(ctxt, executable=None, file_=None, input_=None, output=None,
            args=None, dir_=None, filter_=None):
    """Generic external program execution.
    
    This function is not itself bound to a recipe command, but rather used from
    other commands.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param executable: name of the executable to run
    :param file\_: name of the script file, relative to the project directory,
                  that should be run
    :param input\_: name of the file containing the data that should be passed
                   to the shell script on its standard input stream
    :param output: name of the file to which the output of the script should be
                   written
    :param args: command-line arguments to pass to the script
    :param dirs:
    :param filter\_: function to filter out messages from the executable stdout
    """
    if args:
        if isinstance(args, basestring):
            args = shlex.split(args)
    else:
        args = []

    if dir_:
        def resolve(*args):
            return ctxt.resolve(dir_, *args)
    else:
        resolve = ctxt.resolve

    if file_ and os.path.isfile(resolve(file_)):
        file_ = resolve(file_)

    if executable is None:
        executable = file_
    elif file_:
        args[:0] = [file_]

    if input_:
        input_file = file(resolve(input_), 'r')
    else:
        input_file = None

    if output:
        output_file = file(resolve(output), 'w')
    else:
        output_file = None

    if dir_ and os.path.isdir(ctxt.resolve(dir_)):
        dir_ = ctxt.resolve(dir_)
    else:
        dir_ = ctxt.basedir
        
    if not filter_:
        filter_=lambda s: s

    try:
        cmdline = CommandLine(executable, args, input=input_file,
                              cwd=dir_)
        log_elem = xmlio.Fragment()
        for out, err in cmdline.execute():
            if out is not None:
                log.info(out)
                info = filter_(out)
                if info:
                    log_elem.append(xmlio.Element('message', level='info')[
                        info.replace(ctxt.basedir + os.sep, '')
                            .replace(ctxt.basedir, '')
                ])
                if output:
                    output_file.write(out + os.linesep)
            if err is not None:
                log.error(err)
                log_elem.append(xmlio.Element('message', level='error')[
                    err.replace(ctxt.basedir + os.sep, '')
                       .replace(ctxt.basedir, '')
                ])
                if output:
                    output_file.write(err + os.linesep)
        ctxt.log(log_elem)
    finally:
        if input_:
            input_file.close()
        if output:
            output_file.close()

    return cmdline.returncode
