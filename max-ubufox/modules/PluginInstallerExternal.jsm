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
 * Portions created by the IBM Corporation are Copyright (C) 2004
 * IBM Corporation. All Rights Reserved.
 *
 * Contributor(s):
 *   Doron Rosenberg <doronr@us.ibm.com>
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
const Ci = Components.interfaces;
const Cr = Components.results;

const nsIFile               = Ci.nsIFile;
const nsIDownloader         = Ci.nsIDownloader;
const nsISupports           = Ci.nsISupports;
const nsIInterfaceRequestor = Ci.nsIInterfaceRequestor;
const nsIDownloadObserver   = Ci.nsIDownloadObserver;
const nsIProgressEventSink  = Ci.nsIProgressEventSink;
const nsIFileInputStream    = Ci.nsIFileInputStream;
const nsICryptoHash         = Ci.nsICryptoHash;
const nsIProcess            = Ci.nsIProcess;

var EXPORTED_SYMBOLS = [ ];

Cu.import("resource://ubufox/PluginFinder.jsm");
Cu.import("resource://gre/modules/AddonManager.jsm");
Cu.import("resource://gre/modules/Services.jsm");

["LOG", "WARN", "ERROR"].forEach(function(aName) {
  this.__defineGetter__(aName, function() {
    Components.utils.import("resource://gre/modules/AddonLogging.jsm");

    LogManager.getLogger("ubufox.plugininstaller.external", this);
    return this[aName];
  });
}, this);



function binaryToHex(input) {
  return [('0' + input.charCodeAt(i).toString(16)).slice(-2)
          for (i in input)].join('');
}

function verifyHash(aFile, aHash) {
  try {
    var [, method, hash] = /^([A-Za-z0-9]+):(.*)$/.exec(aHash);

    var fis = Cc['@mozilla.org/network/file-input-stream;1'].
      createInstance(nsIFileInputStream);
    fis.init(aFile, -1, -1, 0);

    var hasher = Cc['@mozilla.org/security/hash;1'].createInstance(nsICryptoHash);
    hasher.initWithString(method);
    hasher.updateFromStream(fis, -1);
    dlhash = binaryToHex(hasher.finish(false));
    return dlhash == hash;
  }
  catch (e) {
    Cu.reportError(e);
    return false;
  }
}

var ExternalInstaller = {
  install: function EI_install(aPluginInfo, aListener) {
    LOG("Starting external install for " + aPluginInfo.location);
    let uri = Services.io.newURI(aPluginInfo.location, null, null);

    let resultfile = Services.dirsvc.get("TmpD", nsIFile);
    resultfile.append(uri.fileName);
    resultfile.createUnique(nsIFile.NORMAL_FILE_TYPE, 0750);

    let downloader = Cc["@mozilla.org/network/downloader;1"]
                        .createInstance(nsIDownloader);
    downloader.init({
      onDownloadComplete: function(aDownloader, aRequest, aCtxt,
                                   aStatus, aResult) {
        ExternalInstaller.onDownloadComplete(aDownloader, aRequest, aCtxt,
                                             aStatus, aResult, aListener);
      }
    }, resultfile);

    let channel = Services.io.newChannelFromURI(uri);
    channel.notificationCallbacks = {
      onProgress: function (aRequest, aContext, aProgress, aProgressMax) {
        aListener.onProgressChanged((aProgress / aProgressMax) * 100);
      },

      onStatus: function (aRequest, aContext, aStatus, aStatusArg) { },

      QueryInterface: function(aIID) {
        if (aIID.equals(nsISupports) || aIID.equals(nsIInterfaceRequestor) ||
            aIID.equals(nsIDownloadObserver) || aIID.equals(nsIProgressEventSink)) {
          return this;
        }

        throw Cr.NS_ERROR_NO_INTERFACE;
      },

      getInterface: function(aIID)
      {
        if (aIID.equals(nsIProgressEventSink)) {
          return this;
        }

        return null;
      }
    };

    channel.asyncOpen(downloader, null);
  },

  onDownloadComplete: function EI_onDownloadComplete(aDownloader, aRequest, aCtxt,
                                                     aStatus, aResult, aListener) {
    if (!Components.isSuccessCode(aStatus)) {
      aListener.onInstallFailed(PluginInstallerExternal.xpinstallbundle
                                .GetStringFromName("error-228"));
      aResult.remove(false);
      return;
    }

    if (aPluginInfo.hash && !verifyHash(aResult, aPluginInfo.hash)) {
      aListener.onInstallFailed(PluginInstallerExternal.xpinstallbundle
                                .GetStringFromName("error-261"));
      aResult.remove(false);
      return;
    }

    aResult.permissions = 0750;

    var process = Cc["@mozilla.org/process/util;1"].createInstance(nsIProcess);
    process.init(aResult);

    process.runAsync([], 0, {
      observe: function(aSubject, aTopic, aData) {
        if (aTopic != "process-finished") {
          ERROR("Failed to launch installer");
          aListener.onInstallFailed(PluginInstallerExternal.xpinstallbundle
                                    .GetStringFromName("error-207"));
        } else if (process.exitValue != 0) {
          ERROR("Installer returned with exit code " + process.exitValue);
          aListener.onInstallFailed(PluginInstallerExternal.xpinstallbundle
                                    .GetStringFromName("error-203"));
        } else {
          aListener.onInstallFinished();
        }

        aResult.remove(false);
      }
    });
  }
};

var XPIInstaller = {
  install: function XI_install(aPluginInfo, aListener) {
    LOG("Starting XPI install for " + aPluginInfo.location);
    AddonManager.getInstallForURL(aPluginInfo.location, function(aInstall) {
      aInstall.addListener({
        onNewInstall: function(aInstall) { },

        onDownloadStarted: function(aInstall) {
          aListener.onInstallStarted();
        },

        onDownloadProgress: function(aInstall) {
          aListener.onProgressChanged((aInstall.progress /
                                       aInstall.maxProgress) * 100);
        },

        onDownloadEnded: function(aInstall) { },

        onDownloadCancelled: function(aInstall) { },

        onDownloadFailed: function(aInstall) {
          let error = null;
          switch (aInstall.error) {
          case AddonManager.ERROR_NETWORK_FAILURE:
            error = PluginInstallerExternal.xpinstallbundle
                                           .GetStringFromName("error-228");
            break;
          case AddonManager.ERROR_INCORRECT_HASH:
            error = PluginInstallerExternal.xpinstallbundle
                                           .GetStringFromName("error-261");
            break;
          case AddonManager.ERROR_CORRUPT_FILE:
            error = PluginInstallerExternal.xpinstallbundle
                                           .GetStringFromName("error-207");
            break;
          }

          aListener.onInstallFailed(error);
        },

        onInstallStarted: function(aInstall) { },

        onInstallEnded: function(aInstall) {
          aListener.onInstallFinished();
        },

        onInstallCancelled: function(aInstall) { },

        onInstallFailed: function(aInstall) {
          aListener.onInstallFailed(PluginInstallerExternal
                                    .xpinstallbundle
                                    .GetStringFromName("error-203"));
        },

        onExternalInstall: function(aInstall, aExistingAddon, aNeedsRestart) { }
      });
      aInstall.install();
    }, "application/x-xpinstall", aPluginInfo.hash);
  }
};

var PluginInstallerExternal = {
  install: function PIE_install(aPluginInfo, aListener) {
    if (aPluginInfo.type == "xpi") {
      XPIInstaller.install(aPluginInfo, aListener);
    } else if (aPluginInfo.type == "external") {
      ExternalInstaller.install(aPluginInfo, aListener);
    } else {
      throw new Error("Cannot handle plugin with install type "
                      + aPluginInfo.type);
    }
  },

  get xpinstallbundle() {
    if (!this._xpinstallbundle) {
      this._xpinstallbundle = Services.strings.createBundle("chrome://global/locale/xpinstall/xpinstall.properties");
    }

    return this._xpinstallbundle;
  }
};

var PluginInstallerIface = {
  types: "external,xpi",

  install: function PII_install(aPluginInfo, aListener) {
    return PluginInstallerExternal.install(aPluginInfo, aListener);
  },

  shutdown: function PII_shutdown() {
    PluginFinder.unregisterInstallHandler(this);
  }
};

PluginFinder.registerInstallHandler(PluginInstallerIface);
