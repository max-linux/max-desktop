# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Interfaces of extension points provided by the Bitten Trac plugin."""

from trac.core import *

__all__ = ['IBuildListener', 'ILogFormatter', 'IReportChartGenerator',
           'IReportSummarizer']
__docformat__ = 'restructuredtext en'


class IBuildListener(Interface):
    """Extension point interface for components that need to be notified of
    build events.
    
    Note that these will be notified in the process running the build master,
    not the web interface.
    """

    def build_started(build):
        """Called when a build slave has accepted a build initiation.
        
        :param build: the build that was started
        :type build: `Build`
        """

    def build_aborted(build):
        """Called when a build slave cancels a build or disconnects.
        
        :param build: the build that was aborted
        :type build: `Build`
        """

    def build_completed(build):
        """Called when a build slave has completed a build, regardless of the
        outcome.
        
        :param build: the build that was aborted
        :type build: `Build`
        """


class ILogFormatter(Interface):
    """Extension point interface for components that format build log
    messages."""

    def get_formatter(req, build):
        """Return a function that gets called for every log message.
        
        The function must take four positional arguments, ``step``,
        ``generator``, ``level`` and ``message``, and return the formatted
        message as a string.

        :param req: the request object
        :param build: the build to which the logs belong that should be
                      formatted
        :type build: `Build`
        :return: the formatted log message
        :rtype: `basestring`
        """


class IReportSummarizer(Interface):
    """Extension point interface for components that render a summary of reports
    of some kind."""

    def get_supported_categories():
        """Return a list of strings identifying the types of reports this 
        component supports.
        """

    def render_summary(req, config, build, step, category):
        """Render a summary for the given report.
        
        This function should return a tuple of the form `(template, data)`,
        where `template` is the name of the template to use and `data` is the
        data to be passed to the template.
        
        :param req: the request object
        :param config: the build configuration
        :type config: `BuildConfig`
        :param build: the build
        :type build: `Build`
        :param step: the build step
        :type step: `BuildStep`
        :param category: the category of the report that should be summarized
        :type category: `basestring`
        """


class IReportChartGenerator(Interface):
    """Extension point interface for components that generate a chart for a
    set of reports."""

    def get_supported_categories():
        """Return a list of strings identifying the types of reports this 
        component supports.
        """

    def generate_chart_data(req, config, category):
        """Generate the data for a report chart.
        
        This function should return a tuple of the form `(template, data)`,
        where `template` is the name of the template to use and `data` is the
        data to be passed to the template.
        
        :param req: the request object
        :param config: the build configuration
        :type config: `BuildConfig`
        :param category: the category of reports to include in the chart
        :type category: `basestring`
        """
