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
from trac.test import EnvironmentStub, Mock
from bitten.model import *
from bitten.report.testing import TestResultsChartGenerator


class TestResultsChartGeneratorTestCase(unittest.TestCase):

    def setUp(self):
        self.env = EnvironmentStub()
        self.env.path = ''

        db = self.env.get_db_cnx()
        cursor = db.cursor()
        connector, _ = DatabaseManager(self.env)._get_connector()
        for table in schema:
            for stmt in connector.to_sql(table):
                cursor.execute(stmt)

    def test_supported_categories(self):
        generator = TestResultsChartGenerator(self.env)
        self.assertEqual(['test'], generator.get_supported_categories())

    def test_no_reports(self):
        req = Mock()
        config = Mock(name='trunk')
        generator = TestResultsChartGenerator(self.env)
        template, data = generator.generate_chart_data(req, config, 'test')
        self.assertEqual('bitten_chart_tests.html', template)
        self.assertEqual('Unit Tests', data['title'])
        self.assertEqual('', data['data'][0][0])
        self.assertEqual('Total', data['data'][1][0])
        self.assertEqual('Failures', data['data'][2][0])

    def test_single_platform(self):
        config = Mock(name='trunk')
        build = Build(self.env, config='trunk', platform=1, rev=123,
                      rev_time=42)
        build.insert()
        report = Report(self.env, build=build.id, step='foo', category='test')
        report.items += [{'status': 'success'}, {'status': 'failure'},
                         {'status': 'success'}]
        report.insert()

        req = Mock()
        generator = TestResultsChartGenerator(self.env)
        template, data = generator.generate_chart_data(req, config, 'test')
        self.assertEqual('bitten_chart_tests.html', template)
        self.assertEqual('Unit Tests', data['title'])
        self.assertEqual('', data['data'][0][0])
        self.assertEqual('[123]', data['data'][0][1])
        self.assertEqual('Total', data['data'][1][0])
        self.assertEqual(3, data['data'][1][1])
        self.assertEqual('Failures', data['data'][2][0])
        self.assertEqual(1, data['data'][2][1])

    def test_multi_platform(self):
        config = Mock(name='trunk')

        build = Build(self.env, config='trunk', platform=1, rev=123,
                      rev_time=42)
        build.insert()
        report = Report(self.env, build=build.id, step='foo', category='test')
        report.items += [{'status': 'success'}, {'status': 'failure'},
                         {'status': 'success'}]
        report.insert()

        build = Build(self.env, config='trunk', platform=2, rev=123,
                      rev_time=42)
        build.insert()
        report = Report(self.env, build=build.id, step='foo', category='test')
        report.items += [{'status': 'success'}, {'status': 'failure'},
                         {'status': 'failure'}]
        report.insert()

        req = Mock()
        generator = TestResultsChartGenerator(self.env)
        template, data = generator.generate_chart_data(req, config, 'test')
        self.assertEqual('bitten_chart_tests.html', template)
        self.assertEqual('Unit Tests', data['title'])
        self.assertEqual('', data['data'][0][0])
        self.assertEqual('[123]', data['data'][0][1])
        self.assertEqual('Total', data['data'][1][0])
        self.assertEqual(3, data['data'][1][1])
        self.assertEqual('Failures', data['data'][2][0])
        self.assertEqual(2, data['data'][2][1])


def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(TestResultsChartGeneratorTestCase))
    return suite

if __name__ == '__main__':
    unittest.main(defaultTest='suite')
