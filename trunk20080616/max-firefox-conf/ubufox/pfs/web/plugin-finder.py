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

import httplib

from cStringIO import StringIO
from mod_python import apache
from pysqlite2 import dbapi2 as sqlite
from xml.dom.xmlbuilder import *
from xml.dom.minidom import *
import os
import time
import thread
import threading
import sys
import plugindata

# TestURL: http://localhost/~asac/cgi-bin/PluginFinderService.php?mimetype=%PLUGIN_MIMETYPE%&appID=%APP_ID%&appVersion=%APP_VERSION%&clientOS=%CLIENT_OS%&chromeLocale=%CHROME_LOCALE%

counter=0
docroot=""
CACHE_TIME_SEC=3600
DB_DIR="/home/asac/pfsdb/"
ENABLE_MOZILLA_PFS_RESULTS=None

arch_map=dict()
cache_con_map=dict()

arch_map['x86_64']="amd64"
arch_map['i686']="i386"

def connect_db(docroot):
	apt_con = sqlite.connect(DB_DIR+"apt-plugins.sqlite")
	return apt_con

def get_db(req):
	apt_con = connect_db(req.document_root())
	return apt_con

def retrieve_proxied_result(req):
	builder = DOMBuilder()
	result_doc = None
	if ENABLE_MOZILLA_PFS_RESULTS:
		hc = httplib.HTTPSConnection("pfs.mozilla.org")
		hc.request("GET", "/plugins/PluginFinderService.php?" + req.args)
		res = hc.getresponse()
		source = DOMInputSource()
		source.byteStream = StringIO(res.read())
		result_doc = builder.parse(source)
	else:
		result_doc = Document()
		root = result_doc.createElement("RDF:RDF")
		root.setAttribute("xmlns:RDF", "http://www.w3.org/1999/02/22-rdf-syntax-ns#")
		root.setAttribute("xmlns:pfs", "http://www.mozilla.org/2004/pfs-rdf#")
		result_doc.appendChild(root)

	return result_doc


def parse_http_parameters (req):
	parameters = dict()
	for parameterblock in req.args.split("&"):
		keyvaluepair = parameterblock.split("=")
		if len(keyvaluepair) < 2:
			continue
		parameters[url_unquote(keyvaluepair[0])] = url_unquote(keyvaluepair[1])
	return parameters


def retrieve_descriptions(req, parameters):
	ret = []
	apt_con = get_db(req)

	mimetype = parameters['mimetype']
	architecture = None
	if not parameters['clientOS'] is None:
		tmp = parameters['clientOS'].split(" ")
		architecture = arch_map[tmp[1]]
		if architecture is None:
			architecture = tmp[1]
	else:
		return ret # nothing to do for no arch

        distribution = "7.10"
        if parameters.has_key('distributionID') and parameters['distributionID'] is not None:
		distribution = parameters['distributionID']

	appid = parameters['appID']

	cur = apt_con.cursor()
	cur.execute("SELECT name, mimetype, pkgname, pkgdesc, section FROM package " \
			+ "WHERE mimetype=? AND architecture=? AND appid=? " \
			+ "AND distribution=?", (mimetype, architecture, appid, distribution))
	for row in cur:
		desc = plugindata.PluginDescription ( 		\
				row[0], 	\
				row[1], 	\
				None, 		\
				None,		\
				None,		\
				"apt:" + row[2] + "?section=" + row[4],	\
				None,	\
				None,	\
				None,		\
				"false")
		ret.append(desc)

	cur.close()

	return ret		

def retrieve_ubuntu_results(req, http_parameters):
	description_results = []
	description_results = retrieve_descriptions(req, http_parameters)
	return description_results

def url_unquote(s):
	res = s.split('%')
	for i in xrange(1, len(res)):
		item = res[i]
		try:
			res[i] = unichr(int(item[:2], 16)) + item[2:]
		except KeyError:
			res[i] = '%' + item
		except ValueError:
			res[i] = '%' + item

	return "".join(res)

def get_cache_con():
	if not cache_con_map.has_key(threading._get_ident()):
		cache_con = sqlite.connect(":memory:")
		cur = cache_con.cursor()
		cur.execute("create table plugin_result_cache(url string, timestamp long, content string)")
		cur.close()
		cache_con_map[threading._get_ident()] = cache_con
	con = cache_con_map[threading._get_ident()]
	return con

def inject_ubuntu_descriptions_in_upstream_result (resxml, ubuntu_descriptions, mimetype):
	result_list = None

	node = resxml.documentElement.firstChild
	seq_r = None
	while not node is None:
		nextNode = node.nextSibling
		if node.nodeType == node.ELEMENT_NODE:
			about = node.getAttribute("about")
			if not about is None and about.rfind("urn:mozilla:plugin-results:") == 0:
				item = node.getElementsByTagName("RDF:Seq").item(0)
				seq_r = item
				result_list = node
				break
		node = nextNode

	if not result_list:
		result_list = resxml.createElement("RDF:Description")

	if seq_r is None:
		result_list.setAttribute("about", "urn:mozilla:plugin-results:"+mimetype)
		pluginsElement = resxml.createElement("pfs:plugins")
		result_list.appendChild(pluginsElement)
		seq_r = resxml.createElement("RDF:Seq")
		pluginsElement.appendChild(seq_r)
		resxml.documentElement.appendChild(result_list)

	for description in ubuntu_descriptions:
		result_list.setAttribute("about", "urn:mozilla:plugin-results:"+description.requestedMimetype)
		update_list = resxml.createElement("RDF:Description")
		update_list.setAttribute("about", "urn:mozilla:plugin:"+ description.guid)
		resxml.documentElement.appendChild(update_list)
		updatesElement = resxml.createElement("pfs:updates")
		update_list.appendChild(updatesElement)
		seq_u = resxml.createElement("RDF:Seq")
		updatesElement.appendChild(seq_u)

		main_element = description.to_element(resxml)
		resxml.documentElement.appendChild(main_element);
		li_r = resxml.createElement("RDF:li")
		li_u = resxml.createElement("RDF:li")
		li_r.setAttribute("resource", "urn:mozilla:plugin:" + description.guid)
		li_u.setAttribute("resource", description.id)
		seq_r.appendChild(li_r)
		seq_u.appendChild(li_u)

	node = seq_r.lastChild
	while not node is None:
		nextNode = node.previousSibling
		if node.nodeType == node.ELEMENT_NODE and node.getAttribute("resource") == "urn:mozilla:plugin:-1":
			seq_r.removeChild(node)
		
		node = nextNode



def get_cached_and_cache(req, http_params):
	global counter
	counter= (counter + 1) % 2

	if counter == 0:
		now = int(time.time() * 1000)
		low_time = now - CACHE_TIME_SEC * 1000
		cur = get_cache_con().cursor()
		cur.execute("delete from plugin_result_cache where timestamp <= ?", (low_time,))
		cur.close()

	key = req.args
	found = False
	result = None
	cur = get_cache_con().cursor()
	cur.execute("select timestamp, content from plugin_result_cache where url = ?", (key,))
	for row in cur:
		found = True
		result = row[1]

	cur.close()

	if found:
		print "found!"
		return result

	resxml = retrieve_proxied_result(req)
	ubuntu_descriptions = retrieve_ubuntu_results(req, http_params)

	inject_ubuntu_descriptions_in_upstream_result(resxml, ubuntu_descriptions, http_params['mimetype'])

	result_string = resxml.toxml()

	now = int(time.time() * 1000)
	cur = get_cache_con().cursor()
	cur.execute("insert into plugin_result_cache (timestamp, url, content) values (?, ?, ?)", (now, key, result_string))
	return result_string


apt_con_lock = thread.allocate_lock()
apt_plugin_db_updater=None
error_txt=""

def handler(req):
	global apt_plugin_db_updater
	pfsdbdir =  req.document_root() + "/pfsdb/"
	req.content_type = 'text/xml'
	http_params = parse_http_parameters(req)
	res = get_cached_and_cache(req, http_params)
	req.write(res)
	return apache.OK

