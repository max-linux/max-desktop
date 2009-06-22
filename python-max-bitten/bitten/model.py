# -*- coding: utf-8 -*-
#
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.

"""Model classes for objects persisted in the database."""

from trac.db import Table, Column, Index
from trac.util.text import to_unicode
import codecs
import os

__docformat__ = 'restructuredtext en'


class BuildConfig(object):
    """Representation of a build configuration."""

    _schema = [
        Table('bitten_config', key='name')[
            Column('name'), Column('path'), Column('active', type='int'),
            Column('recipe'), Column('min_rev'), Column('max_rev'),
            Column('label'), Column('description')
        ]
    ]

    def __init__(self, env, name=None, path=None, active=False, recipe=None,
                 min_rev=None, max_rev=None, label=None, description=None):
        """Initialize a new build configuration with the specified attributes.

        To actually create this configuration in the database, the `insert`
        method needs to be called.
        """
        self.env = env
        self._old_name = None
        self.name = name
        self.path = path or ''
        self.active = bool(active)
        self.recipe = recipe or ''
        self.min_rev = min_rev or None
        self.max_rev = max_rev or None
        self.label = label or ''
        self.description = description or ''

    def __repr__(self):
        return '<%s %r>' % (type(self).__name__, self.name)

    exists = property(fget=lambda self: self._old_name is not None,
                      doc='Whether this configuration exists in the database')

    def delete(self, db=None):
        """Remove a build configuration and all dependent objects from the
        database."""
        assert self.exists, 'Cannot delete non-existing configuration'
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        for platform in list(TargetPlatform.select(self.env, self.name, db=db)):
            platform.delete(db=db)

        for build in list(Build.select(self.env, config=self.name, db=db)):
            build.delete(db=db)

        cursor = db.cursor()
        cursor.execute("DELETE FROM bitten_config WHERE name=%s", (self.name,))

        if handle_ta:
            db.commit()
        self._old_name = None

    def insert(self, db=None):
        """Insert a new configuration into the database."""
        assert not self.exists, 'Cannot insert existing configuration'
        assert self.name, 'Configuration requires a name'
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_config (name,path,active,"
                       "recipe,min_rev,max_rev,label,description) "
                       "VALUES (%s,%s,%s,%s,%s,%s,%s,%s)",
                       (self.name, self.path, int(self.active or 0),
                        self.recipe or '', self.min_rev, self.max_rev,
                        self.label or '', self.description or ''))

        if handle_ta:
            db.commit()
        self._old_name = self.name

    def update(self, db=None):
        """Save changes to an existing build configuration."""
        assert self.exists, 'Cannot update a non-existing configuration'
        assert self.name, 'Configuration requires a name'
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        cursor = db.cursor()
        cursor.execute("UPDATE bitten_config SET name=%s,path=%s,active=%s,"
                       "recipe=%s,min_rev=%s,max_rev=%s,label=%s,"
                       "description=%s WHERE name=%s",
                       (self.name, self.path, int(self.active or 0),
                        self.recipe, self.min_rev, self.max_rev,
                        self.label, self.description, self._old_name))
        if self.name != self._old_name:
            cursor.execute("UPDATE bitten_platform SET config=%s "
                           "WHERE config=%s", (self.name, self._old_name))
            cursor.execute("UPDATE bitten_build SET config=%s "
                           "WHERE config=%s", (self.name, self._old_name))

        if handle_ta:
            db.commit()
        self._old_name = self.name

    def fetch(cls, env, name, db=None):
        """Retrieve an existing build configuration from the database by
        name.
        """
        if not db:
            db = env.get_db_cnx()

        cursor = db.cursor()
        cursor.execute("SELECT path,active,recipe,min_rev,max_rev,label,"
                       "description FROM bitten_config WHERE name=%s", (name,))
        row = cursor.fetchone()
        if not row:
            return None

        config = BuildConfig(env)
        config.name = config._old_name = name
        config.path = row[0] or ''
        config.active = bool(row[1])
        config.recipe = row[2] or ''
        config.min_rev = row[3] or None
        config.max_rev = row[4] or None
        config.label = row[5] or ''
        config.description = row[6] or ''
        return config

    fetch = classmethod(fetch)

    def select(cls, env, include_inactive=False, db=None):
        """Retrieve existing build configurations from the database that match
        the specified criteria.
        """
        if not db:
            db = env.get_db_cnx()

        cursor = db.cursor()
        if include_inactive:
            cursor.execute("SELECT name,path,active,recipe,min_rev,max_rev,"
                           "label,description FROM bitten_config ORDER BY name")
        else:
            cursor.execute("SELECT name,path,active,recipe,min_rev,max_rev,"
                           "label,description FROM bitten_config "
                           "WHERE active=1 ORDER BY name")
        for name, path, active, recipe, min_rev, max_rev, label, description \
                in cursor:
            config = BuildConfig(env, name=name, path=path or '',
                                 active=bool(active), recipe=recipe or '',
                                 min_rev=min_rev or None,
                                 max_rev=max_rev or None, label=label or '',
                                 description=description or '')
            config._old_name = name
            yield config

    select = classmethod(select)


class TargetPlatform(object):
    """Target platform for a build configuration."""

    _schema = [
        Table('bitten_platform', key='id')[
            Column('id', auto_increment=True), Column('config'), Column('name')
        ],
        Table('bitten_rule', key=('id', 'propname'))[
            Column('id', type='int'), Column('propname'), Column('pattern'),
            Column('orderno', type='int')
        ]
    ]

    def __init__(self, env, config=None, name=None):
        """Initialize a new target platform with the specified attributes.

        To actually create this platform in the database, the `insert` method
        needs to be called.
        """
        self.env = env
        self.id = None
        self.config = config
        self.name = name
        self.rules = []

    def __repr__(self):
        return '<%s %r>' % (type(self).__name__, self.id)

    exists = property(fget=lambda self: self.id is not None,
                      doc='Whether this target platform exists in the database')

    def delete(self, db=None):
        """Remove the target platform from the database."""
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        for build in Build.select(self.env, platform=self.id, status=Build.PENDING, db=db):
            build.delete()

        cursor = db.cursor()
        cursor.execute("DELETE FROM bitten_rule WHERE id=%s", (self.id,))
        cursor.execute("DELETE FROM bitten_platform WHERE id=%s", (self.id,))
        if handle_ta:
            db.commit()

    def insert(self, db=None):
        """Insert a new target platform into the database."""
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        assert not self.exists, 'Cannot insert existing target platform'
        assert self.config, 'Target platform needs to be associated with a ' \
                            'configuration'
        assert self.name, 'Target platform requires a name'

        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_platform (config,name) "
                       "VALUES (%s,%s)", (self.config, self.name))
        self.id = db.get_last_id(cursor, 'bitten_platform')
        if self.rules:
            cursor.executemany("INSERT INTO bitten_rule VALUES (%s,%s,%s,%s)",
                               [(self.id, propname, pattern, idx)
                                for idx, (propname, pattern)
                                in enumerate(self.rules)])

        if handle_ta:
            db.commit()

    def update(self, db=None):
        """Save changes to an existing target platform."""
        assert self.exists, 'Cannot update a non-existing platform'
        assert self.config, 'Target platform needs to be associated with a ' \
                            'configuration'
        assert self.name, 'Target platform requires a name'
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        cursor = db.cursor()
        cursor.execute("UPDATE bitten_platform SET name=%s WHERE id=%s",
                       (self.name, self.id))
        cursor.execute("DELETE FROM bitten_rule WHERE id=%s", (self.id,))
        if self.rules:
            cursor.executemany("INSERT INTO bitten_rule VALUES (%s,%s,%s,%s)",
                               [(self.id, propname, pattern, idx)
                                for idx, (propname, pattern)
                                in enumerate(self.rules)])

        if handle_ta:
            db.commit()

    def fetch(cls, env, id, db=None):
        """Retrieve an existing target platform from the database by ID."""
        if not db:
            db = env.get_db_cnx()

        cursor = db.cursor()
        cursor.execute("SELECT config,name FROM bitten_platform "
                       "WHERE id=%s", (id,))
        row = cursor.fetchone()
        if not row:
            return None

        platform = TargetPlatform(env, config=row[0], name=row[1])
        platform.id = id
        cursor.execute("SELECT propname,pattern FROM bitten_rule "
                       "WHERE id=%s ORDER BY orderno", (id,))
        for propname, pattern in cursor:
            platform.rules.append((propname, pattern))
        return platform

    fetch = classmethod(fetch)

    def select(cls, env, config=None, db=None):
        """Retrieve existing target platforms from the database that match the
        specified criteria.
        """
        if not db:
            db = env.get_db_cnx()

        where_clauses = []
        if config is not None:
            where_clauses.append(("config=%s", config))
        if where_clauses:
            where = "WHERE " + " AND ".join([wc[0] for wc in where_clauses])
        else:
            where = ""

        cursor = db.cursor()
        cursor.execute("SELECT id FROM bitten_platform %s ORDER BY name"
                       % where, [wc[1] for wc in where_clauses])
        for (id,) in cursor:
            yield TargetPlatform.fetch(env, id)

    select = classmethod(select)


class Build(object):
    """Representation of a build."""

    _schema = [
        Table('bitten_build', key='id')[
            Column('id', auto_increment=True), Column('config'), Column('rev'),
            Column('rev_time', type='int'), Column('platform', type='int'),
            Column('slave'), Column('started', type='int'),
            Column('stopped', type='int'), Column('status', size=1),
            Index(['config', 'rev', 'slave'])
        ],
        Table('bitten_slave', key=('build', 'propname'))[
            Column('build', type='int'), Column('propname'), Column('propvalue')
        ]
    ]

    # Build status codes
    PENDING = 'P'
    IN_PROGRESS = 'I'
    SUCCESS = 'S'
    FAILURE = 'F'

    # Standard slave properties
    IP_ADDRESS = 'ipnr'
    MAINTAINER = 'owner'
    OS_NAME = 'os'
    OS_FAMILY = 'family'
    OS_VERSION = 'version'
    MACHINE = 'machine'
    PROCESSOR = 'processor'

    def __init__(self, env, config=None, rev=None, platform=None, slave=None,
                 started=0, stopped=0, rev_time=0, status=PENDING):
        """Initialize a new build with the specified attributes.

        To actually create this build in the database, the `insert` method needs
        to be called.
        """
        self.env = env
        self.id = None
        self.config = config
        self.rev = rev and str(rev) or None
        self.platform = platform
        self.slave = slave
        self.started = started or 0
        self.stopped = stopped or 0
        self.rev_time = rev_time
        self.status = status
        self.slave_info = {}

    def __repr__(self):
        return '<%s %r>' % (type(self).__name__, self.id)

    exists = property(fget=lambda self: self.id is not None,
                      doc='Whether this build exists in the database')
    completed = property(fget=lambda self: self.status != Build.IN_PROGRESS,
                         doc='Whether the build has been completed')
    successful = property(fget=lambda self: self.status == Build.SUCCESS,
                          doc='Whether the build was successful')

    def delete(self, db=None):
        """Remove the build from the database."""
        assert self.exists, 'Cannot delete a non-existing build'
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        for step in list(BuildStep.select(self.env, build=self.id)):
            step.delete(db=db)

        cursor = db.cursor()
        cursor.execute("DELETE FROM bitten_slave WHERE build=%s", (self.id,))
        cursor.execute("DELETE FROM bitten_build WHERE id=%s", (self.id,))

        if handle_ta:
            db.commit()

    def insert(self, db=None):
        """Insert a new build into the database."""
        assert not self.exists, 'Cannot insert an existing build'
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        assert self.config and self.rev and self.rev_time and self.platform
        assert self.status in (self.PENDING, self.IN_PROGRESS, self.SUCCESS,
                               self.FAILURE)
        if not self.slave:
            assert self.status == self.PENDING

        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_build (config,rev,rev_time,platform,"
                       "slave,started,stopped,status) "
                       "VALUES (%s,%s,%s,%s,%s,%s,%s,%s)",
                       (self.config, self.rev, int(self.rev_time),
                        self.platform, self.slave or '', self.started or 0,
                        self.stopped or 0, self.status))
        self.id = db.get_last_id(cursor, 'bitten_build')
        if self.slave_info:
            cursor.executemany("INSERT INTO bitten_slave VALUES (%s,%s,%s)",
                               [(self.id, name, value) for name, value
                                in self.slave_info.items()])

        if handle_ta:
            db.commit()

    def update(self, db=None):
        """Save changes to an existing build."""
        assert self.exists, 'Cannot update a non-existing build'
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        assert self.config and self.rev
        assert self.status in (self.PENDING, self.IN_PROGRESS, self.SUCCESS,
                               self.FAILURE)
        if not self.slave:
            assert self.status == self.PENDING

        cursor = db.cursor()
        cursor.execute("UPDATE bitten_build SET slave=%s,started=%s,"
                       "stopped=%s,status=%s WHERE id=%s",
                       (self.slave or '', self.started or 0,
                        self.stopped or 0, self.status, self.id))
        cursor.execute("DELETE FROM bitten_slave WHERE build=%s", (self.id,))
        if self.slave_info:
            cursor.executemany("INSERT INTO bitten_slave VALUES (%s,%s,%s)",
                               [(self.id, name, value) for name, value
                                in self.slave_info.items()])
        if handle_ta:
            db.commit()

    def fetch(cls, env, id, db=None):
        """Retrieve an existing build from the database by ID."""
        if not db:
            db = env.get_db_cnx()

        cursor = db.cursor()
        cursor.execute("SELECT config,rev,rev_time,platform,slave,started,"
                       "stopped,status FROM bitten_build WHERE id=%s", (id,))
        row = cursor.fetchone()
        if not row:
            return None

        build = Build(env, config=row[0], rev=row[1], rev_time=int(row[2]),
                      platform=int(row[3]), slave=row[4],
                      started=row[5] and int(row[5]) or 0,
                      stopped=row[6] and int(row[6]) or 0, status=row[7])
        build.id = int(id)
        cursor.execute("SELECT propname,propvalue FROM bitten_slave "
                       "WHERE build=%s", (id,))
        for propname, propvalue in cursor:
            build.slave_info[propname] = propvalue
        return build

    fetch = classmethod(fetch)

    def select(cls, env, config=None, rev=None, platform=None, slave=None,
               status=None, db=None):
        """Retrieve existing builds from the database that match the specified
        criteria.
        """
        if not db:
            db = env.get_db_cnx()

        where_clauses = []
        if config is not None:
            where_clauses.append(("config=%s", config))
        if rev is not None:
            where_clauses.append(("rev=%s", str(rev)))
        if platform is not None:
            where_clauses.append(("platform=%s", platform))
        if slave is not None:
            where_clauses.append(("slave=%s", slave))
        if status is not None:
            where_clauses.append(("status=%s", status))
        if where_clauses:
            where = "WHERE " + " AND ".join([wc[0] for wc in where_clauses])
        else:
            where = ""

        cursor = db.cursor()
        cursor.execute("SELECT id FROM bitten_build %s "
                       "ORDER BY config,rev_time DESC,slave"
                       % where, [wc[1] for wc in where_clauses])
        for (id,) in cursor:
            yield Build.fetch(env, id)
    select = classmethod(select)


class BuildStep(object):
    """Represents an individual step of an executed build."""

    _schema = [
        Table('bitten_step', key=('build', 'name'))[
            Column('build', type='int'), Column('name'), Column('description'),
            Column('status', size=1), Column('started', type='int'),
            Column('stopped', type='int')
        ],
        Table('bitten_error', key=('build', 'step', 'orderno'))[
            Column('build', type='int'), Column('step'), Column('message'),
            Column('orderno', type='int')
        ]
    ]

    # Step status codes
    SUCCESS = 'S'
    FAILURE = 'F'

    def __init__(self, env, build=None, name=None, description=None,
                 status=None, started=None, stopped=None):
        """Initialize a new build step with the specified attributes.

        To actually create this build step in the database, the `insert` method
        needs to be called.
        """
        self.env = env
        self.build = build
        self.name = name
        self.description = description
        self.status = status
        self.started = started
        self.stopped = stopped
        self.errors = []
        self._exists = False

    exists = property(fget=lambda self: self._exists,
                      doc='Whether this build step exists in the database')
    successful = property(fget=lambda self: self.status == BuildStep.SUCCESS,
                          doc='Whether the build step was successful')

    def delete(self, db=None):
        """Remove the build step from the database."""
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        for log in list(BuildLog.select(self.env, build=self.build,
                                        step=self.name, db=db)):
            log.delete(db=db)
        for report in list(Report.select(self.env, build=self.build,
                                         step=self.name, db=db)):
            report.delete(db=db)

        cursor = db.cursor()
        cursor.execute("DELETE FROM bitten_step WHERE build=%s AND name=%s",
                       (self.build, self.name))
        cursor.execute("DELETE FROM bitten_error WHERE build=%s AND step=%s",
                       (self.build, self.name))

        if handle_ta:
            db.commit()
        self._exists = False

    def insert(self, db=None):
        """Insert a new build step into the database."""
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        assert self.build and self.name
        assert self.status in (self.SUCCESS, self.FAILURE)

        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_step (build,name,description,status,"
                       "started,stopped) VALUES (%s,%s,%s,%s,%s,%s)",
                       (self.build, self.name, self.description or '',
                        self.status, self.started or 0, self.stopped or 0))
        if self.errors:
            cursor.executemany("INSERT INTO bitten_error (build,step,message,"
                               "orderno) VALUES (%s,%s,%s,%s)",
                               [(self.build, self.name, message, idx)
                                for idx, message in enumerate(self.errors)])

        if handle_ta:
            db.commit()
        self._exists = True

    def fetch(cls, env, build, name, db=None):
        """Retrieve an existing build from the database by build ID and step
        name."""
        if not db:
            db = env.get_db_cnx()

        cursor = db.cursor()
        cursor.execute("SELECT description,status,started,stopped "
                       "FROM bitten_step WHERE build=%s AND name=%s",
                       (build, name))
        row = cursor.fetchone()
        if not row:
            return None
        step = BuildStep(env, build, name, row[0] or '', row[1],
                         row[2] and int(row[2]), row[3] and int(row[3]))
        step._exists = True

        cursor.execute("SELECT message FROM bitten_error WHERE build=%s "
                       "AND step=%s ORDER BY orderno", (build, name))
        for row in cursor:
            step.errors.append(row[0] or '')
        return step

    fetch = classmethod(fetch)

    def select(cls, env, build=None, name=None, status=None, db=None):
        """Retrieve existing build steps from the database that match the
        specified criteria.
        """
        if not db:
            db = env.get_db_cnx()

        assert status in (None, BuildStep.SUCCESS, BuildStep.FAILURE)

        where_clauses = []
        if build is not None:
            where_clauses.append(("build=%s", build))
        if name is not None:
            where_clauses.append(("name=%s", name))
        if status is not None:
            where_clauses.append(("status=%s", status))
        if where_clauses:
            where = "WHERE " + " AND ".join([wc[0] for wc in where_clauses])
        else:
            where = ""

        cursor = db.cursor()
        cursor.execute("SELECT build,name FROM bitten_step %s ORDER BY stopped"
                       % where, [wc[1] for wc in where_clauses])
        for build, name in cursor:
            yield BuildStep.fetch(env, build, name, db=db)

    select = classmethod(select)


class BuildLog(object):
    """Represents a build log."""

    _schema = [
        Table('bitten_log', key='id')[
            Column('id', auto_increment=True), Column('build', type='int'),
            Column('step'), Column('generator'), Column('orderno', type='int'),
            Column('filename'),
            Index(['build', 'step'])
        ],
    ]

    # Message levels
    DEBUG = 'D'
    INFO = 'I'
    WARNING = 'W'
    ERROR = 'E'
    UNKNOWN = ''

    def __init__(self, env, build=None, step=None, generator=None,
                 orderno=None, filename=None):
        """Initialize a new build log with the specified attributes.

        To actually create this build log in the database, the `insert` method
        needs to be called.
        """
        self.env = env
        self.id = None
        self.build = build
        self.step = step
        self.generator = generator or ''
        self.orderno = orderno and int(orderno) or 0
        self.filename = filename or None
        self.messages = []
        self.logs_dir = env.config.get('bitten', 'logs_dir', 'log/bitten')
        if not os.path.isabs(self.logs_dir):
            self.logs_dir = os.path.join(env.path, self.logs_dir)
        if not os.path.exists(self.logs_dir):
            os.makedirs(self.logs_dir)

    exists = property(fget=lambda self: self.id is not None,
                      doc='Whether this build log exists in the database')

    def get_log_file(self, filename):
        """Returns the full path to the log file"""
        if filename != os.path.basename(filename):
            raise ValueError("Filename may not contain path: %s" % (filename,))
        return os.path.join(self.logs_dir, filename)

    def delete(self, db=None):
        """Remove the build log from the database."""
        assert self.exists, 'Cannot delete a non-existing build log'
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        cursor = db.cursor()
        cursor.execute("SELECT filename FROM bitten_log WHERE id=%s", (self.id,))
        log_files = cursor.fetchall() or []
        cursor.execute("DELETE FROM bitten_log WHERE id=%s", (self.id,))
        for log_file in log_files:
            log_file = self.get_log_file(log_file[0])
            if os.path.exists(log_file):
                try:
                    os.remove(log_file)
                except Exception, e:
                    self.env.log.warning("Error removing log file %s: %s" % (log_file, e))
            level_file = self.get_log_file(log_file[1])
            if os.path.exists(log_file):
                try:
                    os.remove(level_file)
                except Exception, e:
                    self.env.log.warning("Error removing log file %s: %s" % (log_file, e))

        if handle_ta:
            db.commit()
        self.id = None

    def insert(self, db=None):
        """Insert a new build log into the database."""
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        assert self.build and self.step

        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_log (build,step,generator,orderno) "
                       "VALUES (%s,%s,%s,%s)", (self.build, self.step,
                       self.generator, self.orderno))
        id = db.get_last_id(cursor, 'bitten_log')
        log_file = "%s.log" % (id,)
        cursor.execute("UPDATE bitten_log SET filename=%s WHERE id=%s", (log_file, id))
        if self.messages:
            log_file_name = self.get_log_file(log_file)
            level_file_name = log_file_name + ".levels"
            codecs.open(log_file_name, "wb", "UTF-8").writelines([to_unicode(msg[1]+"\n") for msg in self.messages])
            codecs.open(level_file_name, "wb", "UTF-8").writelines([to_unicode(msg[0]+"\n") for msg in self.messages])

        if handle_ta:
            db.commit()
        self.id = id

    def fetch(cls, env, id, db=None):
        """Retrieve an existing build from the database by ID."""
        if not db:
            db = env.get_db_cnx()

        cursor = db.cursor()
        cursor.execute("SELECT build,step,generator,orderno,filename FROM bitten_log "
                       "WHERE id=%s", (id,))
        row = cursor.fetchone()
        if not row:
            return None
        log = BuildLog(env, int(row[0]), row[1], row[2], row[3], row[4])
        log.id = id
        if log.filename:
            log_filename = log.get_log_file(log.filename)
            if os.path.exists(log_filename):
                log_lines = codecs.open(log_filename, "rb", "UTF-8").readlines()
            else:
                log_lines = []
            level_filename = log.get_log_file(log.filename + ".levels")
            if os.path.exists(level_filename):
                log_levels = dict(enumerate(codecs.open(level_filename, "rb", "UTF-8").readlines()))
            else:
                log_levels = {}
            log.messages = [(log_levels.get(line_num, BuildLog.UNKNOWN).rstrip("\n"), line.rstrip("\n")) for line_num, line in enumerate(log_lines)]
        else:
            log.messages = []

        return log

    fetch = classmethod(fetch)

    def select(cls, env, build=None, step=None, generator=None, db=None):
        """Retrieve existing build logs from the database that match the
        specified criteria.
        """
        if not db:
            db = env.get_db_cnx()

        where_clauses = []
        if build is not None:
            where_clauses.append(("build=%s", build))
        if step is not None:
            where_clauses.append(("step=%s", step))
        if generator is not None:
            where_clauses.append(("generator=%s", generator))
        if where_clauses:
            where = "WHERE " + " AND ".join([wc[0] for wc in where_clauses])
        else:
            where = ""

        cursor = db.cursor()
        cursor.execute("SELECT id FROM bitten_log %s ORDER BY orderno"
                       % where, [wc[1] for wc in where_clauses])
        for (id, ) in cursor:
            yield BuildLog.fetch(env, id, db=db)

    select = classmethod(select)


class Report(object):
    """Represents a generated report."""

    _schema = [
        Table('bitten_report', key='id')[
            Column('id', auto_increment=True), Column('build', type='int'),
            Column('step'), Column('category'), Column('generator'),
            Index(['build', 'step', 'category'])
        ],
        Table('bitten_report_item', key=('report', 'item', 'name'))[
            Column('report', type='int'), Column('item', type='int'),
            Column('name'), Column('value')
        ]
    ]

    def __init__(self, env, build=None, step=None, category=None,
                 generator=None):
        """Initialize a new report with the specified attributes.

        To actually create this build log in the database, the `insert` method
        needs to be called.
        """
        self.env = env
        self.id = None
        self.build = build
        self.step = step
        self.category = category
        self.generator = generator or ''
        self.items = []

    exists = property(fget=lambda self: self.id is not None,
                      doc='Whether this report exists in the database')

    def delete(self, db=None):
        """Remove the report from the database."""
        assert self.exists, 'Cannot delete a non-existing report'
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        cursor = db.cursor()
        cursor.execute("DELETE FROM bitten_report_item WHERE report=%s",
                       (self.id,))
        cursor.execute("DELETE FROM bitten_report WHERE id=%s", (self.id,))

        if handle_ta:
            db.commit()
        self.id = None

    def insert(self, db=None):
        """Insert a new build log into the database."""
        if not db:
            db = self.env.get_db_cnx()
            handle_ta = True
        else:
            handle_ta = False

        assert self.build and self.step and self.category

        # Enforce uniqueness of build-step-category.
        # This should be done by the database, but the DB schema helpers in Trac
        # currently don't support UNIQUE() constraints
        assert not list(Report.select(self.env, build=self.build,
                                      step=self.step, category=self.category,
                                      db=db)), 'Report already exists'

        cursor = db.cursor()
        cursor.execute("INSERT INTO bitten_report "
                       "(build,step,category,generator) VALUES (%s,%s,%s,%s)",
                       (self.build, self.step, self.category, self.generator))
        id = db.get_last_id(cursor, 'bitten_report')
        for idx, item in enumerate([item for item in self.items if item]):
            cursor.executemany("INSERT INTO bitten_report_item "
                               "(report,item,name,value) VALUES (%s,%s,%s,%s)",
                               [(id, idx, key, value) for key, value
                                in item.items()])

        if handle_ta:
            db.commit()
        self.id = id

    def fetch(cls, env, id, db=None):
        """Retrieve an existing build from the database by ID."""
        if not db:
            db = env.get_db_cnx()

        cursor = db.cursor()
        cursor.execute("SELECT build,step,category,generator "
                       "FROM bitten_report WHERE id=%s", (id,))
        row = cursor.fetchone()
        if not row:
            return None
        report = Report(env, int(row[0]), row[1], row[2] or '', row[3] or '')
        report.id = id

        cursor.execute("SELECT item,name,value FROM bitten_report_item "
                       "WHERE report=%s ORDER BY item", (id,))
        items = {}
        for item, name, value in cursor:
            items.setdefault(item, {})[name] = value
        report.items = items.values()

        return report

    fetch = classmethod(fetch)

    def select(cls, env, config=None, build=None, step=None, category=None,
               db=None):
        """Retrieve existing reports from the database that match the specified
        criteria.
        """
        where_clauses = []
        joins = []
        if config is not None:
            where_clauses.append(("config=%s", config))
            joins.append("INNER JOIN bitten_build ON (bitten_build.id=build)")
        if build is not None:
            where_clauses.append(("build=%s", build))
        if step is not None:
            where_clauses.append(("step=%s", step))
        if category is not None:
            where_clauses.append(("category=%s", category))

        if where_clauses:
            where = "WHERE " + " AND ".join([wc[0] for wc in where_clauses])
        else:
            where = ""

        if not db:
            db = env.get_db_cnx()
        cursor = db.cursor()
        cursor.execute("SELECT bitten_report.id FROM bitten_report %s %s "
                       "ORDER BY category" % (' '.join(joins), where),
                       [wc[1] for wc in where_clauses])
        for (id, ) in cursor:
            yield Report.fetch(env, id, db=db)

    select = classmethod(select)


schema = BuildConfig._schema + TargetPlatform._schema + Build._schema + \
         BuildStep._schema + BuildLog._schema + Report._schema
schema_version = 9
