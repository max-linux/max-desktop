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
const Ci = Components.interfaces;
const Cc = Components.classes;

const nsIXULAppInfo             = Ci.nsIXULAppInfo;
const nsIInterfaceRequestor     = Ci.nsIInterfaceRequestor;
const nsIWebProgress            = Ci.nsIWebProgress;
const nsIWebProgressListener    = Ci.nsIWebProgressListener;
const nsISupportsWeakReference  = Ci.nsISupportsWeakReference;
const nsISupports               = Ci.nsISupports;
const nsIWebNavigation          = Ci.nsIWebNavigation;
const nsIScriptSecurityManager  = Ci.nsIScriptSecurityManager;
const nsIHttpProtocolHandler    = Ci.nsIHttpProtocolHandler;
const nsIXULChromeRegistry      = Ci.nsIXULChromeRegistry;
const nsIObserverService        = Ci.nsIObserverService;
const nsISupportsPRBool         = Ci.nsISupportsPRBool;
const nsIAppStartup             = Ci.nsIAppStartup;
const nsIPluginHost             = Ci.nsIPluginHost;

Cu.import("resource://ubufox/PluginFinder.jsm");

function nsPluginInstallerWizard() {

  // create the request array
  this.mPluginRequestArray = new Object();
  // since the array is a hash, store the length
  this.mPluginRequestArrayLength = 0;

  // create the plugin info array.
  // a hash indexed by mimetype
  this.mPluginInfoArray = new Object();
  this.mPluginInfoArrayLength = 0;

  this.mMimeTypePluginSelections = new Object();

  // holds plugins we couldn't find
  this.mPluginNotFoundArray = new Object();
  this.mPluginNotFoundArrayLength = 0;

  // array holding pids of plugins that require a license
  this.mPluginLicenseArray = new Array();

  // how many plugins are to be installed
  this.pluginsToInstallNum = 0;

  this.mBrowser = null;
  this.mSuccessfullPluginInstallation = 0;
  this.mNeedsRestart = false;

  this.mPluginPidArray = new Object();
  // arguments[0] is an array that contains two items:
  //     an array of mimetypes that are missing
  //     a reference to the browser that needs them, 
  //        so we can notify which browser can be reloaded.

  if ("arguments" in window) {
    for (var item in window.arguments[0].plugins){
      this.mPluginRequestArray[window.arguments[0].plugins[item].mimetype] =
        new nsPluginRequest(window.arguments[0].plugins[item]);

      this.mPluginRequestArrayLength++;
    }

    this.mBrowser = window.arguments[0].browser;
  }

  this.WSPluginCounter = 0;
  this.licenseAcceptCounter = 0;
}

nsPluginInstallerWizard.prototype.getPluginData = function() {
  this.WSPluginCounter = 0;

  for (let mimetype in this.mPluginRequestArray) {
    PluginFinder.getPluginInfo(mimetype, function(aMimeType, aPluginInfos) {
      gPluginInstaller.pluginInfoReceived(aMimeType, aPluginInfos);
    });
  }
}

// aPluginInfo is null if the datasource call failed, and pid is -1 if
// no matching plugin was found.
nsPluginInstallerWizard.prototype.pluginInfoReceived = function(aMimeType,
                                                                aPluginInfos) {
  this.WSPluginCounter++;

  if (aPluginInfos ) {
    var noResultInfo = null;
    for each (let aPluginInfo in aPluginInfos) {
    	this.mPluginPidArray[aPluginInfo.pid] = aPluginInfo;
    }

    if(aPluginInfos.length > 0)
    {
      this.mPluginInfoArray[aMimeType] = aPluginInfos;
      this.mPluginInfoArrayLength++;
    } else {
      this.mPluginNotFoundArray[aMimeType] = true;
      this.mPluginNotFoundArrayLength++;
    }
  }

  var progressMeter = document.getElementById("ws_request_progress");

  if (progressMeter.getAttribute("mode") == "undetermined") {
    progressMeter.setAttribute("mode", "determined");
  }

  progressMeter.setAttribute("value",
      ((this.WSPluginCounter / this.mPluginRequestArrayLength) * 100) + "%");

  if (this.WSPluginCounter == this.mPluginRequestArrayLength) {
    // check if no plugins were found
    if (this.mPluginInfoArrayLength == 0) {
      this.advancePage("lastpage");
    } else {
      // we want to allow user to cancel
      this.advancePage(null);
    }
  }
}

nsPluginInstallerWizard.prototype.createPluginSetGroupBox = function(mimetype) {

  var stringBundle = document.getElementById("ubufoxPluginWizardString");
  var gbox = document.createElement("vbox");
  gbox.setAttribute("flex", "1");

  var caption = document.createElement("caption");
  caption.setAttribute("label", stringBundle.getString("pluginwizard.available_plugins.description.label")+" "+mimetype+":");
  gbox.appendChild(caption);

  return gbox;
}

function doToggleInstallPluginEvent(e) {
  gPluginInstaller.toggleInstallPlugin(e);
}

nsPluginInstallerWizard.prototype.showPluginList = function() {
  this.pluginsToInstallNum = 0;
  var hasPluginWithInstallerUI = false;

  var groupBox = null;
  var lastSibling = null;

  for (var mimetype in this.mPluginInfoArray){
    // [plugin image] [Plugin_Name Plugin_Version]
    var pluginInfoSet = this.mPluginInfoArray[mimetype];
    var firstPluginSet = (groupBox == null);
    groupBox = this.createPluginSetGroupBox(mimetype);

    if(firstPluginSet) {
      placeHolder = document.getElementById("pluginselection-placeholder");
      placeHolder.parentNode.replaceChild(groupBox, placeHolder);
    } else {
      lastSibling.parentNode.insertBefore(groupBox, lastSibling);
    }
    lastSibling = groupBox;

    var radiogroup = document.createElement("richlistbox");
    radiogroup.setAttribute("flex", "1");
    groupBox.appendChild(radiogroup);
    radiogroup.addEventListener("select", doToggleInstallPluginEvent, false);

    // OK, lets add the "None" option first
    var first = true;
    for each (let pluginInfo in pluginInfoSet) {
      var rli = document.createElement("richlistitem");
      rli.setAttribute("style", "border-bottom: dotted 1px lightgrey; padding: 0.5em 1em 0.5em 1em");
      rli._ubufoxPluginInfo = pluginInfo;
      rli._ubufoxPluginInfoMimeType = mimetype;
      radiogroup.appendChild(rli);

      var distributorImageWrap = document.createElement("vbox");

      var spacer = document.createElement("hbox");
      spacer.setAttribute("flex", "1");
      distributorImageWrap.appendChild(spacer);

      var distributorImage = document.createElement("image");
      distributorImageWrap.appendChild(distributorImage);
      distributorImageWrap.setAttribute("style", "vertical-align: middle");
      distributorImage.setAttribute("style", "vertical-align: middle");
      distributorImageWrap.setAttribute("flex", "0");
      distributorImage.setAttribute("class", "distributor-image");
      if (!pluginInfo.IconUrl || pluginInfo.IconUrl.length == 0) {
        if (pluginInfo.type == "apt") {
        	pluginInfo.IconUrl = "chrome://ubufox/content/ubuntulogo32.png";
        } else {
      	  pluginInfo.IconUrl = "chrome://ubufox/content/internet32.png";
        }
      }
      distributorImage.setAttribute("src", pluginInfo.IconUrl);

      spacer = document.createElement("hbox");
      spacer.setAttribute("flex", "1");
      distributorImageWrap.appendChild(spacer);

      rli.appendChild(distributorImageWrap);

      var nameAndDesc = document.createElement("vbox");
      rli.appendChild(nameAndDesc);

      var nameLabel = document.createElement("label");
      nameLabel.setAttribute("value", pluginInfo.name + " " + (pluginInfo.version ? pluginInfo.version : ""));
      nameLabel.setAttribute("style", "font-weight: bold");
      nameAndDesc.appendChild(nameLabel);

      var descDesc = document.createElement("description");
      if (!pluginInfo.desc) {
        pluginInfo.desc = this.getString("pluginwizard.description.notfound");
      }
      descDesc.appendChild(document.createTextNode(pluginInfo.desc));
      descDesc.setAttribute("style", "white-space: pre;");
      nameAndDesc.appendChild(descDesc);

      if (pluginInfo.InstallerShowsUI == "true") {
        hasPluginWithInstallerUI = true;
      }
    }

    radiogroup.selectedIndex = 0;

    // keep a running count of plugins the user wants to install
    this.pluginsToInstallNum++;
  }

  if (hasPluginWithInstallerUI) {
    document.getElementById("installerUI").hidden = false;
  }

  if (this.pluginsToInstallNum > 0) {
    this.canAdvance(true);
  } else {
    this.canAdvance(false);
  }
  this.canRewind(false);
}

nsPluginInstallerWizard.prototype.toggleInstallPlugin = function(e) {
  var selectedItem = e.target.selectedItem;
  var mime = selectedItem._ubufoxPluginInfoMimeType;
  this.mMimeTypePluginSelections[mime] = selectedItem._ubufoxPluginInfo.pid;

  this.pluginsToInstallNum = 0;
  for (var mime in this.mMimeTypePluginSelections) {
    if(this.mMimeTypePluginSelections[mime] && this.mMimeTypePluginSelections[mime] != "-1")
      this.pluginsToInstallNum++;
  }
 
  // if no plugins are checked, don't allow to advance
  if (this.pluginsToInstallNum > 0)
    this.canAdvance(true);
  else
    this.canAdvance(false);
}

nsPluginInstallerWizard.prototype.canAdvance = function(aBool) {
  document.getElementById("plugin-installer-wizard").canAdvance = aBool;
}

nsPluginInstallerWizard.prototype.canRewind = function(aBool) {
  document.getElementById("plugin-installer-wizard").canRewind = aBool;
}

nsPluginInstallerWizard.prototype.canCancel = function(aBool) {
  document.documentElement.getButton("cancel").disabled = !aBool;
}

nsPluginInstallerWizard.prototype.showLicenses = function() {
  this.canAdvance(false);
  this.canRewind(false);

  // only add if a license is provided and the plugin was selected to
  // be installed
  for (var mimetype in this.mMimeTypePluginSelections) {
    var pid = this.mMimeTypePluginSelections[mimetype];
    var pluginInfo = this.mPluginPidArray[pid];
    if (pluginInfo && pluginInfo.licenseURL && (pluginInfo.licenseURL != "")) {
      this.mPluginLicenseArray.push(pluginInfo.pid);
    }
  }

  if (this.mPluginLicenseArray.length == 0) {
    // no plugins require licenses
    this.advancePage(null);
  } else {
    this.licenseAcceptCounter = 0;

    // add a nsIWebProgress listener to the license iframe.
    var docShell = document.getElementById("licenseIFrame").docShell;
    var iiReq = docShell.QueryInterface(nsIInterfaceRequestor);
    var webProgress = iiReq.getInterface(nsIWebProgress);
    webProgress.addProgressListener(gPluginInstaller.progressListener,
                                    nsIWebProgress.NOTIFY_ALL);


    this.showLicense();
  }
}

nsPluginInstallerWizard.prototype.enableNext = function() {
  // if only one plugin exists, don't enable the next button until
  // the license is accepted
  if (gPluginInstaller.pluginsToInstallNum > 1)
    gPluginInstaller.canAdvance(true);

  document.getElementById("licenseRadioGroup1").disabled = false;
  document.getElementById("licenseRadioGroup2").disabled = false;
}

nsPluginInstallerWizard.prototype.progressListener = {
  onStateChange : function(aWebProgress, aRequest, aStateFlags, aStatus)
  {
    if ((aStateFlags & nsIWebProgressListener.STATE_STOP) &&
       (aStateFlags & nsIWebProgressListener.STATE_IS_NETWORK)) {
      // iframe loaded
      gPluginInstaller.enableNext();
    }
  },

  onProgressChange : function(aWebProgress, aRequest, aCurSelfProgress,
                              aMaxSelfProgress, aCurTotalProgress, aMaxTotalProgress)
  {},
  onStatusChange : function(aWebProgress, aRequest, aStatus, aMessage)
  {},

  QueryInterface : function(aIID)
  {
     if (aIID.equals(nsIWebProgressListener) ||
         aIID.equals(nsISupportsWeakReference) ||
         aIID.equals(nsISupports))
       return this;
     throw Components.results.NS_NOINTERFACE;
   }
}

nsPluginInstallerWizard.prototype.showLicense = function() {
  var pluginInfo = this.mPluginPidArray[this.mPluginLicenseArray[this.licenseAcceptCounter]];

  this.canAdvance(false);

  var loadFlags = nsIWebNavigation.LOAD_FLAGS_NONE;

  document.getElementById("licenseIFrame").webNavigation.loadURI(pluginInfo.licenseURL, loadFlags, null, null, null);

  document.getElementById("pluginLicenseLabel").firstChild.nodeValue = 
    this.getFormattedString("pluginLicenseAgreement.label", [pluginInfo.name]);

  document.getElementById("licenseRadioGroup1").disabled = true;
  document.getElementById("licenseRadioGroup2").disabled = true;
  document.getElementById("licenseRadioGroup").selectedIndex = 
    pluginInfo.licenseAccepted ? 0 : 1;
}

nsPluginInstallerWizard.prototype.showNextLicense = function() {
  var rv = true;

  if (this.mPluginLicenseArray.length > 0) {
    this.storeLicenseRadioGroup();

    this.licenseAcceptCounter++;

    if (this.licenseAcceptCounter < this.mPluginLicenseArray.length) {
      this.showLicense();

      rv = false;
      this.canRewind(true);
    }
  }

  return rv;
}

nsPluginInstallerWizard.prototype.showPreviousLicense = function() {
  this.storeLicenseRadioGroup();
  this.licenseAcceptCounter--;

  if (this.licenseAcceptCounter > 0)
    this.canRewind(true);
  else
    this.canRewind(false);

  this.showLicense();
  
  // don't allow to return from the license screens
  return false;
}

nsPluginInstallerWizard.prototype.storeLicenseRadioGroup = function() {
  var pluginInfo = this.mPluginPidArray[this.mPluginLicenseArray[this.licenseAcceptCounter]];
  pluginInfo.licenseAccepted = !document.getElementById("licenseRadioGroup").selectedIndex;
}

nsPluginInstallerWizard.prototype.licenseRadioGroupChange = function(aAccepted) {
  // only if one plugin is to be installed should selection change the next button
  if (this.pluginsToInstallNum == 1)
    this.canAdvance(aAccepted);
}

nsPluginInstallerWizard.prototype.advancePage = function(aPageId) {
  this.canAdvance(true);
  document.getElementById("plugin-installer-wizard").advance(aPageId);
}

nsPluginInstallerWizard.prototype.startPluginInstallation = function() {
  this.canAdvance(false);
  this.canRewind(false);

  let plugins = [];

  for each (let pid in this.mMimeTypePluginSelections) {
    if (this.mPluginPidArray[pid].type != "manual") {
      plugins.push(this.mPluginPidArray[pid]);
    }
  }

  if (plugins.length > 0) {
    document.getElementById("plugin_install_progress")
            .setAttribute("mode", "undetermined");
    PluginFinder.install(plugins, this);
  } else {
    this.advancePage(null);
  }
}

nsPluginInstallerWizard.prototype.onInstallStarted = function() {

}

nsPluginInstallerWizard.prototype.onProgressChanged = function(aProgress) {
  let elm = document.getElementById("plugin_install_progress");
  elm.setAttribute("mode", "determined");
  elm.setAttribute("value", Math.ceil(aProgress) + "%");
}

nsPluginInstallerWizard.prototype.onInstallFinished = function() {
  this.advancePage(null);
}

nsPluginInstallerWizard.prototype.onPluginFailed = function(aPid, aError) {
  this.mPluginPidArray[aPid].error = aError;
  this.mPluginPidArray[aPid].failed = true;
}

nsPluginInstallerWizard.prototype.onPluginFinished = function(aPid) {
  //this.mPluginPidArray[aPid].installed = true;
}

nsPluginInstallerWizard.prototype.addPluginResultRow = function(aImgSrc, aName, aNameTooltip, aStatus, aStatusTooltip, aManualUrl) {
  var myRows = document.getElementById("pluginResultList");

  var myRow = document.createElement("row");
  myRow.setAttribute("align", "center");

  // create the image
  var myImage = document.createElement("image");
  myImage.setAttribute("src", aImgSrc);
  myImage.setAttribute("height", "16px");
  myImage.setAttribute("width", "16px");
  myRow.appendChild(myImage)

  // create the labels
  var myLabel = document.createElement("label");
  myLabel.setAttribute("value", aName);
  if (aNameTooltip) {
    myLabel.setAttribute("tooltiptext", aNameTooltip);
  }
  myRow.appendChild(myLabel);

  if (aStatus) {
    myLabel = document.createElement("label");
    myLabel.setAttribute("value", aStatus);
    if (aStatusTooltip) {
      myLabel.setAttribute("tooltiptext", aStatusTooltip);
    }
    myRow.appendChild(myLabel);
  }

  // manual install
  if (aManualUrl) {
    var myButton = document.createElement("button");

    var manualInstallLabel = this.getString("pluginInstallationSummary.manualInstall.label");
    var manualInstallTooltip = this.getString("pluginInstallationSummary.manualInstall.tooltip");

    myButton.setAttribute("label", manualInstallLabel);
    myButton.setAttribute("tooltiptext", manualInstallTooltip);

    myRow.appendChild(myButton);

    // XXX: XUL sucks, need to add the listener after it got added into the document
    if (aManualUrl)
      myButton.addEventListener("command", function() { gPluginInstaller.loadURL(aManualUrl) }, false);
  }

  myRows.appendChild(myRow);
}

nsPluginInstallerWizard.prototype.showPluginResults = function() {
  var notInstalledList = "?action=missingplugins";
  var myRows = document.getElementById("pluginResultList");

  for (var mimetype in this.mMimeTypePluginSelections) {
    var myPluginItem = this.mPluginPidArray[this.mMimeTypePluginSelections[mimetype]];
    // [plugin image] [Plugin_Name Plugin_Version] [Success/Failed] [Manual Install (if Failed)]

    var statusMsg;
    var statusTooltip;
    if (myPluginItem.failed){
      statusMsg = this.getString("pluginInstallationSummary.failed");
      statusTooltip = myPluginItem.error;
      notInstalledList += "&mimetype=" + mimetype;
    } else if (myPluginItem.licenseURL && !myPluginItem.licenseAccepted) {
      statusMsg = this.getString("pluginInstallationSummary.licenseNotAccepted");
    } else {
      this.mSuccessfullPluginInstallation++;
      statusMsg = this.getString("pluginInstallationSummary.success");

      // only check needsRestart if the plugin was successfully installed.
      if (myPluginItem.needsRestart) {
      	this.mNeedsRestart = true;
      }
    }

    // manual url - either returned from the webservice or the pluginspage attribute
    var manualUrl;
    if (myPluginItem.error && this.mPluginRequestArray[mimetype].pluginsPage) {
      manualUrl = this.mPluginRequestArray[mimetype].pluginsPage;
    } else if (myPluginItem.type == "manual") {
      manualUrl = myPluginItem.location;
    }

    this.addPluginResultRow(
			    myPluginItem.IconUrl, 
			    myPluginItem.name + " " + (myPluginItem.version ? myPluginItem.version : ""),
			    null,
			    statusMsg, 
			    statusTooltip,
			    manualUrl);
  }

  // handle plugins we couldn't find
  for (let mimetype in this.mPluginNotFoundArray) {
    if (this.mPluginNotFoundArray[mimetype] == true) {
      var pluginRequest = this.mPluginRequestArray[mimetype];

      // if there is a pluginspage, show UI
      if (pluginRequest.pluginsPage) {
        this.addPluginResultRow(
            "",
            this.getFormattedString("pluginInstallation.unknownPlugin", [mimetype]),
            null,
            null,
            null,
            pluginRequest.pluginsPage);
      }

      notInstalledList += "&mimetype=" + mimetype;
    }
  }

  // no plugins were found, so change the description of the final page.
  if (this.mPluginInfoArrayLength == 0) {
    var noPluginsFound = this.getString("pluginInstallation.noPluginsFound");
    document.getElementById("pluginSummaryDescription").setAttribute("value", noPluginsFound);
  } else if (this.mSuccessfullPluginInstallation == 0) {
    // plugins found, but none were installed.
    var noPluginsInstalled = this.getString("pluginInstallation.noPluginsInstalled");
    document.getElementById("pluginSummaryDescription").setAttribute("value", noPluginsInstalled);
  }

  document.getElementById("pluginSummaryRestartNeeded").hidden = !this.mNeedsRestart;

  var app = Cc["@mozilla.org/xre/app-info;1"].getService(nsIXULAppInfo);

  // set the get more info link to contain the mimetypes we couldn't install.
  notInstalledList +=
    "&appID=" + app.ID +
    "&appVersion=" + app.platformBuildID +
    "&clientOS=" + this.getOS() +
    "&chromeLocale=" + this.getChromeLocale() +
    "&appRelease=" + app.version;

  document.getElementById("moreInfoLink").addEventListener("click", function() { gPluginInstaller.loadURL("https://pfs.mozilla.org/plugins/" + notInstalledList) }, false);

  if (this.mNeedsRestart) {
    var cancel = document.getElementById("plugin-installer-wizard").getButton("cancel");
    cancel.label = this.getString("pluginInstallation.close.label");
    cancel.accessKey = this.getString("pluginInstallation.close.accesskey");
    var finish = document.getElementById("plugin-installer-wizard").getButton("finish");
    finish.label = this.getFormattedString("pluginInstallation.restart.label", [app.name]);
    finish.accessKey = this.getString("pluginInstallation.restart.accesskey");
    this.canCancel(true);
  }
  else {
    this.canCancel(false);
  }

  this.canAdvance(true);
  this.canRewind(false);
}

nsPluginInstallerWizard.prototype.loadURL = function(aUrl) {
  // Check if the page where the plugin came from can load aUrl before
  // loading it, and do *not* allow loading URIs that would inherit our
  // principal.
  
  var pluginPagePrincipal =
    window.opener.content.document.nodePrincipal;

  var secMan = Cc["@mozilla.org/scriptsecuritymanager;1"]
               .getService(nsIScriptSecurityManager);

  secMan.checkLoadURIStrWithPrincipal(pluginPagePrincipal, aUrl,
    nsIScriptSecurityManager.DISALLOW_INHERIT_PRINCIPAL);

  window.opener.open(aUrl);
}

nsPluginInstallerWizard.prototype.getString = function(aName) {
  var result;
  try {
    result = document.getElementById("pluginWizardString").getString(aName);
  }
  catch (e) {
    result = document.getElementById("ubufoxPluginWizardString").getString(aName);
  }
  return result;
}

nsPluginInstallerWizard.prototype.getFormattedString = function (aName, aArray) {
  var result;
  try {
    result = document.getElementById("pluginWizardString").getFormattedString(aName, aArray);
  }
  catch (e) {
    result = document.getElementById("ubufoxPluginWizardString").getFormattedString(aName, aArray);
  }
  return result;
}

nsPluginInstallerWizard.prototype.getOS = function() {
  return Cc["@mozilla.org/network/protocol;1?name=http"]
            .getService(nsIHttpProtocolHandler).oscpu;
}

nsPluginInstallerWizard.prototype.getChromeLocale = function() {
  return Cc["@mozilla.org/chrome/chrome-registry;1"]
            .getService(nsIXULChromeRegistry).getSelectedLocale("global");
}

function nsPluginRequest(aPlugRequest) {
  this.mimetype = encodeURI(aPlugRequest.mimetype);
  this.pluginsPage = aPlugRequest.pluginsPage;
}

var gPluginInstaller;

function wizardInit() {
  gPluginInstaller = new nsPluginInstallerWizard();
  gPluginInstaller.canAdvance(false);
  gPluginInstaller.getPluginData();
}

function wizardFinish() {
  if (gPluginInstaller.mNeedsRestart) {
    // Notify all windows that an application quit has been requested.
    var os = Cc["@mozilla.org/observer-service;1"].getService(nsIObserverService);
    var cancelQuit = Cc["@mozilla.org/supports-PRBool;1"]
                        .createInstance(nsISupportsPRBool);
    os.notifyObservers(cancelQuit, "quit-application-requested", "restart");

    // Something aborted the quit process.
    if (!cancelQuit.data) {
      var appStartup = Cc["@mozilla.org/toolkit/app-startup;1"]
                          .getService(nsIAppStartup);
      appStartup.quit(nsIAppStartup.eAttemptQuit | nsIAppStartup.eRestart);
      return true;
    }
  }

  // don't refresh if no plugins were found or installed
  if ((gPluginInstaller.mSuccessfullPluginInstallation > 0) &&
      (gPluginInstaller.mPluginInfoArray.length != 0)) {

    // reload plugins so JS detection works immediately
    try {
      Cc["@mozilla.org/plugin/host;1"].getService(nsIPluginHost)
                                      .reloadPlugins(false);
    }
    catch (e) {
      // reloadPlugins throws an exception if there were no plugins to load
    }

    if (gPluginInstaller.mBrowser) {
      // notify listeners that a plugin is installed,
      // so that they can reset the UI and update the browser.
      var event = document.createEvent("Events");
      event.initEvent("NewPluginInstalled", true, true);
      gPluginInstaller.mBrowser.dispatchEvent(event);
    }
  }

  return true;
}

