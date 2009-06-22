# -*- coding: utf-8 -*-
#
# Copyright (C) 2007 Edgewall Software
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Build master implementation."""

import calendar
import re
import time

from trac.config import BoolOption, IntOption, PathOption
from trac.core import *
from trac.web import IRequestHandler, HTTPBadRequest, HTTPConflict, \
                     HTTPForbidden, HTTPMethodNotAllowed, HTTPNotFound, \
                     RequestDone

from bitten.model import BuildConfig, Build, BuildStep, BuildLog, Report, \
                     TargetPlatform

from bitten.main import BuildSystem
from bitten.queue import BuildQueue
from bitten.recipe import Recipe
from bitten.util import xmlio

__all__ = ['BuildMaster']
__docformat__ = 'restructuredtext en'


class BuildMaster(Component):
    """Trac request handler implementation for the build master."""

    implements(IRequestHandler)

    # Configuration options

    adjust_timestamps = BoolOption('bitten', 'adjust_timestamps', False, doc=
        """Whether the timestamps of builds should be adjusted to be close
        to the timestamps of the corresponding changesets.""")

    build_all = BoolOption('bitten', 'build_all', False, doc=
        """Whether to request builds of older revisions even if a younger
        revision has already been built.""")
    
    stabilize_wait = IntOption('bitten', 'stabilize_wait', 0, doc=
        """The time in seconds to wait for the repository to stabilize before
        queuing up a new build.  This allows time for developers to check in
        a group of related changes back to back without spawning multiple
        builds.""")

    slave_timeout = IntOption('bitten', 'slave_timeout', 3600, doc=
        """The time in seconds after which a build is cancelled if the slave
        does not report progress.""")

    logs_dir = PathOption('bitten', 'logs_dir', "log/bitten", doc=
         """The directory on the server in which client log files will be stored.""")

    quick_status = BoolOption('bitten', 'quick_status', False, doc=
         """Whether to show the current build status withing the Trac main 
            navigation bar""")

    # IRequestHandler methods

    def match_request(self, req):
        match = re.match(r'/builds(?:/(\d+)(?:/(\w+)/([^/]+)?)?)?$',
                         req.path_info)
        if match:
            if match.group(1):
                req.args['id'] = match.group(1)
                req.args['collection'] = match.group(2)
                req.args['member'] = match.group(3)
            return True

    def process_request(self, req):
        req.perm.assert_permission('BUILD_EXEC')

        if 'id' not in req.args:
            if req.method != 'POST':
                raise HTTPMethodNotAllowed('Method not allowed')
            return self._process_build_creation(req)

        build = Build.fetch(self.env, req.args['id'])
        if not build:
            raise HTTPNotFound('No such build')
        config = BuildConfig.fetch(self.env, build.config)

        if not req.args['collection']:
            if req.method == 'DELETE':
                return self._process_build_cancellation(req, config, build)
            else:
                return self._process_build_initiation(req, config, build)

        if req.method != 'POST':
            raise HTTPMethodNotAllowed('Method not allowed')

        if req.args['collection'] == 'steps':
            return self._process_build_step(req, config, build)
        else:
            raise HTTPNotFound('No such collection')

    def _process_build_creation(self, req):
        queue = BuildQueue(self.env, build_all=self.build_all, 
                           stabilize_wait=self.stabilize_wait,
                           timeout=self.slave_timeout)
        queue.populate()

        try:
            elem = xmlio.parse(req.read())
        except xmlio.ParseError, e:
            self.log.error('Error parsing build initialization request: %s', e,
                           exc_info=True)
            raise HTTPBadRequest('XML parser error')

        slavename = elem.attr['name']
        properties = {'name': slavename, Build.IP_ADDRESS: req.remote_addr}
        self.log.info('Build slave %r connected from %s', slavename,
                      req.remote_addr)

        for child in elem.children():
            if child.name == 'platform':
                properties[Build.MACHINE] = child.gettext()
                properties[Build.PROCESSOR] = child.attr.get('processor')
            elif child.name == 'os':
                properties[Build.OS_NAME] = child.gettext()
                properties[Build.OS_FAMILY] = child.attr.get('family')
                properties[Build.OS_VERSION] = child.attr.get('version')
            elif child.name == 'package':
                for name, value in child.attr.items():
                    if name == 'name':
                        continue
                    properties[child.attr['name'] + '.' + name] = value

        self.log.debug('Build slave configuration: %r', properties)

        build = queue.get_build_for_slave(slavename, properties)
        if not build:
            req.send_response(204)
            req.write('')
            raise RequestDone

        req.send_response(201)
        req.send_header('Content-Type', 'text/plain')
        req.send_header('Location', req.abs_href.builds(build.id))
        req.write('Build pending')
        raise RequestDone

    def _process_build_cancellation(self, req, config, build):
        self.log.info('Build slave %r cancelled build %d', build.slave,
                      build.id)
        build.status = Build.PENDING
        build.slave = None
        build.slave_info = {}
        build.started = 0
        db = self.env.get_db_cnx()
        for step in list(BuildStep.select(self.env, build=build.id, db=db)):
            step.delete(db=db)
        build.update(db=db)
        db.commit()

        for listener in BuildSystem(self.env).listeners:
            listener.build_aborted(build)

        req.send_response(204)
        req.write('')
        raise RequestDone

    def _process_build_initiation(self, req, config, build):
        self.log.info('Build slave %r initiated build %d', build.slave,
                      build.id)
        build.started = int(time.time())
        build.update()

        for listener in BuildSystem(self.env).listeners:
            listener.build_started(build)

        xml = xmlio.parse(config.recipe)
        xml.attr['path'] = config.path
        xml.attr['revision'] = build.rev
        xml.attr['config'] = config.name
        xml.attr['build'] = str(build.id)
        target_platform = TargetPlatform.fetch(self.env, build.platform)
        xml.attr['platform'] = target_platform.name
        body = str(xml)

        self.log.info('Build slave %r initiated build %d', build.slave,
                      build.id)

        req.send_response(200)
        req.send_header('Content-Type', 'application/x-bitten+xml')
        req.send_header('Content-Length', str(len(body)))
        req.send_header('Content-Disposition',
                        'attachment; filename=recipe_%s_r%s.xml' %
                        (config.name, build.rev))
        req.write(body)
        raise RequestDone

    def _process_build_step(self, req, config, build):
        try:
            elem = xmlio.parse(req.read())
        except xmlio.ParseError, e:
            self.log.error('Error parsing build step result: %s', e,
                           exc_info=True)
            raise HTTPBadRequest('XML parser error')
        stepname = elem.attr['step']
	
        # make sure it's the right slave.
        if build.status != Build.IN_PROGRESS or \
                build.slave_info.get(Build.IP_ADDRESS) != req.remote_addr:
            raise HTTPForbidden('Build %s has been invalidated for host %s.'
                                % (build.id, req.remote_addr))

        step = BuildStep.fetch(self.env, build=build.id, name=stepname)
        if step:
            raise HTTPConflict('Build step already exists')

        recipe = Recipe(xmlio.parse(config.recipe))
        index = None
        current_step = None
        for num, recipe_step in enumerate(recipe):
            if recipe_step.id == stepname:
                index = num
                current_step = recipe_step
        if index is None:
            raise HTTPForbidden('No such build step')
        last_step = index == num

        self.log.debug('Slave %s (build %d) completed step %d (%s) with '
                       'status %s', build.slave, build.id, index, stepname,
                       elem.attr['status'])

        db = self.env.get_db_cnx()

        step = BuildStep(self.env, build=build.id, name=stepname)
        try:
            step.started = int(_parse_iso_datetime(elem.attr['time']))
            step.stopped = step.started + float(elem.attr['duration'])
        except ValueError, e:
            self.log.error('Error parsing build step timestamp: %s', e,
                           exc_info=True)
            raise HTTPBadRequest(e.args[0])
        if elem.attr['status'] == 'failure':
            self.log.warning('Build %s step %s failed', build.id, stepname)
            step.status = BuildStep.FAILURE
            if current_step.onerror == 'fail':
                last_step = True
        else:
            step.status = BuildStep.SUCCESS
        step.errors += [error.gettext() for error in elem.children('error')]
        step.insert(db=db)

        # Collect log messages from the request body
        for idx, log_elem in enumerate(elem.children('log')):
            build_log = BuildLog(self.env, build=build.id, step=stepname,
                                 generator=log_elem.attr.get('generator'),
                                 orderno=idx)
            for message_elem in log_elem.children('message'):
                build_log.messages.append((message_elem.attr['level'],
                                           message_elem.gettext()))
            build_log.insert(db=db)

        # Collect report data from the request body
        for report_elem in elem.children('report'):
            report = Report(self.env, build=build.id, step=stepname,
                            category=report_elem.attr.get('category'),
                            generator=report_elem.attr.get('generator'))
            for item_elem in report_elem.children():
                item = {'type': item_elem.name}
                item.update(item_elem.attr)
                for child_elem in item_elem.children():
                    item[child_elem.name] = child_elem.gettext()
                report.items.append(item)
            report.insert(db=db)

        # If this was the last step in the recipe we mark the build as
        # completed
        if last_step:
            self.log.info('Slave %s completed build %d ("%s" as of [%s])',
                          build.slave, build.id, build.config, build.rev)
            build.stopped = step.stopped

            # Determine overall outcome of the build by checking the outcome
            # of the individual steps against the "onerror" specification of
            # each step in the recipe
            for num, recipe_step in enumerate(recipe):
                step = BuildStep.fetch(self.env, build.id, recipe_step.id)
                if step.status == BuildStep.FAILURE:
                    if recipe_step.onerror != 'ignore':
                        build.status = Build.FAILURE
                        break
            else:
                build.status = Build.SUCCESS

            build.update(db=db)

        db.commit()

        if last_step:
            for listener in BuildSystem(self.env).listeners:
                listener.build_completed(build)

        body = 'Build step processed'
        req.send_response(201)
        req.send_header('Content-Type', 'text/plain')
        req.send_header('Content-Length', str(len(body)))
        req.send_header('Location', req.abs_href.builds(build.id, 'steps',
                        stepname))
        req.write(body)
        raise RequestDone


def _parse_iso_datetime(string):
    """Minimal parser for ISO date-time strings.
    
    Return the time as floating point number. Only handles UTC timestamps
    without time zone information."""
    try:
        string = string.split('.', 1)[0] # strip out microseconds
        return calendar.timegm(time.strptime(string, '%Y-%m-%dT%H:%M:%S'))
    except ValueError, e:
        raise ValueError('Invalid ISO date/time %r' % string)
