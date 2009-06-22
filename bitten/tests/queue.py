# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

import os
import shutil
import tempfile
import time
import unittest

from trac.db import DatabaseManager
from trac.test import EnvironmentStub, Mock
from bitten.model import BuildConfig, TargetPlatform, Build, schema
from bitten.queue import BuildQueue, collect_changes


class CollectChangesTestCase(unittest.TestCase):
    """
    Unit tests for the `bitten.queue.collect_changes` function.
    """

    def setUp(self):
        self.env = EnvironmentStub()
        self.env.path = tempfile.mkdtemp()

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        connector, _ = DatabaseManager(self.env)._get_connector()
        for table in schema:
            for stmt in connector.to_sql(table):
                cursor.execute(stmt)

        self.config = BuildConfig(self.env, name='test', path='somepath')
        self.config.insert(db=db)
        self.platform = TargetPlatform(self.env, config='test', name='Foo')
        self.platform.insert(db=db)
        db.commit()

    def tearDown(self):
        shutil.rmtree(self.env.path)

    def test_stop_on_copy(self):
        self.env.get_repository = lambda authname=None: Mock(
            get_node=lambda path, rev=None: Mock(
                get_history=lambda: [('otherpath', 123, 'copy')]
            ),
            normalize_path=lambda path: path
        )

        retval = list(collect_changes(self.env.get_repository(), self.config))
        self.assertEqual(0, len(retval))

    def test_stop_on_minrev(self):
        self.env.get_repository = lambda authname=None: Mock(
            get_node=lambda path, rev=None: Mock(
                get_entries=lambda: [Mock(), Mock()],
                get_history=lambda: [('somepath', 123, 'edit'),
                                     ('somepath', 121, 'edit'),
                                     ('somepath', 120, 'edit')]
            ),
            normalize_path=lambda path: path,
            rev_older_than=lambda rev1, rev2: rev1 < rev2
        )

        self.config.min_rev = 123
        self.config.update()

        retval = list(collect_changes(self.env.get_repository(), self.config))
        self.assertEqual(1, len(retval))
        self.assertEqual(123, retval[0][1])

    def test_skip_until_maxrev(self):
        self.env.get_repository = lambda authname=None: Mock(
            get_node=lambda path, rev=None: Mock(
                get_entries=lambda: [Mock(), Mock()],
                get_history=lambda: [('somepath', 123, 'edit'),
                                     ('somepath', 121, 'edit'),
                                     ('somepath', 120, 'edit')]
            ),
            normalize_path=lambda path: path,
            rev_older_than=lambda rev1, rev2: rev1 < rev2
        )

        self.config.max_rev=121
        self.config.update()

        retval = list(collect_changes(self.env.get_repository(), self.config))
        self.assertEqual(2, len(retval))
        self.assertEqual(121, retval[0][1])
        self.assertEqual(120, retval[1][1])

    def test_skip_empty_dir(self):
        def _mock_get_node(path, rev=None):
            if rev and rev == 121:
                return Mock(
                    get_entries=lambda: []
                )
            else:
                return Mock(
                    get_entries=lambda: [Mock(), Mock()],
                    get_history=lambda: [('somepath', 123, 'edit'),
                                         ('somepath', 121, 'edit'),
                                         ('somepath', 120, 'edit')]
                )

        self.env.get_repository = lambda authname=None: Mock(
            get_node=_mock_get_node,
            normalize_path=lambda path: path,
            rev_older_than=lambda rev1, rev2: rev1 < rev2
        )

        retval = list(collect_changes(self.env.get_repository(), self.config))
        self.assertEqual(2, len(retval))
        self.assertEqual(123, retval[0][1])
        self.assertEqual(120, retval[1][1])


class BuildQueueTestCase(unittest.TestCase):

    def setUp(self):
        self.env = EnvironmentStub()
        self.env.path = tempfile.mkdtemp()
        os.mkdir(os.path.join(self.env.path, 'snapshots'))

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        connector, _ = DatabaseManager(self.env)._get_connector()
        for table in schema:
            for stmt in connector.to_sql(table):
                cursor.execute(stmt)
        db.commit()

        # Hook up a dummy repository
        self.repos = Mock()
        self.env.get_repository = lambda authname=None: self.repos

    def tearDown(self):
        shutil.rmtree(self.env.path)

    def test_get_build_for_slave(self):
        """
        Make sure that a pending build of an activated configuration is
        scheduled for a slave that matches the target platform.
        """
        BuildConfig(self.env, 'test', active=True).insert()
        platform = TargetPlatform(self.env, config='test', name='Foo')
        platform.insert()
        build = Build(self.env, config='test', platform=platform.id, rev=123,
                      rev_time=42, status=Build.PENDING)
        build.insert()
        build_id = build.id

        queue = BuildQueue(self.env)
        build = queue.get_build_for_slave('foobar', {})
        self.assertEqual(build_id, build.id)

    def test_next_pending_build_no_matching_slave(self):
        """
        Make sure that builds for which there is no slave matching the target
        platform are not scheduled.
        """
        BuildConfig(self.env, 'test', active=True).insert()
        build = Build(self.env, config='test', platform=1, rev=123, rev_time=42,
                      status=Build.PENDING)
        build.insert()
        build_id = build.id

        queue = BuildQueue(self.env)
        build = queue.get_build_for_slave('foobar', {})
        self.assertEqual(None, build)

    def test_next_pending_build_inactive_config(self):
        """
        Make sure that builds for a deactived build config are not scheduled.
        """
        BuildConfig(self.env, 'test').insert()
        platform = TargetPlatform(self.env, config='test', name='Foo')
        platform.insert()
        build = Build(self.env, config='test', platform=platform.id, rev=123,
                      rev_time=42, status=Build.PENDING)
        build.insert()

        queue = BuildQueue(self.env)
        build = queue.get_build_for_slave('foobar', {})
        self.assertEqual(None, build)

    def test_populate_not_build_all(self):
        self.env.get_repository = lambda authname=None: Mock(
            get_changeset=lambda rev: Mock(date=rev * 1000),
            get_node=lambda path, rev=None: Mock(
                get_entries=lambda: [Mock(), Mock()],
                get_history=lambda: [('somepath', 123, 'edit'),
                                     ('somepath', 121, 'edit'),
                                     ('somepath', 120, 'edit')]
            ),
            normalize_path=lambda path: path,
            rev_older_than=lambda rev1, rev2: rev1 < rev2
        )
        BuildConfig(self.env, 'test', path='somepath', active=True).insert()
        platform1 = TargetPlatform(self.env, config='test', name='P1')
        platform1.insert()
        platform2 = TargetPlatform(self.env, config='test', name='P2')
        platform2.insert()

        queue = BuildQueue(self.env)
        queue.populate()
        queue.populate()
        queue.populate()

        builds = list(Build.select(self.env, config='test'))
        builds.sort(lambda a, b: cmp(a.platform, b.platform))
        self.assertEqual(2, len(builds))
        self.assertEqual(platform1.id, builds[0].platform)
        self.assertEqual('123', builds[0].rev)
        self.assertEqual(platform2.id, builds[1].platform)
        self.assertEqual('123', builds[1].rev)

    def test_populate_build_all(self):
        self.env.get_repository = lambda authname=None: Mock(
            get_changeset=lambda rev: Mock(date=rev * 1000),
            get_node=lambda path, rev=None: Mock(
                get_entries=lambda: [Mock(), Mock()],
                get_history=lambda: [('somepath', 123, 'edit'),
                                     ('somepath', 121, 'edit'),
                                     ('somepath', 120, 'edit')]
            ),
            normalize_path=lambda path: path,
            rev_older_than=lambda rev1, rev2: rev1 < rev2
        )
        BuildConfig(self.env, 'test', path='somepath', active=True).insert()
        platform1 = TargetPlatform(self.env, config='test', name='P1')
        platform1.insert()
        platform2 = TargetPlatform(self.env, config='test', name='P2')
        platform2.insert()

        queue = BuildQueue(self.env, build_all=True)
        queue.populate()
        queue.populate()
        queue.populate()

        builds = list(Build.select(self.env, config='test'))
        builds.sort(lambda a, b: cmp(a.platform, b.platform))
        self.assertEqual(6, len(builds))
        self.assertEqual(platform1.id, builds[0].platform)
        self.assertEqual('123', builds[0].rev)
        self.assertEqual(platform1.id, builds[1].platform)
        self.assertEqual('121', builds[1].rev)
        self.assertEqual(platform1.id, builds[2].platform)
        self.assertEqual('120', builds[2].rev)
        self.assertEqual(platform2.id, builds[3].platform)
        self.assertEqual('123', builds[3].rev)
        self.assertEqual(platform2.id, builds[4].platform)
        self.assertEqual('121', builds[4].rev)
        self.assertEqual(platform2.id, builds[5].platform)
        self.assertEqual('120', builds[5].rev)

    def test_reset_orphaned_builds(self):
        BuildConfig(self.env, 'test').insert()
        platform = TargetPlatform(self.env, config='test', name='Foo')
        platform.insert()
        build1 = Build(self.env, config='test', platform=platform.id, rev=123,
                      rev_time=42, status=Build.IN_PROGRESS, slave='heinz',
                      started=time.time() - 600) # Started ten minutes ago
        build1.insert()

        build2 = Build(self.env, config='test', platform=platform.id, rev=124,
                       rev_time=42, status=Build.IN_PROGRESS, slave='heinz',
                       started=time.time() - 60) # Started a minute ago
        build2.insert()

        queue = BuildQueue(self.env, timeout=300) # 5 minutes timeout
        build = queue.reset_orphaned_builds()
        self.assertEqual(Build.PENDING, Build.fetch(self.env, build1.id).status)
        self.assertEqual(Build.IN_PROGRESS,
                         Build.fetch(self.env, build2.id).status)

    def test_match_slave_match(self):
        BuildConfig(self.env, 'test', active=True).insert()
        platform = TargetPlatform(self.env, config='test', name="Unix")
        platform.rules.append(('family', 'posix'))
        platform.insert()
        platform_id = platform.id

        queue = BuildQueue(self.env)
        platforms = queue.match_slave('foo', {'family': 'posix'})
        self.assertEqual(1, len(platforms))
        self.assertEqual(platform_id, platforms[0].id)

    def test_register_slave_match_simple_fail(self):
        BuildConfig(self.env, 'test', active=True).insert()
        platform = TargetPlatform(self.env, config='test', name="Unix")
        platform.rules.append(('family', 'posix'))
        platform.insert()

        queue = BuildQueue(self.env)
        platforms = queue.match_slave('foo', {'family': 'nt'})
        self.assertEqual([], platforms)

    def test_register_slave_match_regexp(self):
        BuildConfig(self.env, 'test', active=True).insert()
        platform = TargetPlatform(self.env, config='test', name="Unix")
        platform.rules.append(('version', '8\.\d\.\d'))
        platform.insert()
        platform_id = platform.id

        queue = BuildQueue(self.env)
        platforms = queue.match_slave('foo', {'version': '8.2.0'})
        self.assertEqual(1, len(platforms))
        self.assertEqual(platform_id, platforms[0].id)

    def test_register_slave_match_regexp_multi(self):
        BuildConfig(self.env, 'test', active=True).insert()
        platform = TargetPlatform(self.env, config='test', name="Unix")
        platform.rules.append(('os', '^Linux'))
        platform.rules.append(('processor', '^[xi]\d?86$'))
        platform.insert()
        platform_id = platform.id

        queue = BuildQueue(self.env)
        platforms = queue.match_slave('foo', {'os': 'Linux', 'processor': 'i686'})
        self.assertEqual(1, len(platforms))
        self.assertEqual(platform_id, platforms[0].id)

    def test_register_slave_match_regexp_fail(self):
        BuildConfig(self.env, 'test', active=True).insert()
        platform = TargetPlatform(self.env, config='test', name="Unix")
        platform.rules.append(('version', '8\.\d\.\d'))
        platform.insert()

        queue = BuildQueue(self.env)
        platforms = queue.match_slave('foo', {'version': '7.8.1'})
        self.assertEqual([], platforms)

    def test_register_slave_match_regexp_invalid(self):
        BuildConfig(self.env, 'test', active=True).insert()
        platform = TargetPlatform(self.env, config='test', name="Unix")
        platform.rules.append(('version', '8(\.\d'))
        platform.insert()

        queue = BuildQueue(self.env)
        platforms = queue.match_slave('foo', {'version': '7.8.1'})
        self.assertEqual([], platforms)


def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(CollectChangesTestCase, 'test'))
    suite.addTest(unittest.makeSuite(BuildQueueTestCase, 'test'))
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
