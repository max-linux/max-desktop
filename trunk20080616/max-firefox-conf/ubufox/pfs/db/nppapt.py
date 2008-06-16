#!/usr/bin/python
#
# Copyright (C) 2007  Canonical Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

from apt import *
import apt_pkg
import os
import shutil

dump_type="sql"

class NppInfo:
	def __init__ ( \
			self, \
			pkgname, \
			pkgdesc, \
			pkglongdesc, \
			name, \
			mimetype, \
			app_id, \
			architecture, \
			distribution, \
			section \
			):
		self.pkgname = pkgname
		self.pkgdesc = pkgdesc
		self.pkglongdesc = pkglongdesc
		self.name = name
		self.mimetype = mimetype
		self.app_id = app_id
		self.architecture = architecture
		self.distribution = distribution
		self.section = section

def get_npp_entries_for_arch_and_distribution (topdir, architecture, dist):

	entries = []
	apt_pkg.init()
	apt_pkg.Config.Set("APT::Architecture", architecture)

	cache = Cache(None, topdir)
	cache.update()
	cache.open(None)

	for pkg in cache:
		record = pkg.candidateRecord
		if record is None:
			record = pkg.installedRecord

		if record is None:
			continue

		if not record.has_key('Npp-Name'):
			continue

		mime_types = record['Npp-MimeType'].split(",");
		app_ids = record['Npp-Applications'].split(",");

		for mime_type in mime_types:
			for app_id in app_ids:
				section = record['Section']
				(real_section, seperator, tail) = section.partition( "/" )
				entries.append( NppInfo ( \
						pkg.name.strip(), \
						pkg.description, \
						pkg.summary, \
						record['Npp-Name'], \
						mime_type.strip(), \
						app_id.strip(), \
						record['Architecture'].strip(), \
						dist.strip(), \
						real_section.strip() \
						))

	return entries

def setup_tmp_cache_tree (dataroot):
	dir = os.tempnam("/tmp/")
	statusdir = dir + "/var/lib/dpkg/"
	statusfile = statusdir + "status"

	aptlistspartialdir = dir + "/var/lib/apt/lists/partial"
	aptarchivespartialdir = dir + "/var/cache/apt/archives/partial"
	aptetcdir = dir + "/etc/apt/"
	aptetcsourcelist = aptetcdir + "sources.list"

	os.makedirs (statusdir)
	f = open(statusfile, 'w')
	f.close()

	os.makedirs (aptlistspartialdir)
	os.makedirs (aptarchivespartialdir)
	os.makedirs (aptetcdir)

	shutil.copyfile(dataroot + "sources.list", aptetcsourcelist)

	return dir


def tear_down_tmp_cache (top):
	for root, dirs, files in os.walk(top, topdown=False):
		for name in files:
			os.remove(os.path.join(root, name))
		for name in dirs:
			os.rmdir(os.path.join(root, name))
	os.rmdir(top)

