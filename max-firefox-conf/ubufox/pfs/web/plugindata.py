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


from xml.dom.xmlbuilder import *
import uuid

class MimeTypeDescription:

	def __init__ (mimetype):
		self.mimetype = mimetype
		self.pluginResults = None


class PluginDescription:

	def __init__ (  self, \
			name, \
			requestedMimetype, \
			guid, \
			version, \
			IconUrl, \
			XPILocation, \
			InstallerShowsUI, \
			manualInstallationURL, \
			licenseURL,
			needsRestart ):

		self.guid = "{" + str(uuid.uuid1()) + "}"
		self.id="urn:mozilla:plugin:" + self.guid  + ":"
		self.name = name
		self.requestedMimetype = requestedMimetype
		self.version = version
		self.IconUrl = IconUrl
		self.XPILocation = XPILocation
		self.InstallerShowsUI = InstallerShowsUI
		self.manualInstallationURL = manualInstallationURL
		self.licenseURL = licenseURL
		self.needsRestart = needsRestart

	def to_element (self, doc):
		root_elem = doc.createElement( \
					"RDF:Description")
		root_elem.setAttribute("about", self.id)

		name_elem = doc.createElement( \
					"pfs:name")
		root_elem.appendChild(name_elem)
		name_text = doc.createTextNode(self.name)
		name_elem.appendChild(name_text)

		requestedMimetype_elem = doc.createElement( \
					"pfs:requestedMimetype")
		root_elem.appendChild(requestedMimetype_elem)

		if not self.requestedMimetype is None:
			requestedMimetype_text = doc.createTextNode(self.requestedMimetype)
			requestedMimetype_elem.appendChild(requestedMimetype_text)

		guid_elem = doc.createElement( \
					"pfs:guid")

		root_elem.appendChild(guid_elem)
		if not self.guid is None:
			guid_text = doc.createTextNode(self.guid)
			guid_elem.appendChild(guid_text)

		version_elem = doc.createElement( \
					"pfs:version")
		root_elem.appendChild(version_elem)
		if not self.version is None:
#			version_text = doc.createTextNode(self.version)
			version_elem.appendChild(version_text)

		IconUrl_elem = doc.createElement( \
					"pfs:IconUrl")
		root_elem.appendChild(IconUrl_elem)
		if not self.IconUrl is None:
			IconUrl_text = doc.createTextNode(self.IconUrl)
			IconUrl_elem.appendChild(IconUrl_text)

		XPILocation_elem = doc.createElement( \
					"pfs:XPILocation")
		root_elem.appendChild(XPILocation_elem)
		if not self.XPILocation is None:
			XPILocation_text = doc.createTextNode(self.XPILocation)
			XPILocation_elem.appendChild(XPILocation_text)

		manualInstallationURL_elem = doc.createElement( \
					"pfs:manualInstallationURL")
		root_elem.appendChild(manualInstallationURL_elem)
		if not self.manualInstallationURL is None:
			manualInstallationURL_text = doc.createTextNode(self.manualInstallationURL)
			manualInstallationURL_elem.appendChild(manualInstallationURL_text)

		licenseURL_elem = doc.createElement( \
					"pfs:licenseURL")
		root_elem.appendChild(licenseURL_elem)
		if not self.licenseURL is None:
			licenseURL_text = doc.createTextNode(self.licenseURL)
			licenseURL_elem.appendChild(licenseURL_text)

		needsRestart_elem = doc.createElement( \
					"pfs:needsRestart")
		root_elem.appendChild(needsRestart_elem)
		if not self.needsRestart is None:
			needsRestart_text = doc.createTextNode(self.needsRestart)
			needsRestart_elem.appendChild(needsRestart_text)

		return root_elem

