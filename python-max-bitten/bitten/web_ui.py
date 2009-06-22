# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Implementation of the Bitten web interface."""

from datetime import datetime
import posixpath
import re
from StringIO import StringIO

import pkg_resources
from genshi.builder import tag
from trac.core import *
from trac.timeline import ITimelineEventProvider
from trac.util import escape, pretty_timedelta, format_datetime, shorten_line, \
                      Markup
from trac.util.html import html
from trac.web import IRequestHandler, IRequestFilter
from trac.web.chrome import INavigationContributor, ITemplateProvider, \
                            add_link, add_stylesheet, add_ctxtnav, \
                            prevnext_nav, add_script
from trac.wiki import wiki_to_html, wiki_to_oneliner
from bitten.api import ILogFormatter, IReportChartGenerator, IReportSummarizer
from bitten.master import BuildMaster
from bitten.model import BuildConfig, TargetPlatform, Build, BuildStep, \
                         BuildLog, Report
from bitten.queue import collect_changes

_status_label = {Build.PENDING: 'pending',
                 Build.IN_PROGRESS: 'in progress',
                 Build.SUCCESS: 'completed',
                 Build.FAILURE: 'failed'}
_status_title = {Build.PENDING: 'Pending',
                 Build.IN_PROGRESS: 'In Progress',
                 Build.SUCCESS: 'Success',
                 Build.FAILURE: 'Failure'}

def _get_build_data(env, req, build):
    data = {'id': build.id, 'name': build.slave, 'rev': build.rev,
            'status': _status_label[build.status],
            'cls': _status_label[build.status].replace(' ', '-'),
            'href': req.href.build(build.config, build.id),
            'chgset_href': req.href.changeset(build.rev)}
    if build.started:
        data['started'] = format_datetime(build.started)
        data['started_delta'] = pretty_timedelta(build.started)
        data['duration'] = pretty_timedelta(build.started)
    if build.stopped:
        data['stopped'] = format_datetime(build.stopped)
        data['stopped_delta'] = pretty_timedelta(build.stopped)
        data['duration'] = pretty_timedelta(build.stopped, build.started)
    data['slave'] = {
        'name': build.slave,
        'ipnr': build.slave_info.get(Build.IP_ADDRESS),
        'os_name': build.slave_info.get(Build.OS_NAME),
        'os_family': build.slave_info.get(Build.OS_FAMILY),
        'os_version': build.slave_info.get(Build.OS_VERSION),
        'machine': build.slave_info.get(Build.MACHINE),
        'processor': build.slave_info.get(Build.PROCESSOR)
    }
    return data


class BittenChrome(Component):
    """Provides the Bitten templates and static resources."""

    implements(INavigationContributor, ITemplateProvider)

    # INavigationContributor methods

    def get_active_navigation_item(self, req):
        """Called by Trac to determine which navigation item should be marked
        as active.
        
        :param req: the request object
        """
        return 'build'

    def get_navigation_items(self, req):
        """Return the navigation item for access the build status overview from
        the Trac navigation bar."""
        if 'BUILD_VIEW' in req.perm:
            status = ''
            if BuildMaster(self.env).quick_status:
                repos = self.env.get_repository(req.authname)
                if hasattr(repos, 'sync'):
                    repos.sync()
                for config in BuildConfig.select(self.env, 
                                                 include_inactive=False):
                    prev_rev = None
                    for platform, rev, build in collect_changes(repos, config):
                        if rev != prev_rev:
                            if prev_rev is not None:
                               break
                            prev_rev = rev
                        if build:
                            build_data = _get_build_data(self.env, req, build)
                            if build_data['status'] == 'failed':
                                status='bittenfailed'
                                break
                            if build_data['status'] == 'in progress':
                                status='bitteninprogress'
                            elif not status:
                                if (build_data['status'] == 'completed'):
                                    status='bittencompleted'  
                if not status:
                    status='bittenpending'
            yield ('mainnav', 'build',
                   tag.a('Builds Status', href=req.href.build(), accesskey=5, 
                         class_=status))

    # ITemplatesProvider methods

    def get_htdocs_dirs(self):
        """Return the directories containing static resources."""
        return [('bitten', pkg_resources.resource_filename(__name__, 'htdocs'))]

    def get_templates_dirs(self):
        """Return the directories containing templates."""
        return [pkg_resources.resource_filename(__name__, 'templates')]


class BuildConfigController(Component):
    """Implements the web interface for build configurations."""

    implements(IRequestHandler, IRequestFilter)

    # IRequestHandler methods

    def match_request(self, req):
        match = re.match(r'/build(?:/([\w.-]+))?/?$', req.path_info)
        if match:
            if match.group(1):
                req.args['config'] = match.group(1)
            return True

    def process_request(self, req):
        req.perm.require('BUILD_VIEW')

        action = req.args.get('action')
        view = req.args.get('view')
        config = req.args.get('config')

        if config:
            data = self._render_config(req, config)
        elif view == 'inprogress':
            data = self._render_inprogress(req)
        else:
            data = self._render_overview(req)

        add_stylesheet(req, 'bitten/bitten.css')
        return 'bitten_config.html', data, None

    # IRequestHandler methods
    
    def pre_process_request(self, req, handler):
        return handler

    def post_process_request(self, req, template, data, content_type):
        if template:
            add_stylesheet(req, 'bitten/bitten.css')
        
        return template, data, content_type
        
    # Internal methods

    def _render_overview(self, req):
        data = {'title': 'Build Status'}
        show_all = False
        if req.args.get('show') == 'all':
            show_all = True
        data['show_all'] = show_all

        repos = self.env.get_repository(req.authname)
        if hasattr(repos, 'sync'):
            repos.sync()

        configs = []
        for config in BuildConfig.select(self.env, include_inactive=show_all):
            if not repos.authz.has_permission(config.path):
                continue

            description = config.description
            if description:
                description = wiki_to_html(description, self.env, req)

            platforms_data = []
            for platform in TargetPlatform.select(self.env, config=config.name):
                pd = { 'name': platform.name,
                       'id': platform.id,
                       'builds_pending': len(list(Build.select(self.env, config=config.name, status=Build.PENDING, platform=platform.id))),
                       'builds_inprogress': len(list(Build.select(self.env, config=config.name, status=Build.IN_PROGRESS, platform=platform.id)))
                }
                platforms_data.append(pd)
		
            config_data = {
                'name': config.name, 'label': config.label or config.name,
                'active': config.active, 'path': config.path,
                'description': description,
		'builds_pending' : len(list(Build.select(self.env, config=config.name, status=Build.PENDING))),
		'builds_inprogress' : len(list(Build.select(self.env, config=config.name, status=Build.IN_PROGRESS))),
                'href': req.href.build(config.name),
                'builds': [],
                'platforms': platforms_data
            }
            configs.append(config_data)
            if not config.active:
                continue

            prev_rev = None
            for platform, rev, build in collect_changes(repos, config):
                if rev != prev_rev:
                    if prev_rev is None:
                        chgset = repos.get_changeset(rev)
                        config_data['youngest_rev'] = {
                            'id': rev, 'href': req.href.changeset(rev),
                            'author': chgset.author or 'anonymous',
                            'date': format_datetime(chgset.date),
                            'message': wiki_to_oneliner(
                                shorten_line(chgset.message), self.env, req=req)
                        }
                    else:
                        break
                    prev_rev = rev
                if build:
                    build_data = _get_build_data(self.env, req, build)
                    build_data['platform'] = platform.name
                    config_data['builds'].append(build_data)
                else:
                    config_data['builds'].append({
                        'platform': platform.name, 'status': 'pending'
                    })

        data['configs'] = configs
        data['page_mode'] = 'overview'

        in_progress_builds = Build.select(self.env, status=Build.IN_PROGRESS)
        pending_builds = Build.select(self.env, status=Build.PENDING)

	data['builds_pending'] = len(list(pending_builds))
	data['builds_inprogress'] = len(list(in_progress_builds))

        add_link(req, 'views', req.href.build(view='inprogress'),
                 'In Progress Builds')
        add_ctxtnav(req, 'In Progress Builds',
                    req.href.build(view='inprogress'))
        return data

    def _render_inprogress(self, req):
        data = {'title': 'In Progress Builds',
                'page_mode': 'view-inprogress'}

        db = self.env.get_db_cnx()

        repos = self.env.get_repository(req.authname)
        if hasattr(repos, 'sync'):
            repos.sync()

        configs = []
        for config in BuildConfig.select(self.env, include_inactive=False):
            if not repos.authz.has_permission(config.path):
                continue

            self.log.debug(config.name)
            if not config.active:
                continue

            in_progress_builds = Build.select(self.env, config=config.name,
                                              status=Build.IN_PROGRESS, db=db)

            current_builds = 0
            builds = []
            # sort correctly by revision.
            for build in sorted(in_progress_builds,
                                cmp=lambda x, y: int(y.rev) - int(x.rev)):
                rev = build.rev
                build_data = _get_build_data(self.env, req, build)
                build_data['rev'] = rev
                build_data['rev_href'] = req.href.changeset(rev)
                platform = TargetPlatform.fetch(self.env, build.platform)
                build_data['platform'] = platform.name
                build_data['steps'] = []

                for step in BuildStep.select(self.env, build=build.id, db=db):
                    build_data['steps'].append({
                        'name': step.name,
                        'description': step.description,
                        'duration': datetime.fromtimestamp(step.stopped) - \
                                    datetime.fromtimestamp(step.started),
                        'failed': not step.successful,
                        'errors': step.errors,
                        'href': build_data['href'] + '#step_' + step.name
                    })

                builds.append(build_data)
                current_builds += 1

            if current_builds == 0: 
                continue

            description = config.description
            if description:
                description = wiki_to_html(description, self.env, req)
            configs.append({
                'name': config.name, 'label': config.label or config.name,
                'active': config.active, 'path': config.path,
                'description': description,
                'href': req.href.build(config.name),
                'builds': builds
            })

        data['configs'] = configs
        return data

    def _render_config(self, req, config_name):
        db = self.env.get_db_cnx()

        config = BuildConfig.fetch(self.env, config_name, db=db)

        repos = self.env.get_repository(req.authname)
        if hasattr(repos, 'sync'):
            repos.sync()
        repos.authz.assert_permission(config.path)

        data = {'title': 'Build Configuration "%s"' \
                          % config.label or config.name,
                'page_mode': 'view_config'}
        add_link(req, 'up', req.href.build(), 'Build Status')
        description = config.description
        if description:
            description = wiki_to_html(description, self.env, req)

        pending_builds = list(Build.select(self.env, config=config.name, status=Build.PENDING))
        inprogress_builds = list(Build.select(self.env, config=config.name, status=Build.IN_PROGRESS))

        data['config'] = {
            'name': config.name, 'label': config.label, 'path': config.path,
            'min_rev': config.min_rev,
            'min_rev_href': req.href.changeset(config.min_rev),
            'max_rev': config.max_rev,
            'max_rev_href': req.href.changeset(config.max_rev),
            'active': config.active, 'description': description,
            'browser_href': req.href.browser(config.path),
            'builds_pending' : len(pending_builds),
	    'builds_inprogress' : len(inprogress_builds)
        }

        platforms = list(TargetPlatform.select(self.env, config=config_name,
                                               db=db))
        data['config']['platforms'] = [
            { 'name': platform.name,
              'id': platform.id, 
              'builds_pending': len(list(Build.select(self.env, config=config.name, status=Build.PENDING, platform=platform.id))),
              'builds_inprogress': len(list(Build.select(self.env, config=config.name, status=Build.IN_PROGRESS, platform=platform.id)))
              }
            for platform in platforms
        ]

        has_reports = False
        for report in Report.select(self.env, config=config.name, db=db):
            has_reports = True
            break

        if has_reports:
            chart_generators = []
            for generator in ReportChartController(self.env).generators: 
                for category in generator.get_supported_categories(): 
                    chart_generators.append({
                        'href': req.href.build(config.name, 'chart/' + category) 
                    })
            data['config']['charts'] = chart_generators 
            charts_license = self.config.get('bitten', 'charts_license')
            if charts_license:
                data['config']['charts_license'] = charts_license

        page = max(1, int(req.args.get('page', 1)))
        more = False
        data['page_number'] = page

        repos = self.env.get_repository(req.authname)
        if hasattr(repos, 'sync'):
            repos.sync()

        builds_per_page = 12 * len(platforms)
        idx = 0
        builds = {}
        for platform, rev, build in collect_changes(repos, config):
            if idx >= page * builds_per_page:
                more = True
                break
            elif idx >= (page - 1) * builds_per_page:
                builds.setdefault(rev, {})
                builds[rev].setdefault('href', req.href.changeset(rev))
                if build and build.status != Build.PENDING:
                    build_data = _get_build_data(self.env, req, build)
                    build_data['steps'] = []
                    for step in BuildStep.select(self.env, build=build.id,
                                                 db=db):
                        build_data['steps'].append({
                            'name': step.name,
                            'description': step.description,
                            'duration': datetime.fromtimestamp(step.stopped) - \
                                        datetime.fromtimestamp(step.started),
                            'failed': not step.successful,
                            'errors': step.errors,
                            'href': build_data['href'] + '#step_' + step.name
                        })
                    builds[rev][platform.id] = build_data
            idx += 1
        data['config']['builds'] = builds

        if page > 1:
            if page == 2:
                prev_href = req.href.build(config.name)
            else:
                prev_href = req.href.build(config.name, page=page - 1)
            add_link(req, 'prev', prev_href, 'Previous Page')
        if more:
            next_href = req.href.build(config.name, page=page + 1)
            add_link(req, 'next', next_href, 'Next Page')
        prevnext_nav(req, 'Page')
        return data


class BuildController(Component):
    """Renders the build page."""
    implements(INavigationContributor, IRequestHandler, ITimelineEventProvider)

    log_formatters = ExtensionPoint(ILogFormatter)
    report_summarizers = ExtensionPoint(IReportSummarizer)

    # INavigationContributor methods

    def get_active_navigation_item(self, req):
        return 'build'

    def get_navigation_items(self, req):
        return []

    # IRequestHandler methods

    def match_request(self, req):
        match = re.match(r'/build/([\w.-]+)/(\d+)', req.path_info)
        if match:
            if match.group(1):
                req.args['config'] = match.group(1)
                if match.group(2):
                    req.args['id'] = match.group(2)
            return True

    def process_request(self, req):
        req.perm.require('BUILD_VIEW')

        db = self.env.get_db_cnx()
        build_id = int(req.args.get('id'))
        build = Build.fetch(self.env, build_id, db=db)
        assert build, 'Build %s does not exist' % build_id

        if req.method == 'POST':
            if req.args.get('action') == 'invalidate':
                self._do_invalidate(req, build, db)
            req.redirect(req.href.build(build.config, build.id))

        add_link(req, 'up', req.href.build(build.config),
                 'Build Configuration')
        data = {'title': 'Build %s - %s' % (build_id,
                                            _status_title[build.status]),
                'page_mode': 'view_build',
                'build': {}}
        config = BuildConfig.fetch(self.env, build.config, db=db)
        data['build']['config'] = {
            'name': config.label,
            'href': req.href.build(config.name)
        }

        formatters = []
        for formatter in self.log_formatters:
            formatters.append(formatter.get_formatter(req, build))

        summarizers = {} # keyed by report type
        for summarizer in self.report_summarizers:
            categories = summarizer.get_supported_categories()
            summarizers.update(dict([(cat, summarizer) for cat in categories]))

        data['build'].update(_get_build_data(self.env, req, build))
        steps = []
        for step in BuildStep.select(self.env, build=build.id, db=db):
            steps.append({
                'name': step.name, 'description': step.description,
                'duration': pretty_timedelta(step.started, step.stopped),
                'failed': step.status == BuildStep.FAILURE,
                'errors': step.errors,
                'log': self._render_log(req, build, formatters, step),
                'reports': self._render_reports(req, config, build, summarizers,
                                                step)
            })
        data['build']['steps'] = steps
        data['build']['can_delete'] = ('BUILD_DELETE' in req.perm)

        repos = self.env.get_repository(req.authname)
        repos.authz.assert_permission(config.path)
        chgset = repos.get_changeset(build.rev)
        data['build']['chgset_author'] = chgset.author

        add_script(req, 'bitten/tabset.js')
        add_stylesheet(req, 'bitten/bitten.css')
        return 'bitten_build.html', data, None

    # ITimelineEventProvider methods

    def get_timeline_filters(self, req):
        if 'BUILD_VIEW' in req.perm:
            yield ('build', 'Builds')

    def get_timeline_events(self, req, start, stop, filters):
        if 'build' not in filters:
            return

        if isinstance(start, datetime): # Trac>=0.11
            from trac.util.datefmt import to_timestamp
            start = to_timestamp(start)
            stop = to_timestamp(stop)

        add_stylesheet(req, 'bitten/bitten.css')

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT b.id,b.config,c.label,b.rev,p.name,"
                       "b.stopped,b.status FROM bitten_build AS b"
                       "  INNER JOIN bitten_config AS c ON (c.name=b.config) "
                       "  INNER JOIN bitten_platform AS p ON (p.id=b.platform) "
                       "WHERE b.stopped>=%s AND b.stopped<=%s "
                       "AND b.status IN (%s, %s) ORDER BY b.stopped",
                       (start, stop, Build.SUCCESS, Build.FAILURE))

        repos = self.env.get_repository(req.authname)
        if hasattr(repos, 'sync'):
            repos.sync()

        event_kinds = {Build.SUCCESS: 'successbuild',
                       Build.FAILURE: 'failedbuild'}
        for id, config, label, rev, platform, stopped, status in cursor:

            config_object = BuildConfig.fetch(self.env, config, db=db)
            if not repos.authz.has_permission(config_object.path):
                continue
            
            errors = []
            if status == Build.FAILURE:
                for step in BuildStep.select(self.env, build=id,
                                             status=BuildStep.FAILURE,
                                             db=db):
                    errors += [(step.name, error) for error
                               in step.errors]

            title = tag('Build of ', tag.em('%s [%s]' % (label, rev)),
                        ' on %s %s' % (platform, _status_label[status]))
            message = ''
            if req.args.get('format') == 'rss':
                href = req.abs_href.build(config, id)
                if errors:
                    buf = StringIO()
                    prev_step = None
                    for step, error in errors:
                        if step != prev_step:
                            if prev_step is not None:
                                buf.write('</ul>')
                            buf.write('<p>Step %s failed:</p><ul>' \
                                      % escape(step))
                            prev_step = step
                        buf.write('<li>%s</li>' % escape(error))
                    buf.write('</ul>')
                    message = Markup(buf.getvalue())
            else:
                href = req.href.build(config, id)
                if errors:
                    steps = []
                    for step, error in errors:
                        if step not in steps:
                            steps.append(step)
                    steps = [Markup('<em>%s</em>') % step for step in steps]
                    if len(steps) < 2:
                        message = steps[0]
                    elif len(steps) == 2:
                        message = Markup(' and ').join(steps)
                    elif len(steps) > 2:
                        message = Markup(', ').join(steps[:-1]) + ', and ' + \
                                  steps[-1]
                    message = Markup('Step%s %s failed') % (
                        len(steps) != 1 and 's' or '', message
                    )
            yield event_kinds[status], href, title, stopped, None, message

    # Internal methods

    def _do_invalidate(self, req, build, db):
        self.log.info('Invalidating build %d', build.id)

        for step in BuildStep.select(self.env, build=build.id, db=db):
            step.delete(db=db)

        build.slave = None
        build.started = build.stopped = 0
        build.status = Build.PENDING
        build.slave_info = {}
        build.update()

        db.commit()

        req.redirect(req.href.build(build.config))

    def _render_log(self, req, build, formatters, step):
        items = []
        for log in BuildLog.select(self.env, build=build.id, step=step.name):
            for level, message in log.messages:
                for format in formatters:
                    message = format(step, log.generator, level, message)
                items.append({'level': level, 'message': message})
        return items

    def _render_reports(self, req, config, build, summarizers, step):
        reports = []
        for report in Report.select(self.env, build=build.id, step=step.name):
            summarizer = summarizers.get(report.category)
            if summarizer:
                tmpl, data = summarizer.render_summary(req, config, build,
                                                        step, report.category)
            else:
                tmpl = data = None
            reports.append({'category': report.category,
                            'template': tmpl, 'data': data})
        return reports


class ReportChartController(Component):
    implements(IRequestHandler)

    generators = ExtensionPoint(IReportChartGenerator)

    # IRequestHandler methods

    def match_request(self, req):
        match = re.match(r'/build/([\w.-]+)/chart/(\w+)', req.path_info)
        if match:
            req.args['config'] = match.group(1)
            req.args['category'] = match.group(2)
            return True

    def process_request(self, req):
        category = req.args.get('category')
        config = BuildConfig.fetch(self.env, name=req.args.get('config'))

        for generator in self.generators:
            if category in generator.get_supported_categories():
                tmpl, data = generator.generate_chart_data(req, config,
                                                           category)
                break
        else:
            raise TracError('Unknown report category "%s"' % category)

        return tmpl, data, 'text/xml'


class SourceFileLinkFormatter(Component):
    """Detects references to files in the build log and renders them as links
    to the repository browser.
    """

    implements(ILogFormatter)

    _fileref_re = re.compile('(?P<prefix>-[A-Za-z])?(?P<path>[\w.-]+(?:/[\w.-]+)+)(?P<line>(:\d+))?')

    def get_formatter(self, req, build):
        """Return the log message formatter function."""
        config = BuildConfig.fetch(self.env, name=build.config)
        repos = self.env.get_repository(req.authname)
        href = req.href.browser
        cache = {}

        def _replace(m):
            filepath = posixpath.normpath(m.group('path').replace('\\', '/'))
            if not cache.get(filepath) is True:
                parts = filepath.split('/')
                path = ''
                for part in parts:
                    path = posixpath.join(path, part)
                    if path not in cache:
                        try:
                            full_path = posixpath.join(config.path, path)
                            full_path = posixpath.normpath(full_path)
                            if full_path.startswith(config.path + "/") or full_path == config.path:
                                repos.get_node(full_path,
                                               build.rev)
                                cache[path] = True
                            else:
                                cache[path] = False
                        except TracError:
                            cache[path] = False
                    if cache[path] is False:
                        return m.group(0)
            link = href(config.path, filepath)
            if m.group('line'):
                link += '#L' + m.group('line')[1:]
            return Markup(tag.a(m.group(0), href=link))

        def _formatter(step, type, level, message):
            buf = []
            offset = 0
            for mo in self._fileref_re.finditer(message):
                start, end = mo.span()
                if start > offset:
                    buf.append(message[offset:start])
                buf.append(_replace(mo))
                offset = end
            if offset < len(message):
                buf.append(message[offset:])
            return Markup("").join(buf)

        return _formatter
