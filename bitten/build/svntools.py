# -*- coding: utf-8 -*-
#
# Copyright (C) 2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Recipe commands for Subversion."""

import logging
import posixpath
import re
import shutil
import os

log = logging.getLogger('bitten.build.svntools')

__docformat__ = 'restructuredtext en'

class Error(EnvironmentError):
    pass

def copytree(src, dst, symlinks=False):
    """Recursively copy a directory tree using copy2().

    If exception(s) occur, an Error is raised with a list of reasons.

    If the optional symlinks flag is true, symbolic links in the
    source tree result in symbolic links in the destination tree; if
    it is false, the contents of the files pointed to by symbolic
    links are copied.

    Adapted from shtuil.copytree

    """
    names = os.listdir(src)
    if not os.path.isdir(dst):
        os.makedirs(dst)
    errors = []
    for name in names:
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            if symlinks and os.path.islink(srcname):
                linkto = os.readlink(srcname)
                os.symlink(linkto, dstname)
            elif os.path.isdir(srcname):
                copytree(srcname, dstname, symlinks)
            else:
                shutil.copy2(srcname, dstname)
        except (IOError, os.error), why:
            errors.append((srcname, dstname, str(why)))
        # catch the Error from the recursive copytree so that we can
        # continue with other files
        except Error, err:
            errors.extend(err.args[0])
    try:
        shutil.copystat(src, dst)
    except WindowsError:
        # can't copy file access times on Windows
        pass
    except OSError, why:
        errors.extend((src, dst, str(why)))
    if errors:
        raise Error, errors

def checkout(ctxt, url, path=None, revision=None, dir_='.', verbose=False, shared_path=None,
        username=None, password=None):
    """Perform a checkout from a Subversion repository.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param url: the URL of the repository
    :param path: the path inside the repository
    :param revision: the revision to check out
    :param dir_: the name of a local subdirectory to check out into
    :param verbose: whether to log the list of checked out files
    :param shared_path: a shared directory to do the checkout in, before copying to dir_
    :param username: a username of the repository
    :param password: a password of the repository
    """
    args = ['checkout']
    if revision:
        args += ['-r', revision]
    if path:
        final_url = posixpath.join(url, path.lstrip('/'))
    else:
        final_url = url
    if username:
        args += ['--username', username]
    if password:
        args += ['--password', password]
    args += [final_url, dir_]

    cofilter = None
    if not verbose:
        cre = re.compile(r'^[AU]\s.*$')
        cofilter = lambda s: cre.sub('', s)
    if shared_path is not None:
        # run checkout on shared_path, then copy
        shared_path = ctxt.resolve(shared_path)
        checkout(ctxt, url, path, revision, dir_=shared_path, verbose=verbose)
        try:
            copytree(shared_path, ctxt.resolve(dir_))
        except Exception, e:
            ctxt.log('error copying shared tree (%s)' % e)
    from bitten.build import shtools
    returncode = shtools.execute(ctxt, file_='svn', args=args, 
                                 filter_=cofilter)
    if returncode != 0:
        ctxt.error('svn checkout failed (%s)' % returncode)

def export(ctxt, url, path=None, revision=None, dir_='.', username=None, password=None):
    """Perform an export from a Subversion repository.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param url: the URL of the repository
    :param path: the path inside the repository
    :param revision: the revision to check out
    :param dir_: the name of a local subdirectory to export out into
    :param username: a username of the repository
    :param password: a password of the repository
    """
    args = ['export', '--force']
    if revision:
        args += ['-r', revision]
    if path:
        url = posixpath.join(url, path)
    if username:
        args += ['--username', username]
    if password:
        args += ['--password', password]
    args += [url, dir_]

    from bitten.build import shtools
    returncode = shtools.execute(ctxt, file_='svn', args=args)
    if returncode != 0:
        ctxt.error('svn export failed (%s)' % returncode)

def update(ctxt, revision=None, dir_='.'):
    """Update the local working copy from the Subversion repository.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param revision: the revision to check out
    :param dir_: the name of a local subdirectory containing the working copy
    """
    args = ['update']
    if revision:
        args += ['-r', revision]
    args += [dir_]

    from bitten.build import shtools
    returncode = shtools.execute(ctxt, file_='svn', args=args)
    if returncode != 0:
        ctxt.error('svn update failed (%s)' % returncode)
