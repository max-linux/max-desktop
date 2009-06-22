# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

import shutil
import tempfile
import unittest

from trac.core import TracError
from trac.db import DatabaseManager
from trac.perm import PermissionCache, PermissionSystem
from trac.test import EnvironmentStub, Mock
from trac.util.html import Markup
from trac.web.href import Href
from bitten.main import BuildSystem
from bitten.model import Build, BuildConfig, BuildStep, TargetPlatform, schema
from bitten.web_ui import BuildConfigController, SourceFileLinkFormatter


class BuildConfigControllerTestCase(unittest.TestCase):

    def setUp(self):
        self.env = EnvironmentStub(enable=['trac.*', 'bitten.*'])
        self.env.path = tempfile.mkdtemp()

        # Create tables
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        connector, _ = DatabaseManager(self.env)._get_connector()
        for table in schema:
            for stmt in connector.to_sql(table):
                cursor.execute(stmt)

        # Set up permissions
        self.env.config.set('trac', 'permission_store',
                            'DefaultPermissionStore')

        # Hook up a dummy repository
        self.repos = Mock(
            get_node=lambda path, rev=None: Mock(get_history=lambda: [],
                                                 isdir=True),
            normalize_path=lambda path: path,
            sync=lambda: None,
        )
        self.repos.authz = Mock(has_permission=lambda path: True, assert_permission=lambda path: None)
        self.env.get_repository = lambda authname=None: self.repos

    def tearDown(self):
        shutil.rmtree(self.env.path)

    def test_overview(self):
        PermissionSystem(self.env).grant_permission('joe', 'BUILD_VIEW')
        req = Mock(method='GET', base_path='', cgi_location='',
                   path_info='/build', href=Href('/trac'), args={}, chrome={},
                   perm=PermissionCache(self.env, 'joe'), authname='joe')

        module = BuildConfigController(self.env)
        assert module.match_request(req)
        _, data, _ = module.process_request(req)

        self.assertEqual('overview', data['page_mode'])

    def test_view_config(self):
        config = BuildConfig(self.env, name='test', path='trunk')
        config.insert()
        platform = TargetPlatform(self.env, config='test', name='any')
        platform.insert()

        PermissionSystem(self.env).grant_permission('joe', 'BUILD_VIEW')
        req = Mock(method='GET', base_path='', cgi_location='',
                   path_info='/build/test', href=Href('/trac'), args={},
                   chrome={}, authname='joe',
                   perm=PermissionCache(self.env, 'joe'))

        root = Mock(get_entries=lambda: ['foo'],
                    get_history=lambda: [('trunk', rev, 'edit') for rev in
                                          range(123, 111, -1)])
        self.repos = Mock(get_node=lambda path, rev=None: root,
                          sync=lambda: None, normalize_path=lambda path: path)
        self.repos.authz = Mock(has_permission=lambda path: True, assert_permission=lambda path: None)

        module = BuildConfigController(self.env)
        assert module.match_request(req)
        _, data, _ = module.process_request(req)

        self.assertEqual('view_config', data['page_mode'])
        assert not 'next' in req.chrome['links']

    def test_view_config_paging(self):
        config = BuildConfig(self.env, name='test', path='trunk')
        config.insert()
        platform = TargetPlatform(self.env, config='test', name='any')
        platform.insert()

        PermissionSystem(self.env).grant_permission('joe', 'BUILD_VIEW')
        req = Mock(method='GET', base_path='', cgi_location='',
                   path_info='/build/test', href=Href('/trac'), args={},
                   chrome={}, authname='joe',
                   perm=PermissionCache(self.env, 'joe'))

        root = Mock(get_entries=lambda: ['foo'],
                    get_history=lambda: [('trunk', rev, 'edit') for rev in
                                          range(123, 110, -1)])
        self.repos = Mock(get_node=lambda path, rev=None: root,
                          sync=lambda: None, normalize_path=lambda path: path)
        self.repos.authz = Mock(has_permission=lambda path: True, assert_permission=lambda path: None)

        module = BuildConfigController(self.env)
        assert module.match_request(req)
        _, data, _ = module.process_request(req)

        if req.chrome:
            self.assertEqual('/trac/build/test?page=2',
                             req.chrome['links']['next'][0]['href'])


class SourceFileLinkFormatterTestCase(unittest.TestCase):

    def setUp(self):
        self.env = EnvironmentStub(enable=['trac.*', 'bitten.*'])

        # Create tables
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        connector, _ = DatabaseManager(self.env)._get_connector()
        for table in schema:
            for stmt in connector.to_sql(table):
                cursor.execute(stmt)

        # Hook up a dummy repository
        self.repos = Mock(
            get_node=lambda path, rev=None: Mock(get_history=lambda: [],
                                                 isdir=True),
            normalize_path=lambda path: path,
            sync=lambda: None
        )
        self.repos.authz = Mock(has_permission=lambda path: True, assert_permission=lambda path: None)
        self.env.get_repository = lambda authname=None: self.repos

    def tearDown(self):
        pass

    def test_format_simple_link_in_repos(self):
        BuildConfig(self.env, name='test', path='trunk').insert()
        build = Build(self.env, config='test', platform=1, rev=123, rev_time=42,
                      status=Build.SUCCESS, slave='hal')
        build.insert()
        step = BuildStep(self.env, build=build.id, name='foo',
                         status=BuildStep.SUCCESS)
        step.insert()

        self.repos.get_node = lambda path, rev: (path, rev)

        req = Mock(method='GET', href=Href('/trac'), authname='hal')
        comp = SourceFileLinkFormatter(self.env)
        formatter = comp.get_formatter(req, build)

        output = formatter(step, None, None, u'error in foo/bar.c: bad')
        self.assertEqual(Markup, type(output))
        self.assertEqual('error in <a href="/trac/browser/trunk/foo/bar.c">'
                         'foo/bar.c</a>: bad', output)

    def test_format_bad_links(self):
        BuildConfig(self.env, name='test', path='trunk').insert()
        build = Build(self.env, config='test', platform=1, rev=123, rev_time=42,
                      status=Build.SUCCESS, slave='hal')
        build.insert()
        step = BuildStep(self.env, build=build.id, name='foo',
                         status=BuildStep.SUCCESS)
        step.insert()

        self.repos.get_node = lambda path, rev: (path, rev)

        req = Mock(method='GET', href=Href('/trac'), authname='hal')
        comp = SourceFileLinkFormatter(self.env)
        formatter = comp.get_formatter(req, build)

        output = formatter(step, None, None, u'Linking -I../.. with ../libtool')
        self.assertEqual(Markup, type(output))
        self.assertEqual('Linking -I../.. with ../libtool', output)

    def test_format_simple_link_not_in_repos(self):
        BuildConfig(self.env, name='test', path='trunk').insert()
        build = Build(self.env, config='test', platform=1, rev=123, rev_time=42,
                      status=Build.SUCCESS, slave='hal')
        build.insert()
        step = BuildStep(self.env, build=build.id, name='foo',
                         status=BuildStep.SUCCESS)
        step.insert()

        def _raise():
            raise TracError('No such node')
        self.repos.get_node = lambda path, rev: _raise()

        req = Mock(method='GET', href=Href('/trac'), authname='hal')
        comp = SourceFileLinkFormatter(self.env)
        formatter = comp.get_formatter(req, build)

        output = formatter(step, None, None, u'error in foo/bar.c: bad')
        self.assertEqual(Markup, type(output))
        self.assertEqual('error in foo/bar.c: bad', output)

    def test_format_link_in_repos_with_line(self):
        BuildConfig(self.env, name='test', path='trunk').insert()
        build = Build(self.env, config='test', platform=1, rev=123, rev_time=42,
                      status=Build.SUCCESS, slave='hal')
        build.insert()
        step = BuildStep(self.env, build=build.id, name='foo',
                         status=BuildStep.SUCCESS)
        step.insert()

        self.repos.get_node = lambda path, rev: (path, rev)

        req = Mock(method='GET', href=Href('/trac'), authname='hal')
        comp = SourceFileLinkFormatter(self.env)
        formatter = comp.get_formatter(req, build)

        output = formatter(step, None, None, u'error in foo/bar.c:123: bad')
        self.assertEqual(Markup, type(output))
        self.assertEqual('error in <a href="/trac/browser/trunk/foo/bar.c#L123">'
                         'foo/bar.c:123</a>: bad', output)

    def test_format_link_not_in_repos_with_line(self):
        BuildConfig(self.env, name='test', path='trunk').insert()
        build = Build(self.env, config='test', platform=1, rev=123, rev_time=42,
                      status=Build.SUCCESS, slave='hal')
        build.insert()
        step = BuildStep(self.env, build=build.id, name='foo',
                         status=BuildStep.SUCCESS)
        step.insert()

        def _raise():
            raise TracError('No such node')
        self.repos.get_node = lambda path, rev: _raise()

        req = Mock(method='GET', href=Href('/trac'), authname='hal')
        comp = SourceFileLinkFormatter(self.env)
        formatter = comp.get_formatter(req, build)

        output = formatter(step, None, None, u'error in foo/bar.c:123: bad')
        self.assertEqual(Markup, type(output))
        self.assertEqual('error in foo/bar.c:123: bad', output)


def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(BuildConfigControllerTestCase, 'test'))
    suite.addTest(unittest.makeSuite(SourceFileLinkFormatterTestCase, 'test'))
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
