#-*- coding: utf-8 -*-
#
# Copyright (C) 2007 Ole Trenner, <ole@jayotee.de>
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution.

from trac.core import *
from trac.web.chrome import ITemplateProvider
from trac.web.session import DetachedSession
from trac.config import BoolOption
from trac.notification import NotifyEmail
from bitten.api import IBuildListener
from bitten.model import Build, BuildStep, BuildLog


CONFIG_SECTION = 'notification'
NOTIFY_ON_FAILURE = 'notify_on_failed_build'
NOTIFY_ON_SUCCESS = 'notify_on_successful_build'


class BittenNotify(Component):
    """Sends notifications on build status by mail."""
    implements(IBuildListener, ITemplateProvider)

    notify_on_failure = BoolOption(CONFIG_SECTION, NOTIFY_ON_FAILURE, 'true',
            """Notify if bitten build fails.""")

    notify_on_success = BoolOption(CONFIG_SECTION, NOTIFY_ON_SUCCESS, 'false',
            """Notify if bitten build succeeds.""")

    def __init__(self):
        self.log.debug('Initializing BittenNotify plugin')

    def notify(self, build=None):
        self.log.info('BittenNotify invoked for build %r' % build)
        self.log.debug('build status: %s' % build.status)
        if not self._should_notify(build):
            return
        self.log.info('Sending notification for build %r' % build)
        try:
            email = BittenNotifyEmail(self.env)
            email.notify(BuildInfo(self.env, build))
        except Exception, e:
            self.log.exception("Failure sending notification for build "
                               "%s: %s", build.id, e)

    def _should_notify(self, build):
        if build.status == Build.FAILURE:
            return self.notify_on_failure
        elif build.status == Build.SUCCESS:
            return self.notify_on_success
        else:
            return False

    # IBuildListener methods

    def build_started(self, build):
        """build started"""
        self.notify(build)

    def build_aborted(self, build):
        """build aborted"""
        self.notify(build)

    def build_completed(self, build):
        """build completed"""
        self.notify(build)

    # ITemplateProvider methods

    def get_templates_dirs(self):
        """Return a list of directories containing the provided template
        files."""
        from pkg_resources import resource_filename
        return [resource_filename(__name__, 'templates')]

    def get_htdocs_dirs(self):
        """Return the absolute path of a directory containing additional
        static resources (such as images, style sheets, etc)."""
        return []


class BuildInfo(dict):
    """Wraps a Build instance and exposes properties conveniently"""

    readable_states = {Build.SUCCESS:'Successful', Build.FAILURE:'Failed'}

    def __init__(self, env, build):
        dict.__init__(self)
        self.build = build
        self.env = env
        self['project_name'] = self.env.project_name
        self['id'] = self.build.id
        self['status'] = self.readable_states[self.build.status]
        self['link'] = self.env.abs_href.build(self.build.config,
                self.build.id)
        self['config'] = self.build.config
        self['slave'] = self.build.slave
        self['changeset'] = self.build.rev
        self['changesetlink'] = self.env.abs_href.changeset(self.build.rev)
        self['author'] = self.get_author(build)
        self['errors'] = self.get_errors(build)
        self['faillog'] = self.get_faillog(build)

    def get_author(self, build):
        if build and build.rev:
            changeset = self.env.get_repository().get_changeset(build.rev)
            return changeset.author

    def get_failed_steps(self, build):
        build_steps = BuildStep.select(self.env,
                build=build.id,
                status=BuildStep.FAILURE)
        return build_steps

    def get_errors(self, build):
        errors = ''
        for step in self.get_failed_steps(build):
            errors += ', '.join(['%s: %s' % (step.name, error) \
                    for error in step.errors])
        return errors

    def get_faillog(self, build):
        faillog = ''
        for step in self.get_failed_steps(build):
            build_logs = BuildLog.select(self.env,
                    build=build.id,
                    step=step.name)
            for log in build_logs:
                faillog += '\n'.join(['%5s: %s' % (level, msg) \
                        for level, msg in log.messages])
        return faillog

    def __getattr__(self, attr):
        return dict.__getitem__(self,attr)

    def __repr__(self):
        repr = ''
        for k, v in self.items():
            repr += '%s: %s\n' % (k, v)
        return repr

    def __str__(self):
        return self.repr()


class BittenNotifyEmail(NotifyEmail):
    """Notification of failed builds."""

    template_name = 'bitten_notify_email.txt'
    from_email = 'bitten@localhost'

    def __init__(self, env):
        NotifyEmail.__init__(self, env)

    def notify(self, build_info):
        self.build_info = build_info
        self.data = self.build_info
        subject = '[%s Build] %s [%s] %s' % (self.build_info.status,
                self.env.project_name,
                self.build_info.changeset,
                self.build_info.config)
        stream = self.template.generate(**self.data)
        body = stream.render('text')
        self.env.log.debug('notification: %s' % body )
        NotifyEmail.notify(self, self.build_info.id, subject)

    def get_recipients(self, resid):
        author = self.build_info.author
        author = DetachedSession(self.env, author).get('email') or author
        torecipients = [author]
        ccrecipients = []
        return (torecipients, ccrecipients)

    def send(self, torcpts, ccrcpts, mime_headers={}):
        mime_headers = {
            'X-Trac-Build-ID': str(self.build_info.id),
            'X-Trac-Build-URL': self.build_info.link,
        }
        NotifyEmail.send(self, torcpts, ccrcpts, mime_headers)
