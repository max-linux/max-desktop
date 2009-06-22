# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

import unittest

from trac.db import DatabaseManager
from trac.test import EnvironmentStub
from bitten.model import BuildConfig, TargetPlatform, Build, BuildStep, \
                         BuildLog, Report, schema
import os
import tempfile


class BuildConfigTestCase(unittest.TestCase):

    def setUp(self):
        self.env = EnvironmentStub()
        self.env.path = tempfile.mkdtemp()
        logs_dir = self.env.config.get("bitten", "logs_dir")
        if os.path.isabs(logs_dir):
            raise ValueError("Should not have absolute logs directory for temporary test")
        logs_dir = os.path.join(self.env.path, logs_dir)
        os.makedirs(logs_dir)

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        connector, _ = DatabaseManager(self.env)._get_connector()
        for table in schema:
            for stmt in connector.to_sql(table):
                cursor.execute(stmt)
        db.commit()

    def test_new(self):
        config = BuildConfig(self.env, name='test')
        assert not config.exists

    def test_fetch(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_config (name,path,label,active) "
                       "VALUES (%s,%s,%s,%s)", ('test', 'trunk', 'Test', 0))
        config = BuildConfig.fetch(self.env, name='test')
        assert config.exists
        self.assertEqual('test', config.name)
        self.assertEqual('trunk', config.path)
        self.assertEqual('Test', config.label)
        self.assertEqual(False, config.active)

    def test_fetch_none(self):
        config = BuildConfig.fetch(self.env, name='test')
        self.assertEqual(None, config)

    def test_select_none(self):
        configs = BuildConfig.select(self.env)
        self.assertRaises(StopIteration, configs.next)

    def test_select_none(self):
        configs = BuildConfig.select(self.env)
        self.assertRaises(StopIteration, configs.next)

    def test_insert(self):
        config = BuildConfig(self.env, name='test', path='trunk', label='Test')
        config.insert()

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT name,path,label,active,description "
                       "FROM bitten_config")
        self.assertEqual(('test', 'trunk', 'Test', 0, ''), cursor.fetchone())

    def test_insert_no_name(self):
        config = BuildConfig(self.env)
        self.assertRaises(AssertionError, config.insert)

    def test_update(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_config (name,path,label,active) "
                       "VALUES (%s,%s,%s,%s)", ('test', 'trunk', 'Test', 0))

        config = BuildConfig.fetch(self.env, 'test')
        config.path = 'some_branch'
        config.label = 'Updated'
        config.active = True
        config.description = 'Bla bla bla'
        config.update()

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT name,path,label,active,description "
                       "FROM bitten_config")
        self.assertEqual(('test', 'some_branch', 'Updated', 1, 'Bla bla bla'),
                         cursor.fetchone())
        self.assertEqual(None, cursor.fetchone())

    def test_update_name(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_config (name,path,label,active) "
                       "VALUES (%s,%s,%s,%s)", ('test', 'trunk', 'Test', 0))

        config = BuildConfig.fetch(self.env, 'test')
        config.name = 'foobar'
        config.update()

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT name,path,label,active,description "
                       "FROM bitten_config")
        self.assertEqual(('foobar', 'trunk', 'Test', 0, ''), cursor.fetchone())
        self.assertEqual(None, cursor.fetchone())

    def test_update_no_name(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_config (name,path,label,active) "
                       "VALUES (%s,%s,%s,%s)", ('test', 'trunk', 'Test', 0))

        config = BuildConfig.fetch(self.env, 'test')
        config.name = None
        self.assertRaises(AssertionError, config.update)

    def test_update_name_with_platform(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_config (name,path,label,active) "
                       "VALUES (%s,%s,%s,%s)", ('test', 'trunk', 'Test', 0))
        cursor.execute("INSERT INTO bitten_platform (config,name) "
                       "VALUES (%s,%s)", ('test', 'NetBSD'))

        config = BuildConfig.fetch(self.env, 'test')
        config.name = 'foobar'
        config.update()

        cursor.execute("SELECT config,name FROM bitten_platform")
        self.assertEqual(('foobar', 'NetBSD'), cursor.fetchone())
        self.assertEqual(None, cursor.fetchone())

    def test_delete(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_config (name,path,label,active) "
                       "VALUES (%s,%s,%s,%s)", ('test', 'trunk', 'Test', 0))

        config = BuildConfig.fetch(self.env, 'test')
        config.delete()
        self.assertEqual(False, config.exists)

        cursor.execute("SELECT * FROM bitten_config WHERE name=%s", ('test',))
        self.assertEqual(None, cursor.fetchone())

    def test_delete_non_existing(self):
        config = BuildConfig(self.env, 'test')
        self.assertRaises(AssertionError, config.delete)


class TargetPlatformTestCase(unittest.TestCase):

    def setUp(self):
        self.env = EnvironmentStub()
        self.env.path = tempfile.mkdtemp()
        logs_dir = self.env.config.get("bitten", "logs_dir")
        if os.path.isabs(logs_dir):
            raise ValueError("Should not have absolute logs directory for temporary test")
        logs_dir = os.path.join(self.env.path, logs_dir)
        os.makedirs(logs_dir)

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        connector, _ = DatabaseManager(self.env)._get_connector()
        for table in TargetPlatform._schema:
            for stmt in connector.to_sql(table):
                cursor.execute(stmt)
        db.commit()

    def test_new(self):
        platform = TargetPlatform(self.env)
        self.assertEqual(False, platform.exists)
        self.assertEqual([], platform.rules)

    def test_insert(self):
        platform = TargetPlatform(self.env, config='test', name='Windows XP')
        platform.rules += [(Build.OS_NAME, 'Windows'), (Build.OS_VERSION, 'XP')]
        platform.insert()

        assert platform.exists
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT config,name FROM bitten_platform "
                       "WHERE id=%s", (platform.id,))
        self.assertEqual(('test', 'Windows XP'), cursor.fetchone())

        cursor.execute("SELECT propname,pattern,orderno FROM bitten_rule "
                       "WHERE id=%s", (platform.id,))
        self.assertEqual((Build.OS_NAME, 'Windows', 0), cursor.fetchone())
        self.assertEqual((Build.OS_VERSION, 'XP', 1), cursor.fetchone())

    def test_fetch(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_platform (config,name) "
                       "VALUES (%s,%s)", ('test', 'Windows'))
        id = db.get_last_id(cursor, 'bitten_platform')
        platform = TargetPlatform.fetch(self.env, id)
        assert platform.exists
        self.assertEqual('test', platform.config)
        self.assertEqual('Windows', platform.name)

    def test_select(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.executemany("INSERT INTO bitten_platform (config,name) "
                           "VALUES (%s,%s)", [('test', 'Windows'),
                           ('test', 'Mac OS X')])
        platforms = list(TargetPlatform.select(self.env, config='test'))
        self.assertEqual(2, len(platforms))


class BuildTestCase(unittest.TestCase):

    def setUp(self):
        self.env = EnvironmentStub()
        self.env.path = tempfile.mkdtemp()
        logs_dir = self.env.config.get("bitten", "logs_dir")
        if os.path.isabs(logs_dir):
            raise ValueError("Should not have absolute logs directory for temporary test")
        logs_dir = os.path.join(self.env.path, logs_dir)
        os.makedirs(logs_dir)

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        connector, _ = DatabaseManager(self.env)._get_connector()
        for table in Build._schema:
            for stmt in connector.to_sql(table):
                cursor.execute(stmt)
        db.commit()

    def test_new(self):
        build = Build(self.env)
        self.assertEqual(None, build.id)
        self.assertEqual(Build.PENDING, build.status)
        self.assertEqual(0, build.stopped)
        self.assertEqual(0, build.started)

    def test_insert(self):
        build = Build(self.env, config='test', rev='42', rev_time=12039,
                      platform=1)
        build.slave_info.update({Build.IP_ADDRESS: '127.0.0.1',
                                 Build.MAINTAINER: 'joe@example.org'})
        build.insert()

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT config,rev,platform,slave,started,stopped,status"
                       " FROM bitten_build WHERE id=%s" % build.id)
        self.assertEqual(('test', '42', 1, '', 0, 0, 'P'), cursor.fetchone())

        cursor.execute("SELECT propname,propvalue FROM bitten_slave")
        expected = {Build.IP_ADDRESS: '127.0.0.1',
                    Build.MAINTAINER: 'joe@example.org'}
        for propname, propvalue in cursor:
            self.assertEqual(expected[propname], propvalue)

    def test_insert_no_config_or_rev_or_rev_time_or_platform(self):
        build = Build(self.env)
        self.assertRaises(AssertionError, build.insert)

        build = Build(self.env, rev='42', rev_time=12039, platform=1)
        self.assertRaises(AssertionError, build.insert) # No config

        build = Build(self.env, config='test', rev_time=12039, platform=1)
        self.assertRaises(AssertionError, build.insert) # No rev

        build = Build(self.env, config='test', rev='42', platform=1)
        self.assertRaises(AssertionError, build.insert) # No rev time

        build = Build(self.env, config='test', rev='42', rev_time=12039)
        self.assertRaises(AssertionError, build.insert) # No platform

    def test_insert_no_slave(self):
        build = Build(self.env, config='test', rev='42', rev_time=12039,
                      platform=1)
        build.status = Build.SUCCESS
        self.assertRaises(AssertionError, build.insert)
        build.status = Build.FAILURE
        self.assertRaises(AssertionError, build.insert)
        build.status = Build.IN_PROGRESS
        self.assertRaises(AssertionError, build.insert)
        build.status = Build.PENDING
        build.insert()

    def test_insert_invalid_status(self):
        build = Build(self.env, config='test', rev='42', rev_time=12039,
                      status='DUNNO')
        self.assertRaises(AssertionError, build.insert)

    def test_fetch(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_build (config,rev,rev_time,platform,"
                       "slave,started,stopped,status) "
                       "VALUES (%s,%s,%s,%s,%s,%s,%s,%s)",
                       ('test', '42', 12039, 1, 'tehbox', 15006, 16007,
                        Build.SUCCESS))
        build_id = db.get_last_id(cursor, 'bitten_build')
        cursor.executemany("INSERT INTO bitten_slave VALUES (%s,%s,%s)",
                           [(build_id, Build.IP_ADDRESS, '127.0.0.1'),
                            (build_id, Build.MAINTAINER, 'joe@example.org')])

        build = Build.fetch(self.env, build_id)
        self.assertEquals(build_id, build.id)
        self.assertEquals('127.0.0.1', build.slave_info[Build.IP_ADDRESS])
        self.assertEquals('joe@example.org', build.slave_info[Build.MAINTAINER])

    def test_update(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_build (config,rev,rev_time,platform,"
                       "slave,started,stopped,status) "
                       "VALUES (%s,%s,%s,%s,%s,%s,%s,%s)",
                       ('test', '42', 12039, 1, 'tehbox', 15006, 16007,
                        Build.SUCCESS))
        build_id = db.get_last_id(cursor, 'bitten_build')
        cursor.executemany("INSERT INTO bitten_slave VALUES (%s,%s,%s)",
                           [(build_id, Build.IP_ADDRESS, '127.0.0.1'),
                            (build_id, Build.MAINTAINER, 'joe@example.org')])

        build = Build.fetch(self.env, build_id)
        build.status = Build.FAILURE
        build.update()


class BuildStepTestCase(unittest.TestCase):

    def setUp(self):
        self.env = EnvironmentStub()
        self.env.path = tempfile.mkdtemp()
        logs_dir = self.env.config.get("bitten", "logs_dir")
        if os.path.isabs(logs_dir):
            raise ValueError("Should not have absolute logs directory for temporary test")
        logs_dir = os.path.join(self.env.path, logs_dir)
        os.makedirs(logs_dir)

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        connector, _ = DatabaseManager(self.env)._get_connector()
        for table in BuildStep._schema:
            for stmt in connector.to_sql(table):
                cursor.execute(stmt)
        db.commit()

    def test_new(self):
        step = BuildStep(self.env)
        self.assertEqual(False, step.exists)
        self.assertEqual(None, step.build)
        self.assertEqual(None, step.name)

    def test_insert(self):
        step = BuildStep(self.env, build=1, name='test', description='Foo bar',
                         status=BuildStep.SUCCESS)
        step.insert()
        self.assertEqual(True, step.exists)

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT build,name,description,status,started,stopped "
                       "FROM bitten_step")
        self.assertEqual((1, 'test', 'Foo bar', BuildStep.SUCCESS, 0, 0),
                         cursor.fetchone())

    def test_insert_with_errors(self):
        step = BuildStep(self.env, build=1, name='test', description='Foo bar',
                         status=BuildStep.SUCCESS)
        step.errors += ['Foo', 'Bar']
        step.insert()
        self.assertEqual(True, step.exists)

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT build,name,description,status,started,stopped "
                       "FROM bitten_step")
        self.assertEqual((1, 'test', 'Foo bar', BuildStep.SUCCESS, 0, 0),
                         cursor.fetchone())
        cursor.execute("SELECT message FROM bitten_error ORDER BY orderno")
        self.assertEqual(('Foo',), cursor.fetchone())
        self.assertEqual(('Bar',), cursor.fetchone())

    def test_insert_no_build_or_name(self):
        step = BuildStep(self.env, name='test')
        self.assertRaises(AssertionError, step.insert) # No build

        step = BuildStep(self.env, build=1)
        self.assertRaises(AssertionError, step.insert) # No name

    def test_fetch(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_step VALUES (%s,%s,%s,%s,%s,%s)",
                       (1, 'test', 'Foo bar', BuildStep.SUCCESS, 0, 0))

        step = BuildStep.fetch(self.env, build=1, name='test')
        self.assertEqual(1, step.build)
        self.assertEqual('test', step.name)
        self.assertEqual('Foo bar', step.description)
        self.assertEqual(BuildStep.SUCCESS, step.status)

    def test_fetch_with_errors(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_step VALUES (%s,%s,%s,%s,%s,%s)",
                       (1, 'test', 'Foo bar', BuildStep.SUCCESS, 0, 0))
        cursor.executemany("INSERT INTO bitten_error VALUES (%s,%s,%s,%s)",
                           [(1, 'test', 'Foo', 0), (1, 'test', 'Bar', 1)])

        step = BuildStep.fetch(self.env, build=1, name='test')
        self.assertEqual(1, step.build)
        self.assertEqual('test', step.name)
        self.assertEqual('Foo bar', step.description)
        self.assertEqual(BuildStep.SUCCESS, step.status)
        self.assertEqual(['Foo', 'Bar'], step.errors)

    def test_select(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.executemany("INSERT INTO bitten_step VALUES (%s,%s,%s,%s,%s,%s)",
                           [(1, 'test', 'Foo bar', BuildStep.SUCCESS, 1, 2),
                            (1, 'dist', 'Foo baz', BuildStep.FAILURE, 2, 3)])

        steps = list(BuildStep.select(self.env, build=1))
        self.assertEqual(1, steps[0].build)
        self.assertEqual('test', steps[0].name)
        self.assertEqual('Foo bar', steps[0].description)
        self.assertEqual(BuildStep.SUCCESS, steps[0].status)
        self.assertEqual(1, steps[1].build)
        self.assertEqual('dist', steps[1].name)
        self.assertEqual('Foo baz', steps[1].description)
        self.assertEqual(BuildStep.FAILURE, steps[1].status)


class BuildLogTestCase(unittest.TestCase):

    def setUp(self):
        self.env = EnvironmentStub()
        self.env.path = tempfile.mkdtemp()
        logs_dir = self.env.config.get("bitten", "logs_dir")
        if os.path.isabs(logs_dir):
            raise ValueError("Should not have absolute logs directory for temporary test")
        logs_dir = os.path.join(self.env.path, logs_dir)
        os.makedirs(logs_dir)

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        connector, _ = DatabaseManager(self.env)._get_connector()
        for table in BuildLog._schema:
            for stmt in connector.to_sql(table):
                cursor.execute(stmt)
        db.commit()

    def test_new(self):
        log = BuildLog(self.env)
        self.assertEqual(False, log.exists)
        self.assertEqual(None, log.id)
        self.assertEqual(None, log.build)
        self.assertEqual(None, log.step)
        self.assertEqual('', log.generator)
        self.assertEqual([], log.messages)

    def test_insert(self):
        log = BuildLog(self.env, build=1, step='test', generator='distutils', filename='1.log')
        full_file = log.get_log_file('1.log')
        if os.path.exists(full_file):
            os.remove(full_file)
        log.messages = [
            (BuildLog.INFO, 'running tests'),
            (BuildLog.ERROR, 'tests failed')
        ]
        log.insert()
        self.assertNotEqual(None, log.id)

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT build,step,generator,filename FROM bitten_log "
                       "WHERE id=%s", (log.id,))
        self.assertEqual((1, 'test', 'distutils', '1.log'), cursor.fetchone())
        lines = open(full_file, "rb").readlines()
        self.assertEqual('running tests\n', lines[0])
        self.assertEqual('tests failed\n', lines[1])
        if os.path.exists(full_file):
            os.remove(full_file)

    def test_insert_empty(self):
        log = BuildLog(self.env, build=1, step='test', generator='distutils', filename="1.log")
        full_file = log.get_log_file('1.log')
        if os.path.exists(full_file):
            os.remove(full_file)
        log.messages = []
        log.insert()
        self.assertNotEqual(None, log.id)

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT build,step,generator,filename FROM bitten_log "
                       "WHERE id=%s", (log.id,))
        self.assertEqual((1, 'test', 'distutils', '1.log'), cursor.fetchone())
        file_exists = os.path.exists(full_file)
        if file_exists:
            os.remove(full_file)
        assert not file_exists

    def test_insert_no_build_or_step(self):
        log = BuildLog(self.env, step='test')
        self.assertRaises(AssertionError, log.insert) # No build

        step = BuildStep(self.env, build=1)
        self.assertRaises(AssertionError, log.insert) # No step

    def test_delete(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_log (build,step,generator,filename) "
                       "VALUES (%s,%s,%s,%s)", (1, 'test', 'distutils', '1.log'))
        id = db.get_last_id(cursor, 'bitten_log')
        logs_dir = self.env.config.get("bitten", "logsdir", "log/bitten")
        if os.path.isabs(logs_dir):
            raise ValueError("Should not have absolute logs directory for temporary test")
        logs_dir = os.path.join(self.env.path, logs_dir)
        full_file = os.path.join(logs_dir, "1.log")
        open(full_file, "wb").writelines(["running tests\n", "tests failed\n"])

        log = BuildLog.fetch(self.env, id=id, db=db)
        self.assertEqual(True, log.exists)
        log.delete()
        self.assertEqual(False, log.exists)

        cursor.execute("SELECT * FROM bitten_log WHERE id=%s", (id,))
        self.assertEqual(True, not cursor.fetchall())
        file_exists = os.path.exists(full_file)
        if os.path.exists(full_file):
            os.remove(full_file)
        assert not file_exists

    def test_delete_new(self):
        log = BuildLog(self.env, build=1, step='test', generator='foo')
        self.assertRaises(AssertionError, log.delete)

    def test_fetch(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_log (build,step,generator,filename) "
                       "VALUES (%s,%s,%s,%s)", (1, 'test', 'distutils', '1.log'))
        id = db.get_last_id(cursor, 'bitten_log')
        logs_dir = self.env.config.get("bitten", "logsdir", "log/bitten")
        if os.path.isabs(logs_dir):
            raise ValueError("Should not have absolute logs directory for temporary test")
        logs_dir = os.path.join(self.env.path, logs_dir)
        full_file = os.path.join(logs_dir, "1.log")
        open(full_file, "wb").writelines(["running tests\n", "tests failed\n", u"test unicode\xbb\n".encode("UTF-8")])

        log = BuildLog.fetch(self.env, id=id, db=db)
        self.assertEqual(True, log.exists)
        self.assertEqual(id, log.id)
        self.assertEqual(1, log.build)
        self.assertEqual('test', log.step)
        self.assertEqual('distutils', log.generator)
        self.assertEqual((BuildLog.UNKNOWN, 'running tests'), log.messages[0])
        self.assertEqual((BuildLog.UNKNOWN, 'tests failed'), log.messages[1])
        self.assertEqual((BuildLog.UNKNOWN, u'test unicode\xbb'), log.messages[2])
        if os.path.exists(full_file):
            os.remove(full_file)

    def test_select(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_log (build,step,generator,filename) "
                       "VALUES (%s,%s,%s,%s)", (1, 'test', 'distutils', '1.log'))
        id = db.get_last_id(cursor, 'bitten_log')
        logs_dir = self.env.config.get("bitten", "logsdir", "log/bitten")
        if os.path.isabs(logs_dir):
            raise ValueError("Should not have absolute logs directory for temporary test")
        logs_dir = os.path.join(self.env.path, logs_dir)
        full_file = os.path.join(logs_dir, "1.log")
        open(full_file, "wb").writelines(["running tests\n", "tests failed\n", u"test unicode\xbb\n".encode("UTF-8")])

        logs = BuildLog.select(self.env, build=1, step='test', db=db)
        log = logs.next()
        self.assertEqual(True, log.exists)
        self.assertEqual(id, log.id)
        self.assertEqual(1, log.build)
        self.assertEqual('test', log.step)
        self.assertEqual('distutils', log.generator)
        self.assertEqual((BuildLog.UNKNOWN, 'running tests'), log.messages[0])
        self.assertEqual((BuildLog.UNKNOWN, 'tests failed'), log.messages[1])
        self.assertEqual((BuildLog.UNKNOWN, u'test unicode\xbb'), log.messages[2])
        self.assertRaises(StopIteration, logs.next)
        if os.path.exists(full_file):
            os.remove(full_file)


class ReportTestCase(unittest.TestCase):

    def setUp(self):
        self.env = EnvironmentStub()
        self.env.path = tempfile.mkdtemp()
        logs_dir = self.env.config.get("bitten", "logs_dir")
        if os.path.isabs(logs_dir):
            raise ValueError("Should not have absolute logs directory for temporary test")
        logs_dir = os.path.join(self.env.path, logs_dir)
        os.makedirs(logs_dir)

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        connector, _ = DatabaseManager(self.env)._get_connector()
        for table in Report._schema:
            for stmt in connector.to_sql(table):
                cursor.execute(stmt)
        db.commit()

    def test_delete(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_report "
                       "(build,step,category,generator) VALUES (%s,%s,%s,%s)",
                       (1, 'test', 'test', 'unittest'))
        report_id = db.get_last_id(cursor, 'bitten_report')
        cursor.executemany("INSERT INTO bitten_report_item "
                           "(report,item,name,value) VALUES (%s,%s,%s,%s)",
                           [(report_id, 0, 'file', 'tests/foo.c'),
                            (report_id, 0, 'result', 'failure'),
                            (report_id, 1, 'file', 'tests/bar.c'),
                            (report_id, 1, 'result', 'success')])

        report = Report.fetch(self.env, report_id, db=db)
        report.delete(db=db)
        self.assertEqual(False, report.exists)
        report = Report.fetch(self.env, report_id, db=db)
        self.assertEqual(None, report)

    def test_insert(self):
        report = Report(self.env, build=1, step='test', category='test',
                        generator='unittest')
        report.items = [
            {'file': 'tests/foo.c', 'status': 'failure'},
            {'file': 'tests/bar.c', 'status': 'success'}
        ]
        report.insert()

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT build,step,category,generator "
                       "FROM bitten_report WHERE id=%s", (report.id,))
        self.assertEqual((1, 'test', 'test', 'unittest'),
                         cursor.fetchone())
        cursor.execute("SELECT item,name,value FROM bitten_report_item "
                       "WHERE report=%s ORDER BY item", (report.id,))
        items = []
        prev_item = None
        for item, name, value in cursor:
            if item != prev_item:
                items.append({name: value})
                prev_item = item
            else:
                items[-1][name] = value
        self.assertEquals(2, len(items))
        seen_foo, seen_bar = False, False
        for item in items:
            if item['file'] == 'tests/foo.c':
                self.assertEqual('failure', item['status'])
                seen_foo = True
            if item['file'] == 'tests/bar.c':
                self.assertEqual('success', item['status'])
                seen_bar = True
        self.assertEquals((True, True), (seen_foo, seen_bar))

    def test_insert_dupe(self):
        report = Report(self.env, build=1, step='test', category='test',
                        generator='unittest')
        report.insert()

        report = Report(self.env, build=1, step='test', category='test',
                        generator='unittest')
        self.assertRaises(AssertionError, report.insert)

    def test_insert_empty_items(self):
        report = Report(self.env, build=1, step='test', category='test',
                        generator='unittest')
        report.items = [{}, {}]
        report.insert()

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT build,step,category,generator "
                       "FROM bitten_report WHERE id=%s", (report.id,))
        self.assertEqual((1, 'test', 'test', 'unittest'),
                         cursor.fetchone())
        cursor.execute("SELECT COUNT(*) FROM bitten_report_item "
                       "WHERE report=%s", (report.id,))
        self.assertEqual(0, cursor.fetchone()[0])

    def test_fetch(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_report "
                       "(build,step,category,generator) VALUES (%s,%s,%s,%s)",
                       (1, 'test', 'test', 'unittest'))
        report_id = db.get_last_id(cursor, 'bitten_report')
        cursor.executemany("INSERT INTO bitten_report_item "
                           "(report,item,name,value) VALUES (%s,%s,%s,%s)",
                           [(report_id, 0, 'file', 'tests/foo.c'),
                            (report_id, 0, 'result', 'failure'),
                            (report_id, 1, 'file', 'tests/bar.c'),
                            (report_id, 1, 'result', 'success')])

        report = Report.fetch(self.env, report_id)
        self.assertEquals(report_id, report.id)
        self.assertEquals('test', report.step)
        self.assertEquals('test', report.category)
        self.assertEquals('unittest', report.generator)
        self.assertEquals(2, len(report.items))
        assert {'file': 'tests/foo.c', 'result': 'failure'} in report.items
        assert {'file': 'tests/bar.c', 'result': 'success'} in report.items

    def test_select(self):
        db = self.env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_report "
                       "(build,step,category,generator) VALUES (%s,%s,%s,%s)",
                       (1, 'test', 'test', 'unittest'))
        report1_id = db.get_last_id(cursor, 'bitten_report')
        cursor.execute("INSERT INTO bitten_report "
                       "(build,step,category,generator) VALUES (%s,%s,%s,%s)",
                       (1, 'test', 'coverage', 'trace'))
        report2_id = db.get_last_id(cursor, 'bitten_report')
        cursor.executemany("INSERT INTO bitten_report_item "
                           "(report,item,name,value) VALUES (%s,%s,%s,%s)",
                           [(report1_id, 0, 'file', 'tests/foo.c'),
                            (report1_id, 0, 'result', 'failure'),
                            (report1_id, 1, 'file', 'tests/bar.c'),
                            (report1_id, 1, 'result', 'success'),
                            (report2_id, 0, 'file', 'tests/foo.c'),
                            (report2_id, 0, 'loc', '12'),
                            (report2_id, 0, 'cov', '50'),
                            (report2_id, 1, 'file', 'tests/bar.c'),
                            (report2_id, 1, 'loc', '20'),
                            (report2_id, 1, 'cov', '25')])

        reports = Report.select(self.env, build=1, step='test')
        for idx, report in enumerate(reports):
            if report.id == report1_id:
                self.assertEquals('test', report.step)
                self.assertEquals('test', report.category)
                self.assertEquals('unittest', report.generator)
                self.assertEquals(2, len(report.items))
                assert {'file': 'tests/foo.c', 'result': 'failure'} \
                       in report.items
                assert {'file': 'tests/bar.c', 'result': 'success'} \
                       in report.items
            elif report.id == report1_id:
                self.assertEquals('test', report.step)
                self.assertEquals('coverage', report.category)
                self.assertEquals('trace', report.generator)
                self.assertEquals(2, len(report.items))
                assert {'file': 'tests/foo.c', 'loc': '12', 'cov': '50'} \
                       in report.items
                assert {'file': 'tests/bar.c', 'loc': '20', 'cov': '25'} \
                       in report.items
        self.assertEqual(1, idx)


class PlatformBuildTestCase(unittest.TestCase):
    """Tests that involve Builds, TargetPlatforms and BuildSteps"""

    def setUp(self):
        self.env = EnvironmentStub()
        self.env.path = tempfile.mkdtemp()
        logs_dir = self.env.config.get("bitten", "logs_dir")
        if os.path.isabs(logs_dir):
            raise ValueError("Should not have absolute logs directory for temporary test")
        logs_dir = os.path.join(self.env.path, logs_dir)
        os.makedirs(logs_dir)

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        connector, _ = DatabaseManager(self.env)._get_connector()
        for schema in [Build._schema, TargetPlatform._schema, BuildStep._schema]: 
            for table in schema:
                for stmt in connector.to_sql(table):
                    cursor.execute(stmt)
        db.commit()

    def test_delete_platform_with_pending_builds(self):
        """Check that deleting a platform with pending builds removes those pending builds"""
        db = self.env.get_db_cnx()
        platform = TargetPlatform(self.env, config='test', name='Linux')
        platform.insert()
        build = Build(self.env, config='test', platform=platform.id, rev='42', rev_time=12039)
        build.insert()

        platform.delete()
        pending = list(build.select(self.env, config='test', status=Build.PENDING))
        self.assertEqual(0, len(pending))


def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(BuildConfigTestCase, 'test'))
    suite.addTest(unittest.makeSuite(TargetPlatformTestCase, 'test'))
    suite.addTest(unittest.makeSuite(BuildTestCase, 'test'))
    suite.addTest(unittest.makeSuite(BuildStepTestCase, 'test'))
    suite.addTest(unittest.makeSuite(BuildLogTestCase, 'test'))
    suite.addTest(unittest.makeSuite(ReportTestCase, 'test'))
    suite.addTest(unittest.makeSuite(PlatformBuildTestCase, 'test'))
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
