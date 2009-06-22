# -*- coding: utf-8 -*-
#
# Copyright (C) 2007 Edgewall Software
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Execution of build recipes.

This module provides various classes that can be used to process build recipes,
most importantly the `Recipe` class.
"""

import keyword
import logging
import os
try:
    set
except NameError:
    from sets import Set as set

from pkg_resources import WorkingSet
from bitten.build import BuildError
from bitten.build.config import Configuration
from bitten.util import xmlio

__all__ = ['Context', 'Recipe', 'Step', 'InvalidRecipeError']
__docformat__ = 'restructuredtext en'

log = logging.getLogger('bitten.recipe')


class InvalidRecipeError(Exception):
    """Exception raised when a recipe is not valid."""


class Context(object):
    """The context in which a build is executed."""

    step = None # The current step
    generator = None # The current generator (namespace#name)

    def __init__(self, basedir, config=None, vars=None):
        """Initialize the context.
        
        :param basedir: a string containing the working directory for the build.
                        (may be a pattern for replacement ex: 'build_${build}'
        :param config: the build slave configuration
        :type config: `Configuration`
        """        
        self.config = config or Configuration()
        self.vars = vars or {}
        self.output = []
        self.basedir = os.path.realpath(self.config.interpolate(basedir,
                                                                **self.vars))

    def run(self, step, namespace, name, attr):
        """Run the specified recipe command.
        
        :param step: the build step that the command belongs to
        :param namespace: the namespace URI of the command
        :param name: the local tag name of the command
        :param attr: a dictionary containing the attributes defined on the
                     command element
        """
        self.step = step

        try:
            function = None
            qname = '#'.join(filter(None, [namespace, name]))
            if namespace:
                group = 'bitten.recipe_commands'
                for entry_point in WorkingSet().iter_entry_points(group, qname):
                    function = entry_point.load()
                    break
            elif name == 'report':
                function = Context.report_file
            if not function:
                raise InvalidRecipeError('Unknown recipe command %s' % qname)

            def escape(name):
                name = name.replace('-', '_')
                if keyword.iskeyword(name) or name in __builtins__:
                    name = name + '_'
                return name
            args = dict([(escape(name),
                          self.config.interpolate(attr[name], **self.vars))
                         for name in attr])

            self.generator = qname
            log.debug('Executing %s with arguments: %s', function, args)
            function(self, **args)

        finally:
            self.generator = None
            self.step = None

    def error(self, message):
        """Record an error message.
        
        :param message: a string containing the error message.
        """
        self.output.append((Recipe.ERROR, None, self.generator, message))

    def log(self, xml):
        """Record log output.
        
        :param xml: an XML fragment containing the log messages
        """
        self.output.append((Recipe.LOG, None, self.generator, xml))

    def report(self, category, xml):
        """Record report data.
        
        :param category: the name of category of the report
        :param xml: an XML fragment containing the report data
        """
        self.output.append((Recipe.REPORT, category, self.generator, xml))

    def report_file(self, category=None, file_=None):
        """Read report data from a file and record it.
        
        :param category: the name of the category of the report
        :param file\_: the path to the file containing the report data, relative
                       to the base directory
        """
        filename = self.resolve(file_)
        try:
            fileobj = file(filename, 'r')
            try:
                xml_elem = xmlio.Fragment()
                for child in xmlio.parse(fileobj).children():
                    child_elem = xmlio.Element(child.name, **dict([
                        (name, value) for name, value in child.attr.items()
                        if value is not None
                    ]))
                    xml_elem.append(child_elem[
                        [xmlio.Element(grandchild.name)[grandchild.gettext()]
                        for grandchild in child.children()]
                    ])
                self.output.append((Recipe.REPORT, category, None, xml_elem))
            finally:
                fileobj.close()
        except xmlio.ParseError, e:
            self.error('Failed to parse %s report at %s: %s'
                       % (category, filename, e))
        except IOError, e:
            self.error('Failed to read %s report at %s: %s'
                       % (category, filename, e))

    def resolve(self, *path):
        """Return the path of a file relative to the base directory.
        
        Accepts any number of positional arguments, which are joined using the
        system path separator to form the path.
        """
        return os.path.normpath(os.path.join(self.basedir, *path))


class Step(object):
    """Represents a single step of a build recipe.

    Iterate over an object of this class to get the commands to execute, and
    their keyword arguments.
    """

    def __init__(self, elem):
        """Create the step.
        
        :param elem: the XML element representing the step
        :type elem: `ParsedElement`
        """
        self._elem = elem
        self.id = elem.attr['id']
        self.description = elem.attr.get('description')
        self.onerror = elem.attr.get('onerror', 'fail')

    def __repr__(self):
        return '<%s %r>' % (type(self).__name__, self.id)

    def execute(self, ctxt):
        """Execute this step in the given context.
        
        :param ctxt: the build context
        :type ctxt: `Context`
        """
        for child in self._elem:
            ctxt.run(self, child.namespace, child.name, child.attr)

        errors = []
        while ctxt.output:
            type, category, generator, output = ctxt.output.pop(0)
            yield type, category, generator, output
            if type == Recipe.ERROR:
                errors.append((generator, output))
        if errors:
            if self.onerror != 'ignore':
                raise BuildError('Build step %s failed' % self.id)
            log.warning('Continuing despite errors in step %s (%s)', self.id,
                        ', '.join([error[1] for error in errors]))


class Recipe(object):
    """A build recipe.
    
    Iterate over this object to get the individual build steps in the order
    they have been defined in the recipe file.
    """

    ERROR = 'error'
    LOG = 'log'
    REPORT = 'report'

    def __init__(self, xml, basedir=os.getcwd(), config=None):
        """Create the recipe.
        
        :param xml: the XML document representing the recipe
        :type xml: `ParsedElement`
        :param basedir: the base directory for the build
        :param config: the slave configuration (optional)
        :type config: `Configuration`
        """
        assert isinstance(xml, xmlio.ParsedElement)
        vars = dict([(name, value) for name, value in xml.attr.items()
                     if not name.startswith('xmlns')])
        self.ctxt = Context(basedir, config, vars)
        self._root = xml

    def __iter__(self):
        """Iterate over the individual steps of the recipe."""
        for child in self._root.children('step'):
            yield Step(child)

    def validate(self):
        """Validate the recipe.
        
        This method checks a number of constraints:
         - the name of the root element must be "build"
         - the only permitted child elements or the root element with the name
           "step"
         - the recipe must contain at least one step
         - step elements must have a unique "id" attribute
         - a step must contain at least one nested command
         - commands must not have nested content

        :raise InvalidRecipeError: in case any of the above contraints is
                                   violated
        """
        if self._root.name != 'build':
            raise InvalidRecipeError('Root element must be <build>')
        steps = list(self._root.children())
        if not steps:
            raise InvalidRecipeError('Recipe defines no build steps')

        step_ids = set()
        for step in steps:
            if step.name != 'step':
                raise InvalidRecipeError('Only <step> elements allowed at '
                                         'top level of recipe')
            if not step.attr.get('id'):
                raise InvalidRecipeError('Steps must have an "id" attribute')

            if step.attr['id'] in step_ids:
                raise InvalidRecipeError('Duplicate step ID "%s"' %
                                         step.attr['id'])
            step_ids.add(step.attr['id'])

            cmds = list(step.children())
            if not cmds:
                raise InvalidRecipeError('Step "%s" has no recipe commands' %
                                         step.attr['id'])
            for cmd in cmds:
                if len(list(cmd.children())):
                    raise InvalidRecipeError('Recipe command <%s> has nested '
                                             'content' % cmd.name)
