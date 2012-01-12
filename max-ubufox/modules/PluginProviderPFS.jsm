/* -*- Mode: javascript; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Plugin Finder Service.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation.
 * Portions created by the IBM Corporation are Copyright (C) 2004-2005
 * IBM Corporation. All Rights Reserved.
 *
 * Contributor(s):
 *   Doron Rosenberg <doronr@us.ibm.com>
 *   Alexander Sack <asac@jwsdot.com> - Canonical Ltd.
 *   Chris Coulson <chris.coulson@canonical.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

const Cu = Components.utils;
const Cc = Components.classes;
const Ci = Components.interfaces;

const nsIXULAppInfo           = Ci.nsIXULAppInfo;
const nsIHttpProtocolHandler  = Ci.nsIHttpProtocolHandler;
const nsIXULChromeRegistry    = Ci.nsIXULChromeRegistry;
const nsIRDFService           = Ci.nsIRDFService;
const nsIRDFRemoteDataSource  = Ci.nsIRDFRemoteDataSource;
const nsIRDFXMLSink           = Ci.nsIRDFXMLSink;
const nsIRDFDataSource        = Ci.nsIRDFDataSource;
const nsIRDFContainer         = Ci.nsIRDFContainer;
const nsIRDFResource          = Ci.nsIRDFResource;
const nsIRDFLiteral           = Ci.nsIRDFLiteral;

const PFS_NS = "http://www.mozilla.org/2004/pfs-rdf#";

var EXPORTED_SYMBOLS = [ ];

Cu.import("resource://ubufox/PluginFinder.jsm");
Cu.import("resource://gre/modules/Services.jsm");

["LOG", "WARN", "ERROR"].forEach(function(aName) {
  this.__defineGetter__(aName, function() {
    Components.utils.import("resource://gre/modules/AddonLogging.jsm");

    LogManager.getLogger("ubufox.pluginprovider.pfs", this);
    return this[aName];
  });
}, this);

function getPluginInfoRequestObserver(aMimeType, aListener) {
  this.mimetype = aMimeType;
  this.listener = aListener;
}

getPluginInfoRequestObserver.prototype = {
  onBeginLoad: function(aSink) {},
  onInterrupt: function(aSink) {},
  onResume: function(aSink) {},
  onEndLoad: function(aSink) {
    aSink.removeXMLSinkObserver(this);
    
    let ds = aSink.QueryInterface(nsIRDFDataSource);
    PluginProviderPFS.onDataSourceLoaded(ds, this.mimetype, this.listener);
  },
  
  onError: function(aSink, aStatus, aErrorMsg) {  
    aSink.removeXMLSinkObserver(this);   
    PluginProviderPFS.onDataSourceError(aStatus.toString(), this.mimetype, this.listener);
  }
};

var PluginProviderPFS = {
  get appInfo() {
    if (!this._appInfo) {
      this._appInfo = Cc["@mozilla.org/xre/app-info;1"].getService(nsIXULAppInfo);
    }

    return this._appInfo;
  },

  get pfsURI() {
    return Services.prefs.getCharPref("pfs.datasource.url");
    //return "http://localhost/~chr1s/cgi-bin/plugin-finder.py?mimetype=%PLUGIN_MIMETYPE%&appID=%APP_ID%&appVersion=%APP_VERSION%&clientOS=%CLIENT_OS%&chromeLocale=%CHROME_LOCALE%&distributionID=%DIST_ID%";
  },

  get appID() {
    if (!this._appID) {
      this._appID = this.appInfo.ID;
    }

    return this._appID;
  },

  get buildID() {
    if (!this._buildID) {
      this._buildID = this.appInfo.platformBuildID;
    }

    return this._buildID;
  },

  get appRelease() {
    if (!this._appRelease) {
      this._appRelease = this.appInfo.version;
    }

    return this._appRelease;
  },

  get clientOS() {
    if (!this._clientOS) {
      this._clientOS = Cc["@mozilla.org/network/protocol;1?name=http"]
                       .getService(nsIHttpProtocolHandler).oscpu;
    }

    return this._clientOS;
  },

  get chromeLocale() {
    if (!this._chromeLocale) {
      this._chromeLocale = Cc["@mozilla.org/chrome/chrome-registry;1"]
                           .getService(nsIXULChromeRegistry)
                           .getSelectedLocale("global");
    }

    return this._chromeLocale;
  },

  get distID() {
    if (!this._distID) {
      this._distID = Services.prefs.getCharPref("extensions.ubufox.release");
    }

    return this._distID;
  },

  get rdfService() {
    if (!this._rdfService) {
      this._rdfService = Cc["@mozilla.org/rdf/rdf-service;1"].getService(nsIRDFService);
    }

    return this._rdfService;
  },

  getPluginInfo: function PPPFS_getPluginInfo(aMimeType, aListener) {
    LOG("Handling getPluginInfo for " + aMimeType);
    let uri = this.pfsURI;

    uri = uri.replace(/%PLUGIN_MIMETYPE%/g, encodeURIComponent(aMimeType));
    uri = uri.replace(/%APP_ID%/g, this.appID);
    uri = uri.replace(/%APP_VERSION%/g, this.buildID);
    uri = uri.replace(/%APP_RELEASE%/g, this.appRelease);
    uri = uri.replace(/%CLIENT_OS%/g, this.clientOS);
    uri = uri.replace(/%CHROME_LOCALE%/g, this.chromeLocale);
    uri = uri.replace(/%DIST_ID%/g, this.distID);

    LOG("uri = " + uri);
    let ds = this.rdfService.GetDataSource(uri);
    let rds = ds.QueryInterface(nsIRDFRemoteDataSource);
    if (rds.loaded) {
      this.onDataSourceLoaded(ds, aMimeType, aListener);
    } else {
      let sink = ds.QueryInterface(nsIRDFXMLSink);
      sink.addXMLSinkObserver(new getPluginInfoRequestObserver(aMimeType, aListener));
    }
  },

  onDataSourceLoaded: function PPPFS_onDataSourceLoaded(aDataSource, aMimeType, aListener) {
    let container = Cc["@mozilla.org/rdf/container;1"].createInstance(nsIRDFContainer);
    let resultRes = this.rdfService.GetResource("urn:mozilla:plugin-results:" + aMimeType);
    let pluginList = aDataSource.GetTarget(resultRes, this.rdfService.GetResource(PFS_NS+"plugins"), true);
    var pluginInfos = [];

    try {
      container.Init(aDataSource, pluginList);
      let children = container.GetElements();

      // get the first item
      while(children.hasMoreElements()) {

      	let target;
	      let child = children.getNext();
      	if (child instanceof nsIRDFResource) {
      	  let name = this.rdfService.GetResource("http://www.mozilla.org/2004/pfs-rdf#updates");
      	  target = aDataSource.GetTarget(child, name, true);
      	}

      	try {
      	  container.Init(aDataSource, target);
      	  var target2 = null;
      	  let children2 = container.GetElements();

      	  while (children2.hasMoreElements()) {
      	    let child2 = children2.getNext();
	          if (child2 instanceof nsIRDFResource) {
      	      target2 = child2;
	          }

	          var rdfs = this.rdfService;

	          function getPFSValueFromRDF(aValue) {
	            var rv = null;

      	      var myTarget = aDataSource.GetTarget(target2, rdfs.GetResource(PFS_NS + aValue), true);
      	      if (myTarget) {
		            rv = myTarget.QueryInterface(nsIRDFLiteral).Value;
              }

        	    return rv;
	          }

            function getPFSBoolFromRDF(aValue) {
              return getPFSValueFromRDF(aValue) == "true" ? true : false
            }

            let pid = getPFSValueFromRDF("guid");
            if (pid == -1) {
              continue;
            }

      	    let pluginInfo = {
              name: getPFSValueFromRDF("name"),
	            pid: pid,
	            version: getPFSValueFromRDF("version"),
	            IconUrl: getPFSValueFromRDF("IconUrl"),
	            desc: getPFSValueFromRDF("description"),
	            homepage: getPFSValueFromRDF("homepage"),
	            licenseURL: getPFSValueFromRDF("licenseURL"),
	            needsRestart: getPFSBoolFromRDF("needsRestart"),
              requestedMimeType: aMimeType
            };

            let XPILocation = getPFSValueFromRDF("XPILocation");
            let installerLocation = getPFSValueFromRDF("InstallerLocation");
            let manualInstallationURL = getPFSValueFromRDF("manualInstallationURL");

            if (installerLocation) {
              pluginInfo.location = installerLocation;
              pluginInfo.hash = getPFSValueFromRDF("InstallerHash");
              pluginInfo.type = "external";
            } else if (XPILocation) {
              pluginInfo.location = XPILocation;
              let uri = Services.io.newURI(XPILocation, null, null);
              if (uri.scheme = "apt") {
                pluginInfo.type = "apt";
              } else {
                pluginInfo.type = "xpi";
                pluginInfo.hash = getPFSValueFromRDF("XPIHash");
              }
            } else if (manualInstallationURL) {
              pluginInfo.location = manualInstallationURL;
              pluginInfo.type = "manual";
            } else {
              WARN("Plugin info contains no source information");
              continue;
            }

            // no license provided, make it accepted
            pluginInfo.licenseAccepted = pluginInfo.licenseURL ? false : true;

            pluginInfos.push(pluginInfo);
          }
      	}	catch (ex) {
          Cu.reportError(ex);
        }
      }
    } catch (ex) {
      Cu.reportError(ex);
    }

    aListener.pluginInfoReceived(pluginInfos);
  },

  onDataSourceError: function PPPFS_onDatasourceError(aError, aMimeType, aListener) {
    ERROR(aError + " whilst handling request for mimetype " + aMimeType);
    aListener.providerErrorReceived(aError);
  }
};

var PluginProviderIface = {
  getPluginInfo: function PFI_getPluginInfo(aMimeType, aListener) {
    PluginProviderPFS.getPluginInfo(aMimeType, aListener);
  },

  shutdown: function PFI_shutdown() {
    PluginFinder.unregisterProvider(this);
  }
};

PluginFinder.registerProvider(PluginProviderIface);
