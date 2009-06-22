# -*- coding: utf-8 -*-
#
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
from trac.perm import PermissionCache, PermissionError, PermissionSystem
from trac.test import EnvironmentStub, Mock
from trac.web.href import Href
from trac.web.main import RequestDone
from bitten.main import BuildSystem
from bitten.model import BuildConfig, TargetPlatform, schema
from bitten.admin import BuildMasterAdminPageProvider, \
                         BuildConfigurationsAdminPageProvider

try:
    from trac.perm import DefaultPermissionPolicy
except ImportError:
    DefaultPermissionPolicy = None

class BuildMasterAdminPageProviderTestCase(unittest.TestCase):

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
        PermissionSystem(self.env).grant_permission('joe', 'BUILD_ADMIN')
        if DefaultPermissionPolicy is not None and hasattr(DefaultPermissionPolicy, "CACHE_EXPIRY"):
            self.old_perm_cache_expiry = DefaultPermissionPolicy.CACHE_EXPIRY
            DefaultPermissionPolicy.CACHE_EXPIRY = -1

        # Hook up a dummy repository
        self.repos = Mock(
            get_node=lambda path, rev=None: Mock(get_history=lambda: [],
                                                 isdir=True),
            normalize_path=lambda path: path,
            sync=lambda: None
        )
        self.env.get_repository = lambda authname=None: self.repos

    def tearDown(self):
        if DefaultPermissionPolicy is not None and hasattr(DefaultPermissionPolicy, "CACHE_EXPIRY"):
            DefaultPermissionPolicy.CACHE_EXPIRY = self.old_perm_cache_expiry
        shutil.rmtree(self.env.path)

    def test_get_admin_panels(self):
        provider = BuildMasterAdminPageProvider(self.env)

        req = Mock(perm=PermissionCache(self.env, 'joe'))
        self.assertEqual([('bitten', 'Builds', 'master', 'Master Settings')],
                         list(provider.get_admin_panels(req)))

        PermissionSystem(self.env).revoke_permission('joe', 'BUILD_ADMIN')
        req = Mock(perm=PermissionCache(self.env, 'joe'))
        self.assertEqual([], list(provider.get_admin_panels(req)))

    def test_process_get_request(self):
        req = Mock(method='GET', chrome={}, href=Href('/'),
                   perm=PermissionCache(self.env, 'joe'))

        provider = BuildMasterAdminPageProvider(self.env)
        template_name, data = provider.render_admin_panel(
            req, 'bitten', 'master', ''
        )

        self.assertEqual('bitten_admin_master.html', template_name)
        assert 'master' in data
        master = data['master']
        self.assertEqual(3600, master.slave_timeout)
        self.assertEqual(0, master.stabilize_wait)
        assert not master.adjust_timestamps
        assert not master.build_all
        self.assertEqual('log/bitten', master.logs_dir)

    def test_process_config_changes(self):
        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   args={'slave_timeout': '60', 'adjust_timestamps': ''})

        provider = BuildMasterAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'master', '')
            self.fail('Expected RequestDone')

        except RequestDone:
            self.assertEqual('http://example.org/admin/bitten/master',
                             redirected_to[0])
            section = self.env.config['bitten']
            self.assertEqual(60, section.getint('slave_timeout'))
            self.assertEqual(True, section.getbool('adjust_timestamps'))
            self.assertEqual(False, section.getbool('build_all'))


class BuildConfigurationsAdminPageProviderTestCase(unittest.TestCase):

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
        PermissionSystem(self.env).grant_permission('joe', 'BUILD_CREATE')
        PermissionSystem(self.env).grant_permission('joe', 'BUILD_DELETE')
        PermissionSystem(self.env).grant_permission('joe', 'BUILD_MODIFY')
        if DefaultPermissionPolicy is not None and hasattr(DefaultPermissionPolicy, "CACHE_EXPIRY"):
            self.old_perm_cache_expiry = DefaultPermissionPolicy.CACHE_EXPIRY
            DefaultPermissionPolicy.CACHE_EXPIRY = -1

        # Hook up a dummy repository
        self.repos = Mock(
            get_node=lambda path, rev=None: Mock(get_history=lambda: [],
                                                 isdir=True),
            normalize_path=lambda path: path,
            sync=lambda: None
        )
        self.env.get_repository = lambda authname=None: self.repos

    def tearDown(self):
        if DefaultPermissionPolicy is not None and hasattr(DefaultPermissionPolicy, "CACHE_EXPIRY"):
            DefaultPermissionPolicy.CACHE_EXPIRY = self.old_perm_cache_expiry
        shutil.rmtree(self.env.path)

    def test_get_admin_panels(self):
        provider = BuildConfigurationsAdminPageProvider(self.env)

        req = Mock(perm=PermissionCache(self.env, 'joe'))
        self.assertEqual([('bitten', 'Builds', 'configs', 'Configurations')],
                         list(provider.get_admin_panels(req)))

        PermissionSystem(self.env).revoke_permission('joe', 'BUILD_MODIFY')
        req = Mock(perm=PermissionCache(self.env, 'joe'))
        self.assertEqual([], list(provider.get_admin_panels(req)))

    def test_process_view_configs_empty(self):
        req = Mock(method='GET', chrome={}, href=Href('/'),
                   perm=PermissionCache(self.env, 'joe'))

        provider = BuildConfigurationsAdminPageProvider(self.env)
        template_name, data = provider.render_admin_panel(
            req, 'bitten', 'configs', ''
        )

        self.assertEqual('bitten_admin_configs.html', template_name)
        self.assertEqual([], data['configs'])

    def test_process_view_configs(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()
        BuildConfig(self.env, name='bar', label='Bar', path='branches/bar',
                    min_rev='123', max_rev='456').insert()

        req = Mock(method='GET', chrome={}, href=Href('/'),
                   perm=PermissionCache(self.env, 'joe'))

        provider = BuildConfigurationsAdminPageProvider(self.env)
        template_name, data = provider.render_admin_panel(
            req, 'bitten', 'configs', ''
        )

        self.assertEqual('bitten_admin_configs.html', template_name)
        assert 'configs' in data
        configs = data['configs']
        self.assertEqual(2, len(configs))
        self.assertEqual({
            'name': 'bar', 'href': '/admin/bitten/configs/bar',
            'label': 'Bar', 'min_rev': '123', 'max_rev': '456',
            'path': 'branches/bar', 'active': False
        }, configs[0])
        self.assertEqual({
            'name': 'foo', 'href': '/admin/bitten/configs/foo',
            'label': 'Foo', 'min_rev': None, 'max_rev': None,
            'path': 'branches/foo', 'active': True
        }, configs[1])

    def test_process_view_config(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()
        TargetPlatform(self.env, config='foo', name='any').insert()

        req = Mock(method='GET', chrome={}, href=Href('/'),
                   perm=PermissionCache(self.env, 'joe'))

        provider = BuildConfigurationsAdminPageProvider(self.env)
        template_name, data = provider.render_admin_panel(
            req, 'bitten', 'configs', 'foo'
        )

        self.assertEqual('bitten_admin_configs.html', template_name)
        assert 'config' in data
        config = data['config']
        self.assertEqual({
            'name': 'foo', 'label': 'Foo', 'description': '', 'recipe': '',
            'path': 'branches/foo', 'min_rev': None, 'max_rev': None,
            'active': True, 'platforms': [{
                'href': '/admin/bitten/configs/foo/1',
                'name': 'any', 'id': 1, 'rules': []
            }]
        }, config)

    def test_process_activate_config(self):
        BuildConfig(self.env, name='foo', path='branches/foo').insert()
        BuildConfig(self.env, name='bar', path='branches/bar').insert()

        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   authname='joe',
                   args={'apply': '', 'active': ['foo']})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', '')
            self.fail('Expected RequestDone')

        except RequestDone:
            self.assertEqual('http://example.org/admin/bitten/configs',
                             redirected_to[0])
            config = BuildConfig.fetch(self.env, name='foo')
            self.assertEqual(True, config.active)

    def test_process_deactivate_config(self):
        BuildConfig(self.env, name='foo', path='branches/foo',
                    active=True).insert()
        BuildConfig(self.env, name='bar', path='branches/bar',
                    active=True).insert()

        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   authname='joe',
                   args={'apply': ''})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', '')
            self.fail('Expected RequestDone')

        except RequestDone:
            self.assertEqual('http://example.org/admin/bitten/configs',
                             redirected_to[0])
            config = BuildConfig.fetch(self.env, name='foo')
            self.assertEqual(False, config.active)
            config = BuildConfig.fetch(self.env, name='bar')
            self.assertEqual(False, config.active)

    def test_process_add_config(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()

        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   authname='joe',
                   args={'add': '', 'name': 'bar', 'label': 'Bar'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', '')
            self.fail('Expected RequestDone')

        except RequestDone:
            self.assertEqual('http://example.org/admin/bitten/configs/bar',
                             redirected_to[0])
            config = BuildConfig.fetch(self.env, name='bar')
            self.assertEqual('Bar', config.label)

    def test_process_add_config_cancel(self):
        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   args={'cancel': '', 'name': 'bar', 'label': 'Bar'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', '')
            self.fail('Expected RequestDone')

        except RequestDone:
            self.assertEqual('http://example.org/admin/bitten/configs',
                             redirected_to[0])
            configs = list(BuildConfig.select(self.env, include_inactive=True))
            self.assertEqual(0, len(configs))

    def test_process_add_config_no_name(self):
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   args={'add': ''})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', '')
            self.fail('Expected TracError')

        except TracError, e:
            self.assertEqual('Missing required field "name"', e.message)
            self.assertEqual('Missing Field', e.title)

    def test_process_add_config_invalid_name(self):
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   args={'add': '', 'name': 'no spaces allowed'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', '')
            self.fail('Expected TracError')

        except TracError, e:
            self.assertEqual('The field "name" may only contain letters, '
                             'digits, periods, or dashes.', e.message)
            self.assertEqual('Invalid Field', e.title)

    def test_new_config_submit_with_invalid_path(self):
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   authname='joe',
                   args={'add': '', 'name': 'foo', 'path': 'invalid/path'})

        def get_node(path, rev=None):
            raise TracError('No such node')
        self.repos = Mock(get_node=get_node)

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', '')
            self.fail('Expected TracError')

        except TracError, e:
            self.assertEqual('No such node', e.message)
            self.assertEqual('Invalid Repository Path', e.title)

    def test_process_add_config_no_perms(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()
        PermissionSystem(self.env).revoke_permission('joe', 'BUILD_CREATE')

        req = Mock(method='POST',
                   perm=PermissionCache(self.env, 'joe'),
                   args={'add': '', 'name': 'bar', 'label': 'Bar'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        self.assertRaises(PermissionError, provider.render_admin_panel, req,
                          'bitten', 'configs', '')

    def test_process_remove_config(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()
        BuildConfig(self.env, name='bar', label='Bar', path='branches/bar',
                    min_rev='123', max_rev='456').insert()

        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   args={'remove': '', 'sel': 'bar'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', '')
            self.fail('Expected RequestDone')

        except RequestDone:
            self.assertEqual('http://example.org/admin/bitten/configs',
                             redirected_to[0])
            assert not BuildConfig.fetch(self.env, name='bar')

    def test_process_remove_config_cancel(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()
        BuildConfig(self.env, name='bar', label='Bar', path='branches/bar',
                    min_rev='123', max_rev='456').insert()

        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   args={'cancel': '', 'sel': 'bar'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', '')
            self.fail('Expected RequestDone')

        except RequestDone:
            self.assertEqual('http://example.org/admin/bitten/configs',
                             redirected_to[0])
            configs = list(BuildConfig.select(self.env, include_inactive=True))
            self.assertEqual(2, len(configs))

    def test_process_remove_config_no_selection(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()

        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   args={'remove': ''})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', '')
            self.fail('Expected TracError')

        except TracError, e:
            self.assertEqual('No configuration selected', e.message)

    def test_process_remove_config_bad_selection(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()

        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   args={'remove': '', 'sel': 'baz'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', '')
            self.fail('Expected TracError')

        except TracError, e:
            self.assertEqual("Configuration 'baz' not found", e.message)

    def test_process_remove_config_no_perms(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()
        PermissionSystem(self.env).revoke_permission('joe', 'BUILD_DELETE')

        req = Mock(method='POST',
                   perm=PermissionCache(self.env, 'joe'),
                   args={'remove': '', 'sel': 'bar'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        self.assertRaises(PermissionError, provider.render_admin_panel, req,
                          'bitten', 'configs', '')

    def test_process_update_config(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()

        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   authname='joe', args={
            'save': '', 'name': 'foo', 'label': 'Foobar',
            'description': 'Thanks for all the fish!'
        })

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', 'foo')
            self.fail('Expected RequestDone')

        except RequestDone:
            self.assertEqual('http://example.org/admin/bitten/configs',
                             redirected_to[0])
            config = BuildConfig.fetch(self.env, name='foo')
            self.assertEqual('Foobar', config.label)
            self.assertEqual('Thanks for all the fish!', config.description)

    def test_process_update_config_no_name(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()

        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   args={'save': ''})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', 'foo')
            self.fail('Expected TracError')

        except TracError, e:
            self.assertEqual('Missing required field "name"', e.message)
            self.assertEqual('Missing Field', e.title)

    def test_process_update_config_invalid_name(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()

        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   args={'save': '', 'name': 'no spaces allowed'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', 'foo')
            self.fail('Expected TracError')

        except TracError, e:
            self.assertEqual('The field "name" may only contain letters, '
                             'digits, periods, or dashes.', e.message)
            self.assertEqual('Invalid Field', e.title)

    def test_process_update_config_invalid_path(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()

        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   authname='joe',
                   args={'save': '', 'name': 'foo', 'path': 'invalid/path'})

        def get_node(path, rev=None):
            raise TracError('No such node')
        self.repos = Mock(get_node=get_node)

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', 'foo')
            self.fail('Expected TracError')

        except TracError, e:
            self.assertEqual('No such node', e.message)
            self.assertEqual('Invalid Repository Path', e.title)

    def test_process_update_config_non_wellformed_recipe(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()

        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   authname='joe',
                   args={'save': '', 'name': 'foo', 'recipe': 'not_xml'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', 'foo')
            self.fail('Expected TracError')

        except TracError, e:
            self.assertEqual('Failure parsing recipe: syntax error: line 1, '
                             'column 0', e.message)
            self.assertEqual('Invalid Recipe', e.title)

    def test_process_update_config_invalid_recipe(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()

        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   authname='joe',
                   args={'save': '', 'name': 'foo',
                         'recipe': '<build><step /></build>'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', 'foo')
            self.fail('Expected TracError')

        except TracError, e:
            self.assertEqual('Steps must have an "id" attribute', e.message)
            self.assertEqual('Invalid Recipe', e.title)

    def test_process_new_platform(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()

        data = {}
        req = Mock(method='POST', chrome={}, hdf=data, href=Href('/'),
                   perm=PermissionCache(self.env, 'joe'),
                   args={'new': ''})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        template_name, data = provider.render_admin_panel(
            req, 'bitten', 'configs', 'foo'
        )

        self.assertEqual('bitten_admin_configs.html', template_name)
        assert 'platform' in data
        platform = data['platform']
        self.assertEqual({
            'id': None, 'exists': False, 'name': None, 'rules': [('', '')],
        }, platform)

    def test_process_add_platform(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()

        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   authname='joe',
                   args={'add': '', 'new': '', 'name': 'Test',
                         'property_0': 'family', 'pattern_0': 'posix'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', 'foo')
            self.fail('Expected RequestDone')

        except RequestDone:
            self.assertEqual('http://example.org/admin/bitten/configs/foo',
                             redirected_to[0])
            platforms = list(TargetPlatform.select(self.env, config='foo'))
            self.assertEqual(1, len(platforms))
            self.assertEqual('Test', platforms[0].name)
            self.assertEqual([('family', 'posix')], platforms[0].rules)

    def test_process_add_platform_cancel(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()

        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   authname='joe',
                   args={'cancel': '', 'new': '', 'name': 'Test',
                         'property_0': 'family', 'pattern_0': 'posix'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', 'foo')
            self.fail('Expected RequestDone')

        except RequestDone:
            self.assertEqual('http://example.org/admin/bitten/configs/foo',
                             redirected_to[0])
            platforms = list(TargetPlatform.select(self.env, config='foo'))
            self.assertEqual(0, len(platforms))

    def test_process_remove_platforms(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()
        platform = TargetPlatform(self.env, config='foo', name='any')
        platform.insert()

        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   authname='joe',
                   args={'remove': '', 'sel': str(platform.id)})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', 'foo')
            self.fail('Expected RequestDone')

        except RequestDone:
            self.assertEqual('http://example.org/admin/bitten/configs/foo',
                             redirected_to[0])
            platforms = list(TargetPlatform.select(self.env, config='foo'))
            self.assertEqual(0, len(platforms))

    def test_process_remove_platforms_no_selection(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()
        platform = TargetPlatform(self.env, config='foo', name='any')
        platform.insert()

        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   authname='joe',
                   args={'remove': ''})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs', 'foo')
            self.fail('Expected TracError')

        except TracError, e:
            self.assertEqual('No platform selected', e.message)

    def test_process_edit_platform(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()
        platform = TargetPlatform(self.env, config='foo', name='any')
        platform.insert()

        req = Mock(method='GET', chrome={}, href=Href('/'),
                   perm=PermissionCache(self.env, 'joe'), args={})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        template_name, data = provider.render_admin_panel(
            req, 'bitten', 'configs', 'foo/%d' % platform.id
        )

        self.assertEqual('bitten_admin_configs.html', template_name)
        assert 'platform' in data
        platform = data['platform']
        self.assertEqual({
            'id': 1, 'exists': True, 'name': 'any', 'rules': [('', '')],
        }, platform)

    def test_process_update_platform(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()
        platform = TargetPlatform(self.env, config='foo', name='any')
        platform.insert()

        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   authname='joe',
                   args={'save': '', 'edit': '', 'name': 'Test',
                         'property_0': 'family', 'pattern_0': 'posix'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs',
                                           'foo/%d' % platform.id)
            self.fail('Expected RequestDone')

        except RequestDone:
            self.assertEqual('http://example.org/admin/bitten/configs/foo',
                             redirected_to[0])
            platforms = list(TargetPlatform.select(self.env, config='foo'))
            self.assertEqual(1, len(platforms))
            self.assertEqual('Test', platforms[0].name)
            self.assertEqual([('family', 'posix')], platforms[0].rules)

    def test_process_update_platform_cancel(self):
        BuildConfig(self.env, name='foo', label='Foo', path='branches/foo',
                    active=True).insert()
        platform = TargetPlatform(self.env, config='foo', name='any')
        platform.insert()

        redirected_to = []
        def redirect(url):
            redirected_to.append(url)
            raise RequestDone
        req = Mock(method='POST', perm=PermissionCache(self.env, 'joe'),
                   abs_href=Href('http://example.org/'), redirect=redirect,
                   authname='joe',
                   args={'cancel': '', 'edit': '', 'name': 'Changed',
                         'property_0': 'family', 'pattern_0': 'posix'})

        provider = BuildConfigurationsAdminPageProvider(self.env)
        try:
            provider.render_admin_panel(req, 'bitten', 'configs',
                                           'foo/%d' % platform.id)
            self.fail('Expected RequestDone')

        except RequestDone:
            self.assertEqual('http://example.org/admin/bitten/configs/foo',
                             redirected_to[0])
            platforms = list(TargetPlatform.select(self.env, config='foo'))
            self.assertEqual(1, len(platforms))
            self.assertEqual('any', platforms[0].name)
            self.assertEqual([], platforms[0].rules)


def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(
        BuildMasterAdminPageProviderTestCase, 'test'
    ))
    suite.addTest(unittest.makeSuite(
        BuildConfigurationsAdminPageProviderTestCase, 'test'
    ))
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
