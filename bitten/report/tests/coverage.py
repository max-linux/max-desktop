# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

import doctest
import unittest

from trac.db import DatabaseManager
from trac.test import EnvironmentStub, Mock
from bitten.model import *
from bitten.report import coverage
from bitten.report.coverage import TestCoverageChartGenerator

def env_stub_with_tables():
    env = EnvironmentStub()
    db = env.get_db_cnx()
    cursor = db.cursor()
    connector, _ = DatabaseManager(env)._get_connector()
    for table in schema:
        for stmt in connector.to_sql(table):
            cursor.execute(stmt)
    return env

class TestCoverageChartGeneratorTestCase(unittest.TestCase):

    def setUp(self):
        self.env = env_stub_with_tables()
        self.env.path = ''

    def test_supported_categories(self):
        generator = TestCoverageChartGenerator(self.env)
        self.assertEqual(['coverage'], generator.get_supported_categories())

    def test_no_reports(self):
        req = Mock()
        config = Mock(name='trunk')
        generator = TestCoverageChartGenerator(self.env)
        template, data = generator.generate_chart_data(req, config, 'coverage')
        self.assertEqual('bitten_chart_coverage.html', template)
        self.assertEqual('Test Coverage', data['title'])
        self.assertEqual('', data['data'][0][0])
        self.assertEqual('Lines of code', data['data'][1][0])
        self.assertEqual('Coverage', data['data'][2][0])

    def test_single_platform(self):
        config = Mock(name='trunk')
        build = Build(self.env, config='trunk', platform=1, rev=123,
                      rev_time=42)
        build.insert()
        report = Report(self.env, build=build.id, step='foo',
                        category='coverage')
        report.items += [{'lines': '12', 'percentage': '25'}]
        report.insert()

        req = Mock()
        generator = TestCoverageChartGenerator(self.env)
        template, data = generator.generate_chart_data(req, config, 'coverage')
        self.assertEqual('bitten_chart_coverage.html', template)
        self.assertEqual('Test Coverage', data['title'])
        self.assertEqual('', data['data'][0][0])
        self.assertEqual('[123]', data['data'][0][1])
        self.assertEqual('Lines of code', data['data'][1][0])
        self.assertEqual(12, data['data'][1][1])
        self.assertEqual('Coverage', data['data'][2][0])
        self.assertEqual(3, data['data'][2][1])

    def test_multi_platform(self):
        config = Mock(name='trunk')
        build = Build(self.env, config='trunk', platform=1, rev=123,
                      rev_time=42)
        build.insert()
        report = Report(self.env, build=build.id, step='foo',
                        category='coverage')
        report.items += [{'lines': '12', 'percentage': '25'}]
        report.insert()
        build = Build(self.env, config='trunk', platform=2, rev=123,
                      rev_time=42)
        build.insert()
        report = Report(self.env, build=build.id, step='foo',
                        category='coverage')
        report.items += [{'lines': '12', 'percentage': '50'}]
        report.insert()

        req = Mock()
        generator = TestCoverageChartGenerator(self.env)
        template, data = generator.generate_chart_data(req, config, 'coverage')
        self.assertEqual('bitten_chart_coverage.html', template)
        self.assertEqual('Test Coverage', data['title'])
        self.assertEqual('', data['data'][0][0])
        self.assertEqual('[123]', data['data'][0][1])
        self.assertEqual('Lines of code', data['data'][1][0])
        self.assertEqual(12, data['data'][1][1])
        self.assertEqual('Coverage', data['data'][2][0])
        self.assertEqual(6, data['data'][2][1])


def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(TestCoverageChartGeneratorTestCase))
    suite.addTest(doctest.DocTestSuite(coverage))
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
