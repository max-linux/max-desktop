#-*- coding: utf-8 -*-
#
# Copyright (C) 2007 Ole Trenner, <ole@jayotee.de>
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution.

import logging
import os
import sys
import unittest

from trac.db import DatabaseManager
from trac.test import EnvironmentStub, Mock
from trac.web.session import DetachedSession
from bitten.model import *
from bitten.notify import *


class BittenNotifyBaseTest(unittest.TestCase):
    def setUp(self):
        self.set_up_env()

    def set_up_env(self):
        self.env = EnvironmentStub(enable=['trac.*', 'bitten.notify.*'])
        self.env.path = ''
        self.repos = Mock(get_changeset=lambda rev: Mock(author = 'author'))
        self.env.get_repository = lambda authname = None: self.repos

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        connector, _ = DatabaseManager(self.env)._get_connector()
        for table in schema:
            for stmt in connector.to_sql(table):
                cursor.execute(stmt)
        db.commit()


class BittenNotifyTest(BittenNotifyBaseTest):
    """unit tests for BittenNotify dispatcher class"""
    def setUp(self):
        BittenNotifyBaseTest.setUp(self)
        self.dispatcher = BittenNotify(self.env)
        self.failed_build = Build(self.env, status=Build.FAILURE)
        self.successful_build = Build(self.env, status=Build.SUCCESS)

    def test_do_notify_on_failed_build(self):
        self.env.config.set(CONFIG_SECTION, NOTIFY_ON_FAILURE, 'true')
        self.assertTrue(self.dispatcher._should_notify(self.failed_build),
                'notifier should be called for failed builds.')

    def test_do_not_notify_on_failed_build(self):
        self.env.config.set(CONFIG_SECTION, NOTIFY_ON_FAILURE, 'false')
        self.assertFalse(self.dispatcher._should_notify(self.failed_build),
                'notifier should not be called for failed build.')

    def test_do_notify_on_successful_build(self):
        self.env.config.set(CONFIG_SECTION, NOTIFY_ON_SUCCESS, 'true')
        self.dispatcher.notify(self.successful_build)
        self.assertTrue(self.dispatcher._should_notify(self.successful_build),
                'notifier should be called for successful builds when configured.')

    def test_do_not_notify_on_successful_build(self):
        self.env.config.set(CONFIG_SECTION, NOTIFY_ON_SUCCESS, 'false')
        self.dispatcher.notify(self.successful_build)
        self.assertFalse(self.dispatcher._should_notify(self.successful_build),
                'notifier shouldn\'t be called for successful build.')


class BuildInfoTest(BittenNotifyBaseTest):
    """unit tests for BuildInfo class"""

    def setUp(self):
        BittenNotifyBaseTest.setUp(self)
        #fixture
        self.failed_build = Build(self.env,
                config = 'config',
                slave = 'slave',
                rev = 10,
                status = Build.FAILURE)
        self.failed_build.id = 1
        self.successful_build = Build(self.env, status = Build.SUCCESS)
        self.successful_build.id = 2
        step = BuildStep(self.env,
                build = 1,
                name = 'test',
                status = BuildStep.FAILURE)
        step.errors = ['msg']
        step.insert()
        log = BuildLog(self.env, build = 1, step = 'test')
        log.messages = [('info','msg')]
        log.insert()

    def test_exposed_properties(self):
        build_info = BuildInfo(self.env, self.failed_build)
        self.assertEquals(self.failed_build.id, build_info.id)
        self.assertEquals('Failed', build_info.status)
        self.assertEquals('http://example.org/trac.cgi/build/config/1',
                build_info.link)
        self.assertEquals('config', build_info.config)
        self.assertEquals('slave', build_info.slave)
        self.assertEquals('10', build_info.changeset)
        self.assertEquals('http://example.org/trac.cgi/changeset/10',
                build_info.changesetlink)
        self.assertEquals('author', build_info.author)
        self.assertEquals('test: msg', build_info.errors)
        self.assertEquals(' info: msg', build_info.faillog)

    def test_exposed_properties_on_successful_build(self):
        build_info = BuildInfo(self.env, self.successful_build)
        self.assertEquals(self.successful_build.id, build_info.id)
        self.assertEquals('Successful', build_info.status)


class BittenNotifyEmailTest(BittenNotifyBaseTest):
    """unit tests for BittenNotifyEmail class"""
    def setUp(self):
        BittenNotifyBaseTest.setUp(self)
        self.env.config.set('notification','smtp_enabled','true')
        self.notifications_sent_to = []
        def send(to, cc, hdrs={}):
            self.notifications_sent_to = to
        def noop(*args, **kw):
            pass
        self.email = Mock(BittenNotifyEmail, self.env,
                          begin_send=noop,
                          finish_send=noop,
                          send=send)
        self.build_info = BuildInfo(self.env, Build(self.env,
                status = Build.SUCCESS))
        self.build_info['author'] = 'author'

    def test_notification_uses_default_address(self):
        self.email.notify(self.build_info)
        self.assertTrue('author' in self.notifications_sent_to,
                'recipient list should contain plain author')

    def test_notification_uses_custom_address(self):
        self.add_known_user('author', "Author's Name", 'author@email.com')
        self.email.notify(self.build_info)
        self.assertTrue('author@email.com' in self.notifications_sent_to,
                "recipient list should contain custom author's email")

    def test_notification_discards_invalid_address(self):
        self.add_known_user('author', "Author's Name", email=None)
        self.email.notify(self.build_info)
        self.assertTrue('author' in self.notifications_sent_to,
                'recipient list should only use valid custom address')

    def add_known_user(self, username, name, email):
        session = DetachedSession(self.env, username)
        if name is not None:
            session['name'] = name
        if email is not None:
            session['email'] = email
        session.save()


def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(BittenNotifyTest,'test'))
    suite.addTest(unittest.makeSuite(BuildInfoTest,'test'))
    suite.addTest(unittest.makeSuite(BittenNotifyEmailTest,'test'))
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
