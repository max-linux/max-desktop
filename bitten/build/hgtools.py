# -*- coding: utf-8 -*-

"""Recipe commands for Mercurial."""

import logging

log = logging.getLogger('bitten.build.hgtools')

__docformat__ = 'restructuredtext en'

def pull(ctxt, revision=None, dir_='.'):
    """pull and update the local working copy from the Mercurial repository.
    
    :param ctxt: the build context
    :type ctxt: `Context`
    :param revision: the revision to check out
    :param dir_: the name of a local subdirectory containing the working copy
    """
    args = ['pull', '-u']
    if revision:
        args += ['-r', revision.split(':')[0]]
    args += [dir_]

    from bitten.build import shtools
    returncode = shtools.execute(ctxt, file_='hg', args=args)
    if returncode != 0:
        ctxt.error('hg pull -u failed (%s)' % returncode)

