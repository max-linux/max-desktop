# -*- coding: utf-8 -*-
#
# Copyright (C) 2007 Edgewall Software
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Implements the scheduling of builds for a project.

This module provides the functionality for scheduling builds for a specific
Trac environment. It is used by both the build master and the web interface to
get the list of required builds (revisions not built yet).

Furthermore, the `BuildQueue` class is used by the build master to determine
the next pending build, and to match build slaves against configured target
platforms.
"""

from datetime import datetime
from itertools import ifilter
import logging
import re
import time

from trac.versioncontrol import NoSuchNode
from bitten.model import BuildConfig, TargetPlatform, Build, BuildStep

__docformat__ = 'restructuredtext en'

log = logging.getLogger('bitten.queue')


def collect_changes(repos, config, db=None):
    """Collect all changes for a build configuration that either have already
    been built, or still need to be built.
    
    This function is a generator that yields ``(platform, rev, build)`` tuples,
    where ``platform`` is a `TargetPlatform` object, ``rev`` is the identifier
    of the changeset, and ``build`` is a `Build` object or `None`.

    :param repos: the version control repository
    :param config: the build configuration
    :param db: a database connection (optional)
    """
    env = config.env
    if not db:
        db = env.get_db_cnx()
    try:
        node = repos.get_node(config.path)
    except NoSuchNode, e:
        env.log.warn('Node for configuration %r not found', config.name,
                     exc_info=True)
        return

    for path, rev, chg in node.get_history():

        # Don't follow moves/copies
        if path != repos.normalize_path(config.path):
            break

        # Stay within the limits of the build config
        if config.min_rev and repos.rev_older_than(rev, config.min_rev):
            break
        if config.max_rev and repos.rev_older_than(config.max_rev, rev):
            continue

        # Make sure the repository directory isn't empty at this
        # revision
        old_node = repos.get_node(path, rev)
        is_empty = True
        for entry in old_node.get_entries():
            is_empty = False
            break
        if is_empty:
            continue

        # For every target platform, check whether there's a build
        # of this revision
        for platform in TargetPlatform.select(env, config.name, db=db):
            builds = list(Build.select(env, config.name, rev, platform.id,
                                       db=db))
            if builds:
                build = builds[0]
            else:
                build = None

            yield platform, rev, build


class BuildQueue(object):
    """Enapsulates the build queue of an environment.
    
    A build queue manages the the registration of build slaves and detection of
    repository revisions that need to be built.
    """

    def __init__(self, env, build_all=False, stabilize_wait=0, timeout=0):
        """Create the build queue.
        
        :param env: the Trac environment
        :param build_all: whether older revisions should be built
        :param stabilize_wait: The time in seconds to wait before considering
                        the repository stable to create a build in the queue.
        :param timeout: the time in seconds after which an in-progress build
                        should be considered orphaned, and reset to pending
                        state
        """
        self.env = env
        self.log = env.log
        self.build_all = build_all
        self.stabilize_wait = stabilize_wait
        self.timeout = timeout

    # Build scheduling

    def get_build_for_slave(self, name, properties):
        """Check whether one of the pending builds can be built by the build
        slave.
        
        :param name: the name of the slave
        :type name: `basestring`
        :param properties: the slave configuration
        :type properties: `dict`
        :return: the allocated build, or `None` if no build was found
        :rtype: `Build`
        """
        log.debug('Checking for pending builds...')

        db = self.env.get_db_cnx()
        repos = self.env.get_repository()

        self.reset_orphaned_builds()

        # Iterate through pending builds by descending revision timestamp, to
        # avoid the first configuration/platform getting all the builds
        platforms = [p.id for p in self.match_slave(name, properties)]
        build = None
        builds_to_delete = []
        for build in Build.select(self.env, status=Build.PENDING, db=db):
            if self.should_delete_build(build, repos):
                self.log.info('Scheduling build %d for deletion', build.id)
                builds_to_delete.append(build)
            elif build.platform in platforms:
                break
        else:
            self.log.debug('No pending builds.')
            build = None

        # delete any obsolete builds
        for build_to_delete in builds_to_delete:
            build_to_delete.delete(db=db)

        if build:
            build.slave = name
            build.slave_info.update(properties)
            build.status = Build.IN_PROGRESS
            build.update(db=db)

        if build or builds_to_delete:
            db.commit()

        return build

    def match_slave(self, name, properties):
        """Match a build slave against available target platforms.
        
        :param name: the name of the slave
        :type name: `basestring`
        :param properties: the slave configuration
        :type properties: `dict`
        :return: the list of platforms the slave matched
        """
        platforms = []

        for config in BuildConfig.select(self.env):
            for platform in TargetPlatform.select(self.env, config=config.name):
                match = True
                for propname, pattern in ifilter(None, platform.rules):
                    try:
                        propvalue = properties.get(propname)
                        if not propvalue or not re.match(pattern, propvalue):
                            match = False
                            break
                    except re.error:
                        self.log.error('Invalid platform matching pattern "%s"',
                                       pattern, exc_info=True)
                        match = False
                        break
                if match:
                    self.log.debug('Slave %r matched target platform %r of '
                                   'build configuration %r', name,
                                   platform.name, config.name)
                    platforms.append(platform)

        if not platforms:
            self.log.warning('Slave %r matched none of the target platforms',
                             name)

        return platforms

    def populate(self):
        """Add a build for the next change on each build configuration to the
        queue.

        The next change is the latest repository check-in for which there isn't
        a corresponding build on each target platform. Repeatedly calling this
        method will eventually result in the entire change history of the build
        configuration being in the build queue.
        """
        repos = self.env.get_repository()
        if hasattr(repos, 'sync'):
            repos.sync()

        db = self.env.get_db_cnx()
        builds = []

        for config in BuildConfig.select(self.env, db=db):
            platforms = []
            for platform, rev, build in collect_changes(repos, config, db):

                if not self.build_all and platform.id in platforms:
                    # We've seen this platform already, so these are older
                    # builds that should only be built if built_all=True
                    self.log.debug('Ignoring older revisions for configuration '
                                   '%r on %r', config.name, platform.name)
                    break

                platforms.append(platform.id)

                if build is None:
                    self.log.info('Enqueuing build of configuration "%s" at '
                                  'revision [%s] on %s', config.name, rev,
                                  platform.name)

                    rev_time = repos.get_changeset(rev).date
                    if isinstance(rev_time, datetime): # Trac>=0.11
                        from trac.util.datefmt import to_timestamp
                        rev_time = to_timestamp(rev_time)
                    age = int(time.time()) - rev_time
                    if self.stabilize_wait and age < self.stabilize_wait:
                        self.log.info('Delaying build of revision %s until %s '
                                      'seconds pass. Current age is: %s '
                                      'seconds' % (rev, self.stabilize_wait,
                                      age))
                        continue

                    build = Build(self.env, config=config.name,
                                  platform=platform.id, rev=str(rev),
                                  rev_time=rev_time)
                    builds.append(build)

        for build in builds:
            build.insert(db=db)

        db.commit()

    def reset_orphaned_builds(self):
        """Reset all in-progress builds to ``PENDING`` state if they've been
        running so long that the configured timeout has been reached.
        
        This is used to cleanup after slaves that have unexpectedly cancelled
        a build without notifying the master, or are for some other reason not
        reporting back status updates.
        """
        if not self.timeout:
            # If no timeout is set, none of the in-progress builds can be
            # considered orphaned
            return

        db = self.env.get_db_cnx()
        now = int(time.time())
        for build in Build.select(self.env, status=Build.IN_PROGRESS, db=db):
            if now - build.started < self.timeout:
                # This build has not reached the timeout yet, assume it's still
                # being executed
                # FIXME: ideally, we'd base this check on the last activity on
                #        the build, not the start time
                continue
            build.status = Build.PENDING
            build.slave = None
            build.slave_info = {}
            build.started = 0
            for step in list(BuildStep.select(self.env, build=build.id, db=db)):
                step.delete(db=db)
            build.update(db=db)
        db.commit()

    def should_delete_build(self, build, repos):
        # Ignore pending builds for deactived build configs
        config = BuildConfig.fetch(self.env, build.config)
        if not config.active:
            target_platform = TargetPlatform.fetch(self.env, build.platform)
            if target_platform:
                target_platform_name = '"%s"' % (target_platform.name,)
            else:
                target_platform_name = 'unknown platform "%s"' % (build.platform,)
            log.info('Dropping build of configuration "%s" at '
                     'revision [%s] on %s because the configuration is '
                     'deactivated', config.name, build.rev,
                     target_platform_name)
            return True

        # Stay within the revision limits of the build config
        if (config.min_rev and repos.rev_older_than(build.rev,
                                                    config.min_rev)) \
        or (config.max_rev and repos.rev_older_than(config.max_rev,
                                                    build.rev)):
            # This minimum and/or maximum revision has changed since
            # this build was enqueued, so drop it
            log.info('Dropping build of configuration "%s" at revision [%s] on '
                     '"%s" because it is outside of the revision range of the '
                     'configuration', config.name, build.rev,
                     TargetPlatform.fetch(self.env, build.platform).name)
            return True

        return False
