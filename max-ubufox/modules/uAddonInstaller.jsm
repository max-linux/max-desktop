/*
# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is the Extension Manager.
#
# The Initial Developer of the Original Code is
# the Mozilla Foundation.
# Portions created by the Initial Developer are Copyright (C) 2009
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Dave Townsend <dtownsend@oxymoronical.com>
#   Chris Coulson <chris.coulson@canonical.com>
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****
*/

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cu = Components.utils;
const Cr = Components.results;

const nsILocalFile            = Ci.nsILocalFile;
const nsIFile                 = Ci.nsIFile;
const nsIRDFService           = Ci.nsIRDFService;
const nsIRDFLiteral           = Ci.nsIRDFLiteral;
const nsIRDFResource          = Ci.nsIRDFResource;
const nsIRDFInt               = Ci.nsIRDFInt;
const nsIRDFXMLParser         = Ci.nsIRDFXMLParser;
const nsIRDFDataSource        = Ci.nsIRDFDataSource;
const nsIInputStreamChannel   = Ci.nsIInputStreamChannel;
const nsIChannel              = Ci.nsIChannel;
const nsIFileInputStream      = Ci.nsIFileInputStream;
const nsIBufferedInputStream  = Ci.nsIBufferedInputStream;
const nsIZipReader            = Ci.nsIZipReader;

Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/AddonManager.jsm");
Cu.import("resource://gre/modules/NetUtil.jsm");

const PREF_AI_LAST_DIR_MTIME        = "extensions.ubufox@ubuntu.com.addonInstallerDirLastModifiedTime";
const PREF_AI_INSTALLED_ADDONS      = "extensions.ubufox@ubuntu.com.installedAddons.";

const FILE_INSTALL_MANIFEST         = "install.rdf";

const PREFIX_NS_EM                  = "http://www.mozilla.org/2004/em-rdf#";
const RDFURI_INSTALL_MANIFEST_ROOT  = "urn:mozilla:install-manifest";

const REGEXP_XPI_FILE = /^.+\.xpi$/;
const REGEXP_VALID_ID = /^(\{[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\}|[a-z0-9-\._]*\@[a-z0-9-\._]+)$/;

const PROP_METADATA = [ "id", "version" ];

var EXPORTED_SYMBOLS = [];

/**
 * A helpful wrapper around the prefs service that allows for default values
 * when requested values aren't set.
 */
var Prefs = {
  getCharPref: function(aName, aDefaultValue) {
    try {
      return Services.prefs.getCharPref(aName);
    }
    catch (e) {
    }
    return aDefaultValue;
  },

  getBoolPref: function(aName, aDefaultValue) {
    try {
      return Services.prefs.getBoolPref(aName);
    }
    catch (e) {
    }
    return aDefaultValue;
  }
}

function AddonInternal() {
}

this.__defineGetter__("gRDF", function() {
  delete this.gRDF;
  return this.gRDF = Cc["@mozilla.org/rdf/rdf-service;1"].
                     getService(nsIRDFService);
});

function EM_R(aProperty) {
  return gRDF.GetResource(PREFIX_NS_EM + aProperty);
}

function getRDFValue(aLiteral) {
  if (aLiteral instanceof nsIRDFLiteral)
    return aLiteral.Value;
  if (aLiteral instanceof nsIRDFResource)
    return aLiteral.Value;
  if (aLiteral instanceof nsIRDFInt)
    return aLiteral.Value;
  return null;
}

function getRDFProperty(aDs, aResource, aProperty) {
  return getRDFValue(aDs.GetTarget(aResource, EM_R(aProperty), true));
}

function loadManifestFromRDF(aUri, aStream) {
  let rdfParser = Cc["@mozilla.org/rdf/xml-parser;1"].
                  createInstance(nsIRDFXMLParser)
  let ds = Cc["@mozilla.org/rdf/datasource;1?name=in-memory-datasource"].
           createInstance(nsIRDFDataSource);
  let listener = rdfParser.parseAsync(ds, aUri);
  let channel = Cc["@mozilla.org/network/input-stream-channel;1"].
                createInstance(nsIInputStreamChannel);
  channel.setURI(aUri);
  channel.contentStream = aStream;
  channel.QueryInterface(nsIChannel);
  channel.contentType = "text/xml";

  listener.onStartRequest(channel, null);

  try {
    let pos = 0;
    let count = aStream.available();
    while (count > 0) {
      listener.onDataAvailable(channel, null, aStream, pos, count);
      pos += count;
      count = aStream.available();
    }
    listener.onStopRequest(channel, null, Cr.NS_OK);
  }
  catch (e) {
    listener.onStopRequest(channel, null, e.result);
    throw e;
  }

  let root = gRDF.GetResource(RDFURI_INSTALL_MANIFEST_ROOT);
  let addon = new AddonInternal();
  PROP_METADATA.forEach(function(aProp) {
    addon[aProp] = getRDFProperty(ds, root, aProp);
  });

  return addon;
}

function loadManifestFromDir(aDir) {
  let file = aDir.clone();
  file.append(FILE_INSTALL_MANIFEST);
  if (!file.exists() || !file.isFile())
    throw new Error("Directory " + aDir.path + " does not contain a valid " +
                    "install manifest");

  let fis = Cc["@mozilla.org/network/file-input-stream;1"].
            createInstance(nsIFileInputStream);
  fis.init(file, -1, -1, false);
  let bis = Cc["@mozilla.org/network/buffered-input-stream;1"].
            createInstance(nsIBufferedInputStream);
  bis.init(fis, 4096);

  try {
    let addon = loadManifestFromRDF(Services.io.newFileURI(file), bis);
    return addon;
  }
  finally {
    bis.close();
    fis.close();
  }
}

function loadManifestFromZipReader(aZipReader) {
  let zis = aZipReader.getInputStream(FILE_INSTALL_MANIFEST);
  let bis = Cc["@mozilla.org/network/buffered-input-stream;1"].
            createInstance(nsIBufferedInputStream);
  bis.init(zis, 4096);

  try {
    let uri = buildJarURI(aZipReader.file, FILE_INSTALL_MANIFEST);
    let addon = loadManifestFromRDF(uri, bis);

    return addon;
  }
  finally {
    bis.close();
    zis.close();
  }
}

function loadManifestFromZipFile(aXPIFile) {
  let zipReader = Cc["@mozilla.org/libjar/zip-reader;1"].
                  createInstance(nsIZipReader);
  try {
    zipReader.open(aXPIFile);

    return loadManifestFromZipReader(zipReader);
  }
  finally {
    zipReader.close();
  }
}

function loadManifestFromFile(aFile) {
  if (aFile.isFile())
    return loadManifestFromZipFile(aFile);
  else
    return loadManifestFromDir(aFile);
}

function buildJarURI(aJarfile, aPath) {
  let uri = Services.io.newFileURI(aJarfile);
  uri = "jar:" + uri.spec + "!/" + aPath;
  return NetUtil.newURI(uri);
}

var uAddonInstaller = {
  queue: [],
  inProgress: false,

  startup: function() {
    var addonDir = Cc["@mozilla.org/file/local;1"].createInstance(nsILocalFile);
    addonDir.initWithPath("/usr/share/ubufox/extensions");
    if (addonDir.exists() && addonDir.isDirectory() &&
       (parseInt(Prefs.getCharPref(PREF_AI_LAST_DIR_MTIME, "0")) != addonDir.lastModifiedTime)) {
      this.installExtraExtensionsFromDir(addonDir);
      Services.prefs.setCharPref(PREF_AI_LAST_DIR_MTIME, (addonDir.lastModifiedTime).toString());
    }
  },

  installExtraExtensionsFromDir: function(aDir) {
    var entries = aDir.directoryEntries;
    while (entries.hasMoreElements())
      this.queue.push(entries.getNext().QueryInterface(nsIFile));

    if (!this.inProgress)
      this.processNextFile();
  },

  processNextFile: function() {
    if (this.queue.length == 0) {
      this.inProgress = false;
      return;
    }

    this.inProgress = true;

    var entry = this.queue.shift();

    let id = entry.leafName;
    if (entry.isFile()) {
      if (REGEXP_XPI_FILE.test(id))
        id = id.substring(0, (id.length - 4));
    } else if (!entry.isDirectory()) {
      /* Not an xpi file or directory */
      return this.processNextFile();
    }

    if (!REGEXP_VALID_ID.test(id))
      return this.processNextFile();

    var addon;
    try {
      addon = loadManifestFromFile(entry);
    } catch(e) {
      return this.processNextFile();
    }

    if (addon.id != id)
      return this.processNextFile();

    var self = this;
    AddonManager.getAddonByID(id, function(aAddon) {
      let shouldInstall = false;
      if (aAddon) {
        if ((Services.vc.compare(aAddon.version, addon.version) < 0) ||
            (!aAddon.permissions & AddonManager.PERM_CAN_UNINSTALL)) {
          shouldInstall = true;
        }
      } else if (!Prefs.getBoolPref(PREF_AI_INSTALLED_ADDONS + id, false)) {
        shouldInstall = true;
      }

      if (shouldInstall == true) {
        AddonManager.getInstallForFile(entry, function(aInstall) {
          if (aInstall) {
            aInstall.addListener(self);
            aInstall.install();
            Services.prefs.setBoolPref(PREF_AI_INSTALLED_ADDONS + id, true);
          }
          self.processNextFile();
        });
      } else {
        self.processNextFile();
      }
    });
  },

  onInstallEnded: function(aInstall, aAddon) {
    aInstall.removeListener(this);
    aAddon.findUpdates({
      onUpdateAvailable: function(aAddon, aInstall) {
        if (aInstall)
          aInstall.install();
      }
    }, AddonManager.UPDATE_WHEN_ADDON_INSTALLED);
  },

  onInstallCancelled: function(aInstall) {
    aInstall.removeListener(this);
  },

  onInstallFailed: function(aInstall) {
    aInstall.removeListener(this);
  }
};

uAddonInstaller.startup();
