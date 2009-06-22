# -*- coding: utf-8 -*-
#
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Implementation of the web administration interface."""

from pkg_resources import require, DistributionNotFound
import re

from trac.core import *
from trac.admin import IAdminPanelProvider
from trac.web.chrome import add_stylesheet, add_script

from bitten.model import BuildConfig, TargetPlatform
from bitten.recipe import Recipe, InvalidRecipeError
from bitten.util import xmlio


class BuildMasterAdminPageProvider(Component):
    """Web administration panel for configuring the build master."""

    implements(IAdminPanelProvider)

    # IAdminPanelProvider methods

    def get_admin_panels(self, req):
        if req.perm.has_permission('BUILD_ADMIN'):
            yield ('bitten', 'Builds', 'master', 'Master Settings')

    def render_admin_panel(self, req, cat, page, path_info):
        from bitten.master import BuildMaster
        master = BuildMaster(self.env)

        if req.method == 'POST':
            self._save_config_changes(req, master)
            req.redirect(req.abs_href.admin(cat, page))

        data = {'master': master}
        add_stylesheet(req, 'bitten/admin.css')
        return 'bitten_admin_master.html', data

    # Internal methods

    def _save_config_changes(self, req, master):
        changed = False

        build_all = 'build_all' in req.args
        if build_all != master.build_all:
            self.config['bitten'].set('build_all',
                                      build_all and 'yes' or 'no')
            changed = True

        adjust_timestamps = 'adjust_timestamps' in req.args
        if adjust_timestamps != master.adjust_timestamps:
            self.config['bitten'].set('adjust_timestamps',
                                      adjust_timestamps and 'yes' or 'no')
            changed = True

        stabilize_wait = int(req.args.get('stabilize_wait', 0))
        if stabilize_wait != master.stabilize_wait:
            self.config['bitten'].set('stabilize_wait', str(stabilize_wait))
            changed = True

        slave_timeout = int(req.args.get('slave_timeout', 0))
        if slave_timeout != master.slave_timeout:
            self.config['bitten'].set('slave_timeout', str(slave_timeout))
            changed = True

        quick_status = 'quick_status' in req.args
        if quick_status != master.quick_status:
            self.config['bitten'].set('quick_status',
                                      quick_status and 'yes' or 'no')
            changed = True

        logs_dir = req.args.get('logs_dir', None)
        if logs_dir != master.logs_dir:
            self.config['bitten'].set('logs_dir', str(logs_dir))
            changed = True

        if changed:
            self.config.save()

        return master


class BuildConfigurationsAdminPageProvider(Component):
    """Web administration panel for configuring the build master."""

    implements(IAdminPanelProvider)

    # IAdminPanelProvider methods

    def get_admin_panels(self, req):
        if req.perm.has_permission('BUILD_MODIFY'):
            yield ('bitten', 'Builds', 'configs', 'Configurations')

    def render_admin_panel(self, req, cat, page, path_info):
        data = {}

        # Analyze url
        try:
            config_name, platform_id = path_info.split('/', 1)
        except:
            config_name = path_info
            platform_id = None

        if config_name: # Existing build config
            if platform_id or (
                    # Editing or creating one of the config's target platforms
                    req.method == 'POST' and 'new' in req.args):

                if platform_id: # Editing target platform
                    platform_id = int(platform_id)
                    platform = TargetPlatform.fetch(self.env, platform_id)

                    if req.method == 'POST':
                        if 'cancel' in req.args or \
                                self._update_platform(req, platform):
                            req.redirect(req.abs_href.admin(cat, page,
                                                            config_name))
                else: # creating target platform
                    if req.method == 'POST':
                        if 'add' in req.args:
                            self._create_platform(req, config_name)
                            req.redirect(req.abs_href.admin(cat, page,
                                                            config_name))
                        elif 'cancel' in req.args:
                            req.redirect(req.abs_href.admin(cat, page,
                                                            config_name))

                    platform = TargetPlatform(self.env, config=config_name)

                # Set up template variables
                data['platform'] = {
                    'id': platform.id, 'name': platform.name,
                    'exists': platform.exists,
                    'rules': [
                        {'property': propname, 'pattern': pattern}
                        for propname, pattern in platform.rules
                    ] or [('', '')]
                }

            else: # Editing existing build config itself
                config = BuildConfig.fetch(self.env, config_name)
                platforms = list(TargetPlatform.select(self.env,
                                                       config=config.name))

                if req.method == 'POST':
                    if 'add' in req.args: # Add target platform
                        platform = self._create_platform(req, config)
                        req.redirect(req.abs_href.admin(cat, page, config.name))

                    elif 'remove' in req.args: # Remove selected platforms
                        self._remove_platforms(req)
                        req.redirect(req.abs_href.admin(cat, page, config.name))

                    elif 'save' in req.args: # Save this build config
                        self._update_config(req, config)

                    req.redirect(req.abs_href.admin(cat, page))

                # Prepare template variables
                data['config'] = {
                    'name': config.name, 'label': config.label or config.name,
                    'active': config.active, 'path': config.path,
                    'min_rev': config.min_rev, 'max_rev': config.max_rev,
                    'description': config.description,
                    'recipe': config.recipe,
                    'platforms': [{
                        'name': platform.name,
                        'id': platform.id,
                        'href': req.href.admin('bitten', 'configs', config.name,
                                               platform.id),
                        'rules': [{'property': propname, 'pattern': pattern}
                                   for propname, pattern in platform.rules]
                    } for platform in platforms]
                }

        else: # At the top level build config list
            if req.method == 'POST':
                if 'add' in req.args: # Add build config
                    config = self._create_config(req)
                    req.redirect(req.abs_href.admin(cat, page, config.name))

                elif 'remove' in req.args: # Remove selected build configs
                    self._remove_configs(req)

                elif 'apply' in req.args: # Update active state of configs
                    self._activate_configs(req)
                req.redirect(req.abs_href.admin(cat, page))

            # Prepare template variables
            configs = []
            for config in BuildConfig.select(self.env, include_inactive=True):
                configs.append({
                    'name': config.name, 'label': config.label or config.name,
                    'active': config.active, 'path': config.path,
                    'min_rev': config.min_rev, 'max_rev': config.max_rev,
                    'href': req.href.admin('bitten', 'configs', config.name),
                })
            data['configs'] = configs

        add_stylesheet(req, 'bitten/admin.css')
        add_script(req, 'common/js/suggest.js')
        return 'bitten_admin_configs.html', data

    # Internal methods

    def _activate_configs(self, req):
        req.perm.assert_permission('BUILD_MODIFY')

        active = req.args.get('active') or []
        active = isinstance(active, list) and active or [active]

        db = self.env.get_db_cnx()
        for config in list(BuildConfig.select(self.env, db=db,
                                              include_inactive=True)):
            config.active = config.name in active
            config.update(db=db)
        db.commit()

    def _create_config(self, req):
        req.perm.assert_permission('BUILD_CREATE')

        config = BuildConfig(self.env)
        self._update_config(req, config)
        return config

    def _remove_configs(self, req):
        req.perm.assert_permission('BUILD_DELETE')

        sel = req.args.get('sel')
        if not sel:
            raise TracError('No configuration selected')
        sel = isinstance(sel, list) and sel or [sel]

        db = self.env.get_db_cnx()
        for name in sel:
            config = BuildConfig.fetch(self.env, name, db=db)
            if not config:
                raise TracError('Configuration %r not found' % name)
            config.delete(db=db)
        db.commit()

    def _update_config(self, req, config):
        req.perm.assert_permission('BUILD_MODIFY')

        name = req.args.get('name')
        if not name:
            raise TracError('Missing required field "name"', 'Missing Field')
        if not re.match(r'^[\w.-]+$', name):
            raise TracError('The field "name" may only contain letters, '
                            'digits, periods, or dashes.', 'Invalid Field')

        path = req.args.get('path', '')
        repos = self.env.get_repository(req.authname)
        max_rev = req.args.get('max_rev') or None
        try:
            node = repos.get_node(path, max_rev)
            assert node.isdir, '%s is not a directory' % node.path
        except (AssertionError, TracError), e:
            raise TracError(unicode(e), 'Invalid Repository Path')
        if req.args.get('min_rev'):
            try:
                repos.get_node(path, req.args.get('min_rev'))
            except TracError, e:
                raise TracError(unicode(e), 'Invalid Oldest Revision')

        recipe_xml = req.args.get('recipe', '')
        if recipe_xml:
            try:
                Recipe(xmlio.parse(recipe_xml)).validate()
            except xmlio.ParseError, e:
                raise TracError('Failure parsing recipe: %s' % e,
                                'Invalid Recipe')
            except InvalidRecipeError, e:
                raise TracError(unicode(e), 'Invalid Recipe')

        config.name = name
        config.path = repos.normalize_path(path)
        config.recipe = recipe_xml
        config.min_rev = req.args.get('min_rev')
        config.max_rev = req.args.get('max_rev')
        config.label = req.args.get('label', config.name)
        config.description = req.args.get('description', '')

        if config.exists:
            config.update()
        else:
            config.insert()

    def _create_platform(self, req, config_name):
        req.perm.assert_permission('BUILD_MODIFY')

        name = req.args.get('name')
        if not name:
            raise TracError('Missing required field "name"', 'Missing field')

        platform = TargetPlatform(self.env, config=config_name, name=name)
        self._update_platform(req, platform)
        return platform

    def _remove_platforms(self, req):
        req.perm.assert_permission('BUILD_MODIFY')

        sel = req.args.get('sel')
        if not sel:
            raise TracError('No platform selected')
        sel = isinstance(sel, list) and sel or [sel]

        db = self.env.get_db_cnx()
        for platform_id in sel:
            platform = TargetPlatform.fetch(self.env, platform_id, db=db)
            if not platform:
                raise TracError('Target platform %r not found' % platform_id)
            platform.delete(db=db)
        db.commit()

    def _update_platform(self, req, platform):
        platform.name = req.args.get('name')

        properties = [int(key[9:]) for key in req.args.keys()
                      if key.startswith('property_')]
        properties.sort()
        patterns = [int(key[8:]) for key in req.args.keys()
                    if key.startswith('pattern_')]
        patterns.sort()
        platform.rules = [(req.args.get('property_%d' % property),
                           req.args.get('pattern_%d' % pattern))
                          for property, pattern in zip(properties, patterns)
                          if req.args.get('property_%d' % property)]

        if platform.exists:
            platform.update()
        else:
            platform.insert()

        add_rules = [int(key[9:]) for key in req.args.keys()
                     if key.startswith('add_rule_')]
        if add_rules:
            platform.rules.insert(add_rules[0] + 1, ('', ''))
            return False
        rm_rules = [int(key[8:]) for key in req.args.keys()
                    if key.startswith('rm_rule_')]
        if rm_rules:
            if rm_rules[0] < len(platform.rules):
                del platform.rules[rm_rules[0]]
            return False

        return True
