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

function nsPluginInstallerWizard(){

  // create the request array
  this.mPluginRequestArray = new Object();
  // since the array is a hash, store the length
  this.mPluginRequestArrayLength = 0;

  // create the plugin info array.
  // a hash indexed by plugin id so we don't install 
  // the same plugin more than once.
  this.mPluginInfoArray = new Object();
  this.mPluginInfoArrayLength = 0;

  this.mMimeTypePluginSelections = new Object();

  // holds plugins we couldn't find
  this.mPluginNotFoundArray = new Object();
  this.mPluginNotFoundArrayLength = 0;

  // array holding pids of plugins that require a license
  this.mPluginLicenseArray = new Array();

  this.mPluginGroupBoxes = new Array();
  this.mPluginPlaceHolder = null;

  // how many plugins are to be installed
  this.pluginsToInstallNum = 0;

  this.mTab = null;
  this.mBrowser = null;
  this.mSuccessfullPluginInstallation = 0;

  this.mPluginPidArray = new Object();
  // arguments[0] is an array that contains two items:
  //     an array of mimetypes that are missing
  //     a reference to the tab that needs them, so we can reload it

  if ("arguments" in window) {
    for (var item in window.arguments[0].plugins){
      this.mPluginRequestArray[window.arguments[0].plugins[item].mimetype] =
        new nsPluginRequest(window.arguments[0].plugins[item]);

      this.mPluginRequestArrayLength++;
    }

    this.mBrowser = window.arguments[0].browser; // ffox 3
    this.mTab = window.arguments[0].tab; // ffox 2
  }

  this.WSPluginCounter = 0;
  this.licenseAcceptCounter = 0;

  this.prefBranch = null;

  this.mNeedsRestart = false;
}

nsPluginInstallerWizard.prototype.getPluginData = function (){
  // for each mPluginRequestArray item, call the datasource
  this.WSPluginCounter = 0;

  // initiate the datasource call
  var rdfUpdater = new nsRDFItemUpdater(this.getOS(), this.getChromeLocale());

  for (item in this.mPluginRequestArray) {
    rdfUpdater.checkForPlugin(this.mPluginRequestArray[item]);
  }
}

// aPluginInfo is null if the datasource call failed, and pid is -1 if
// no matching plugin was found.
nsPluginInstallerWizard.prototype.pluginInfoReceived = function (aPluginInfos){
  this.WSPluginCounter++;

  if (aPluginInfos ) {
    // hash by id

    var resultSetMimeType = null;
    var filteredPluginInfoSet = new Array();
    var noResultInfo = null;
    var aPluginInfo = null;
    for (var i = 0; i < aPluginInfos.length; i++) {
      aPluginInfo = aPluginInfos[i];
      resultSetMimeType = aPluginInfo.requestedMimetype;
      if (aPluginInfo && aPluginInfo.pid != -1 &&
	  ( aPluginInfo.XPILocation || aPluginInfo.manualInstallationURL )) {
	hasResults = true;
	filteredPluginInfoSet.push(aPluginInfo);
	this.mPluginPidArray[aPluginInfo.pid] = aPluginInfo;
      }
    }

    if(filteredPluginInfoSet.length > 0)
    {
      this.mPluginInfoArray[resultSetMimeType] = filteredPluginInfoSet;
      this.mPluginInfoArrayLength++;
    } else {
      this.mPluginNotFoundArray[resultSetMimeType] = filteredPluginInfoSet;
      this.mPluginNotFoundArrayLength++;
    }
  }

  var progressMeter = document.getElementById("ws_request_progress");

  if (progressMeter.getAttribute("mode") == "undetermined")
    progressMeter.setAttribute("mode", "determined");

  progressMeter.setAttribute("value",
      ((this.WSPluginCounter / this.mPluginRequestArrayLength) * 100) + "%");

  if (this.WSPluginCounter == this.mPluginRequestArrayLength) {
    // check if no plugins were found
    if (this.mPluginInfoArrayLength == 0 && this.mPluginInfoAptArrayLength == 0) {
      this.advancePage("lastpage", true, false, false);
    } else {
      // we want to allow user to cancel
      this.advancePage(null, true, false, true);
    }
  } else {
    // process more.
  }
}

nsPluginInstallerWizard.prototype.createPluginSetGroupBox = function () {

  var gbox = document.createElement("vbox");
  gbox.setAttribute("flex", "1");

  var caption = document.createElement("caption");
  caption.setAttribute("label", this.getString("ubufox.pluginWizard.availablePluginsPage.description.label")+" "+mimetype+":");
  gbox.appendChild(caption);

  return gbox;
}

nsPluginInstallerWizard.prototype.showPluginList = function () {
  var toReplace = null;

  if(this.mPluginGroupBoxes.length > 0)
    this.mPluginGroupBoxes.pop();
  
  if (toReplace && this.mPluginPlaceHolder) {
    toReplace.getParent().replaceChild(toReplace, 
				       this.mPluginPlaceHolder);
  }
  
  this.pluginsToInstallNum = 0;
  var hasPluginWithInstallerUI = false;

  var groupBox = null;
  var lastSibling = null;

  for (mimetype in this.mPluginInfoArray){
    // [plugin image] [Plugin_Name Plugin_Version]
    var pluginInfoSet = this.mPluginInfoArray[mimetype];
    var firstPluginSet = (groupBox == null);
    groupBox = this.createPluginSetGroupBox(document, mimetype, pluginInfoSet);

    if(firstPluginSet) {
      this.mPluginPlaceHolder = document.getElementById("pluginselection-placeholder");
      this.mPluginPlaceHolder.parentNode.replaceChild(groupBox, this.mPluginPlaceHolder);
    } else {
      lastSibling.parentNode.insertBefore(groupBox, lastSibling);
    }
    lastSibling = groupBox;
    this.mPluginGroupBoxes.push(groupBox);

    var vbox = document.createElement("vbox");
    vbox.setAttribute("id", "pluginList");
    vbox.setAttribute("flex", "1");
    vbox.setAttribute("style","overflow: auto;");
    groupBox.appendChild(vbox);

    var radiogroup = document.createElement("radiogroup");
    radiogroup.setAttribute("flex", "1");
    vbox.appendChild(radiogroup);

    // OK, lets add the "None" option first
    var myRadio = null;

    var first = true;
    for (var i = 0; i < pluginInfoSet.length; i++) {
      var table = document.createElement("table");
      table.setAttribute("class", "plugin-row-table");

      var row = document.createElement("tr");
      var cellMyRadio = document.createElement("td");
      cellMyRadio.setAttribute("class", "plugin-radio-cell");

      var pluginInfo = pluginInfoSet[i];
      // create the radio
      myRadio = document.createElement("radio");
      cellMyRadio.appendChild(myRadio);
      row.appendChild(cellMyRadio);

      myRadio.setAttribute("selected", "false");
      // XXXlocalize (nit)
      if(first) {
	myRadio.setAttribute("selected", "true");
	first = false;
	gPluginInstaller.toggleInstallPlugin(mimetype, pluginInfo.pid, myRadio);
      }

      myRadio.setAttribute("oncommand", "gPluginInstaller.toggleInstallPlugin('" + mimetype + "', '" + pluginInfo.pid + "',  this)");
      myRadio.setAttribute("label", pluginInfo.name + " " + (pluginInfo.version ? pluginInfo.version : ""));
      myRadio.setAttribute("flex", "1");

      var distributorImage = document.createElement("image");
      distributorImage.setAttribute("class", "distributor-image");

      var cellDistributorImage = document.createElement("td");
      cellDistributorImage.setAttribute("class", "cell-distributor-image");

      cellDistributorImage.appendChild(distributorImage);
      row.appendChild(cellDistributorImage);

      if(pluginInfo.XPILocation && pluginInfo.XPILocation.indexOf("apt:") == 0) {
	distributorImage.setAttribute("src", "chrome://ubufox/content/ubuntulogo32.png");
	pluginInfo.IconUrl = "chrome://ubufox/content/ubuntulogo32.png";
      } else {
	if(!pluginInfo.IconUrl || pluginInfo.IconUrl.length == 0) {
	  distributorImage.setAttribute("src", "chrome://ubufox/content/internet32.png");
	  pluginInfo.IconUrl = "chrome://ubufox/content/internet32.png";
	} else {
	  distributorImage.setAttribute("src", pluginInfo.IconUrl);
	}
      }

      table.appendChild(row);

      row.setAttribute("flex", "1");
      cellMyRadio.setAttribute("flex", "1");

      radiogroup.appendChild(table);

    }

    if (pluginInfo.InstallerShowsUI == "true")
      hasPluginWithInstallerUI = true;

    this.pluginsToInstallNum++;
  }

  if (hasPluginWithInstallerUI)
    document.getElementById("installerUI").hidden = false;

  this.canAdvance(false);
  this.canRewind(false);
}

nsPluginInstallerWizard.prototype.toggleInstallPlugin = function (aMimetype, aPid, aCheckbox) {

  this.mMimeTypePluginSelections[aMimetype] = aPid;
  count = 0;
  for (mime in this.mMimeTypePluginSelections) {
    if(this.mMimeTypePluginSelections[mime] && this.mMimeTypePluginSelections[mime] != "-1")
      count++;
  }

  // if no plugins are checked, don't allow to advance
  if (count > 0)
    this.canAdvance(true);
  else
    this.canAdvance(false);
}

nsPluginInstallerWizard.prototype.canAdvance = function (aBool){
  document.getElementById("plugin-installer-wizard").canAdvance = aBool;
}

nsPluginInstallerWizard.prototype.canRewind = function (aBool){
  document.getElementById("plugin-installer-wizard").canRewind = aBool;
}

nsPluginInstallerWizard.prototype.canCancel = function (aBool){
  document.documentElement.getButton("cancel").disabled = !aBool;
}

nsPluginInstallerWizard.prototype.showLicenses = function (){
  this.canAdvance(false);
  this.canRewind(false);

  // only add if a license is provided and the plugin was selected to
  // be installed
  for (mimetype in this.mMimeTypePluginSelections){
    var pid = this.mMimeTypePluginSelections[mimetype];
    var pluginInfo = this.mPluginPidArray[pid];
    if (pluginInfo && pluginInfo.licenseURL && (pluginInfo.licenseURL != "")) {
      this.mPluginLicenseArray.push(pluginInfo.pid);
    }
  }

  if (this.mPluginLicenseArray.length == 0) {
    // no plugins require licenses
    this.advancePage(null, true, false, false);
  } else {
    this.licenseAcceptCounter = 0;

    // add a nsIWebProgress listener to the license iframe.
    var docShell = document.getElementById("licenseIFrame").docShell;
    var iiReq = docShell.QueryInterface(Components.interfaces.nsIInterfaceRequestor);
    var webProgress = iiReq.getInterface(Components.interfaces.nsIWebProgress);
    webProgress.addProgressListener(gPluginInstaller.progressListener,
                                    Components.interfaces.nsIWebProgress.NOTIFY_ALL);


    this.showLicense();
  }
}

nsPluginInstallerWizard.prototype.enableNext = function (){
  // if only one plugin exists, don't enable the next button until
  // the license is accepted
  if (gPluginInstaller.pluginsToInstallNum > 1)
    gPluginInstaller.canAdvance(true);

  document.getElementById("licenseRadioGroup1").disabled = false;
  document.getElementById("licenseRadioGroup2").disabled = false;
}

const nsIWebProgressListener = Components.interfaces.nsIWebProgressListener;
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
     if (aIID.equals(Components.interfaces.nsIWebProgressListener) ||
         aIID.equals(Components.interfaces.nsISupportsWeakReference) ||
         aIID.equals(Components.interfaces.nsISupports))
       return this;
     throw Components.results.NS_NOINTERFACE;
   }
}

nsPluginInstallerWizard.prototype.showLicense = function (){
  var pluginInfo = this.mPluginPidArray[this.mPluginLicenseArray[this.licenseAcceptCounter]];

  this.canAdvance(false);

  loadFlags = Components.interfaces.nsIWebNavigation.LOAD_FLAGS_NONE;

  document.getElementById("licenseIFrame").webNavigation.loadURI(pluginInfo.licenseURL, loadFlags, null, null, null);

  document.getElementById("pluginLicenseLabel").firstChild.nodeValue = 
    this.getFormattedString("pluginLicenseAgreement.label", [pluginInfo.name]);

  document.getElementById("licenseRadioGroup1").disabled = true;
  document.getElementById("licenseRadioGroup2").disabled = true;
  document.getElementById("licenseRadioGroup").selectedIndex = 
    pluginInfo.licenseAccepted ? 0 : 1;
}

nsPluginInstallerWizard.prototype.showNextLicense = function (){
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

nsPluginInstallerWizard.prototype.showPreviousLicense = function (){
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

nsPluginInstallerWizard.prototype.storeLicenseRadioGroup = function (){
  var pluginInfo = this.mPluginPidArray[this.mPluginLicenseArray[this.licenseAcceptCounter]];
  pluginInfo.licenseAccepted = !document.getElementById("licenseRadioGroup").selectedIndex;
}

nsPluginInstallerWizard.prototype.licenseRadioGroupChange = function(aAccepted) {
  // only if one plugin is to be installed should selection change the next button
  if (this.pluginsToInstallNum == 1)
    this.canAdvance(aAccepted);
}

nsPluginInstallerWizard.prototype.advancePage = function (aPageId, aCanAdvance, aCanRewind, aCanCancel){
  this.canAdvance(true);
  document.getElementById("plugin-installer-wizard").advance(aPageId);

  this.canAdvance(aCanAdvance);
  this.canRewind(aCanRewind);
  this.canCancel(aCanCancel);
}

nsPluginInstallerWizard.prototype.startPluginInstallation = function (){
  this.canAdvance(false);
  this.canRewind(false);

  // since the user can choose what plugins to install, we need to store
  // which ones were choosen, as nsIXPInstallManager returns an index and not the
  // mimetype.  So store the pids.

  // for ubutfox we deal with multiple cases: first case is XPIInstall, which will
  // run the "normal" XPIInstall process; second case is XPIInstall url contains
  // and apt: protocol url ... this will run apt protocol handler 
  var pluginURLArray = new Array();
  var pluginHashArray = new Array();
  var pluginPidArray = new Array();
  this.mAptPluginURLArray = new Array();
  this.mAptPluginPidArray = new Array();

  for (mime in this.mMimeTypePluginSelections) {
    var pluginPid = this.mMimeTypePluginSelections[mime];
    var pluginItem = this.mPluginPidArray[pluginPid];

    // only push to the array if it has an XPILocation, else nsIXPInstallManager
    // will complain.
    if (pluginItem && pluginItem.XPILocation && pluginItem.XPILocation.indexOf("apt:") != 0 && pluginItem.licenseAccepted) {
      pluginURLArray.push(pluginItem.XPILocation);
      pluginHashArray.push(pluginItem.XPIHash);
      pluginPidArray.push(pluginItem.pid);
    } else if (pluginItem && pluginItem.XPILocation && pluginItem.XPILocation.indexOf("apt:") == 0) {
      this.mAptPluginURLArray.push(pluginItem.XPILocation);
      this.mAptPluginPidArray.push(pluginPid);
    } else {
      window.alert("Unhandled mime install flavour (supported: vendor, apt)");
      continue;
    }
  }

  if (pluginURLArray.length > 0)
    PluginXPIInstallService.startPluginInstallation(pluginURLArray,
						    pluginHashArray,
						    pluginPidArray);
  else if (this.mAptPluginURLArray.length > 0)
    PluginAPTInstallService.startPluginInstallation(this.mAptPluginURLArray,
						    this.mAptPluginPidArray);
  else
    this.advancePage(null, true, false, false);
}

/*
  0    starting download
  1    download finished
  2    starting installation
  3    finished installation
  4    all done
*/
nsPluginInstallerWizard.prototype.pluginXPIInstallationProgress = function (aPid, aProgress, aError) {

  var statMsg = null;
  var pluginInfo = null;

  if(aPid)
    pluginInfo = gPluginInstaller.mPluginPidArray[aPid];

  switch (aProgress) {

    case 0:
      statMsg = this.getFormattedString("pluginInstallation.download.start", [pluginInfo.name]);
      break;

    case 1:
      statMsg = this.getFormattedString("pluginInstallation.download.finish", [pluginInfo.name]);
      break;

    case 2:
      statMsg = this.getFormattedString("pluginInstallation.install.start", [pluginInfo.name]);
      break;
    case 6:
      statMsg = "APT - " + this.getFormattedString("pluginInstallation.install.start", [pluginInfo.name]);
      break;
    case 3:
      if (aError) {
        statMsg = this.getFormattedString("pluginInstallation.install.error", [pluginInfo.name, aError]);
        pluginInfo.error = aError;
      } else {
        statMsg = this.getFormattedString("pluginInstallation.install.finish", [pluginInfo.name]);
        pluginInfo.error = null;
      }
      break;

    case 7:
      if (aError) {
        statMsg = "APT - " + this.getFormattedString("pluginInstallation.install.error", [pluginInfo.name, aError]);
        pluginInfo.error = aError;
      } else {
        statMsg = "APT - " + this.getFormattedString("pluginInstallation.install.finish", [pluginInfo.name]);
        pluginInfo.error = null;
      }
      break;

    case 4:
      PluginAPTInstallService.startPluginInstallation(this.mAptPluginURLArray,
						      this.mAptPluginPidArray);
      break;
    case 8:
      this.advancePage(null, true, false, false);
      statMsg = this.getString("pluginInstallation.complete");
      break;
    default:
      window.alert("unexpected error during plugin install [code=1001]");
      break;
  }

  if (statMsg)
    document.getElementById("plugin_install_progress_message").value = statMsg;
}

nsPluginInstallerWizard.prototype.pluginInstallationProgressMeter = function (aPid, aValue, aMaxValue){
  var progressElm = document.getElementById("plugin_install_progress");

  if (progressElm.getAttribute("mode") == "undetermined")
    progressElm.setAttribute("mode", "determined");
  
  progressElm.setAttribute("value", Math.ceil((aValue / aMaxValue) * 100) + "%")
}

nsPluginInstallerWizard.prototype.addPluginResultRow = function (aImgSrc, aName, aNameTooltip, aStatus, aStatusTooltip, aManualUrl){
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
  if (aNameTooltip)
    myLabel.setAttribute("tooltiptext", aNameTooltip);
  myRow.appendChild(myLabel);

  if (aStatus) {
    myLabel = document.createElement("label");
    myLabel.setAttribute("value", aStatus);
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

nsPluginInstallerWizard.prototype.showPluginResults = function (){
  var notInstalledList = "?action=missingplugins";
  var myRows = document.getElementById("pluginResultList");

  this.mNeedsRestart = false;

  // clear children
  for (var run = myRows.childNodes.length; run--; run > 0)
    myRows.removeChild(myRows.childNodes.item(run));

  for (mimetype in this.mMimeTypePluginSelections) {
    var pid = this.mMimeTypePluginSelections[mimetype];
    var pluginInfoItem = this.mPluginPidArray[pid];
    // [plugin image] [Plugin_Name Plugin_Version] [Success/Failed] [Manual Install (if Failed)]

    var myPluginItem = pluginInfoItem; //this.mPluginInfoArray[pluginInfoItem];

    var statusMsg;
    var statusTooltip;
    if (myPluginItem.error){
      statusMsg = this.getString("pluginInstallationSummary.failed");
      statusTooltip = myPluginItem.error;
      notInstalledList += "&mimetype=" + pluginInfoItem;
    } else if (myPluginItem.licenseURL && !myPluginItem.licenseAccepted) {
      statusMsg = this.getString("pluginInstallationSummary.licenseNotAccepted");
    } else if (!myPluginItem.XPILocation) {
      statusMsg = this.getString("pluginInstallationSummary.notAvailable");
      notInstalledList += "&mimetype=" + pluginInfoItem;
    } else {
      this.mSuccessfullPluginInstallation++;
      statusMsg = this.getString("pluginInstallationSummary.success");

      // only check needsRestart if the plugin was successfully installed.
      if (myPluginItem.needsRestart)
	this.mNeedsRestart = false;
    }

    // manual url - either returned from the webservice or the pluginspage attribute
    var manualUrl;
    if ((myPluginItem.error || !myPluginItem.XPILocation) && (myPluginItem.manualInstallationURL || this.mPluginRequestArray[myPluginItem.requestedMimetype].pluginsPage)){
      manualUrl = myPluginItem.manualInstallationURL ? myPluginItem.manualInstallationURL : this.mPluginRequestArray[myPluginItem.requestedMimetype].pluginsPage;
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
  for (pluginInfoItem in this.mPluginNotFoundArray){
    var pluginRequest = this.mPluginRequestArray[pluginInfoItem];

    // if there is a pluginspage, show UI
    if (pluginRequest) {
      this.addPluginResultRow(
          "",
          this.getFormattedString("pluginInstallation.unknownPlugin", [pluginInfoItem]),
          null,
          null,
          null,
          pluginRequest.pluginsPage);
    }

    notInstalledList += "&mimetype=" + pluginInfoItem;
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

  var app = Components.classes["@mozilla.org/xre/app-info;1"]
                      .getService(Components.interfaces.nsIXULAppInfo);

  // set the get more info link to contain the mimetypes we couldn't install.
  notInstalledList +=
    "&appID=" + app.ID +
    "&appVersion=" + app.platformBuildID +
    "&clientOS=" + this.getOS() +
    "&chromeLocale=" + this.getChromeLocale();

  document.getElementById("moreInfoLink").addEventListener("click", function() { gPluginInstaller.loadURL("https://pfs.mozilla.org/plugins/" + notInstalledList) }, false);

  this.canAdvance(true);
  this.canRewind(false);
  this.canCancel(false);
}

nsPluginInstallerWizard.prototype.loadURL = function (aUrl){
  // Check if the page where the plugin came from can load aUrl before
  // loading it, and do *not* allow loading javascript: or data: URIs.
  var pluginPage = window.opener.content.location.href;

  const nsIScriptSecurityManager =
    Components.interfaces.nsIScriptSecurityManager;
  var secMan =
    Components.classes["@mozilla.org/scriptsecuritymanager;1"]
    .getService(nsIScriptSecurityManager);

  secMan.checkLoadURIStr(pluginPage, aUrl,
                         nsIScriptSecurityManager.DISALLOW_SCRIPT_OR_DATA);

  window.opener.open(aUrl);
}

nsPluginInstallerWizard.prototype.getString = function (aName){
  var result;
  try {
    result = document.getElementById("pluginWizardString").getString(aName);
  }
  catch (e) {
    result = document.getElementById("ubufoxPluginWizardString").getString(aName);
  }
  return result;
}

nsPluginInstallerWizard.prototype.getFormattedString = function (aName, aArray){
  var result;
  try {
    result = document.getElementById("pluginWizardString").getFormattedString(aName, aArray);
  }
  catch (e) {
    result = document.getElementById("ubufoxPluginWizardString").getFormattedString(aName, aArray);
  }
  return result;
}

nsPluginInstallerWizard.prototype.getOS = function (){
  var httpService = Components.classes["@mozilla.org/network/protocol;1?name=http"]
                              .getService(Components.interfaces.nsIHttpProtocolHandler);
  return httpService.oscpu;
}

nsPluginInstallerWizard.prototype.getChromeLocale = function (){
  var chromeReg = Components.classes["@mozilla.org/chrome/chrome-registry;1"]
                            .getService(Components.interfaces.nsIXULChromeRegistry);
  return chromeReg.getSelectedLocale("global");
}

nsPluginInstallerWizard.prototype.getPrefBranch = function (){
  if (!this.prefBranch)
    this.prefBranch = Components.classes["@mozilla.org/preferences-service;1"]
                                .getService(Components.interfaces.nsIPrefBranch);
  return this.prefBranch;
}
function nsPluginRequest(aPlugRequest){
  this.mimetype = encodeURI(aPlugRequest.mimetype);
  this.pluginsPage = aPlugRequest.pluginsPage;
}

function PluginInfo(aResult) {
  this.name = aResult.name;
  this.pid = aResult.pid;
  this.version = aResult.version;
  this.IconUrl = aResult.IconUrl;
  this.XPILocation = aResult.XPILocation;
  this.XPIHash = aResult.XPIHash;
  this.InstallerShowsUI = aResult.InstallerShowsUI;
  this.manualInstallationURL = aResult.manualInstallationURL;
  this.requestedMimetype = aResult.requestedMimetype;
  this.licenseURL = aResult.licenseURL;
  this.needsRestart = (aResult.needsRestart == "true");

  this.error = null;
  this.toBeInstalled = true;

  // no license provided, make it accepted
  this.licenseAccepted = this.licenseURL ? false : true;
}

var gPluginInstaller;

function wizardInit(){
  gPluginInstaller = new nsPluginInstallerWizard();
  gPluginInstaller.canAdvance(false);
  gPluginInstaller.getPluginData();
}

function wizardFinish(){
  // we restart if we have no choice ...
  if (gPluginInstaller.mNeedsRestart) {
    // Notify all windows that an application quit has been requested.
    var os = Components.classes["@mozilla.org/observer-service;1"]
                       .getService(Components.interfaces.nsIObserverService);
    var cancelQuit = Components.classes["@mozilla.org/supports-PRBool;1"]
                               .createInstance(Components.interfaces.nsISupportsPRBool);
    os.notifyObservers(cancelQuit, "quit-application-requested", "restart");

    // Something aborted the quit process.
    if (!cancelQuit.data) {
      var nsIAppStartup = Components.interfaces.nsIAppStartup;
      var appStartup = Components.classes["@mozilla.org/toolkit/app-startup;1"]
                                 .getService(nsIAppStartup);
      appStartup.quit(nsIAppStartup.eAttemptQuit | nsIAppStartup.eRestart);
      return true;
    }
  }

  if (gPluginInstaller.mBrowser) { // ffox 3 code can autoscan ...
    // always refresh
    var event = document.createEvent("Events");
    event.initEvent("NewPluginInstalled", true, true);
    var dispatched = gPluginInstaller.mBrowser.dispatchEvent(event);
  }
  else if (gPluginInstaller.mTab) { // ffox 2 code can autoscan ...
    if ((gPluginInstaller.mSuccessfullPluginInstallation > 0) &&
       (gPluginInstaller.mPluginInfoArrayLength != 0)) {
      // clear the tab's plugin list only if we installed at least one plugin
      gPluginInstaller.mTab.missingPlugins = null;
      // reset UI
      window.opener.gMissingPluginInstaller.closeNotification();
      // reload the browser to make the new plugin show
      window.opener.getBrowser().reloadTab(gPluginInstaller.mTab);
    }
  }

  return true;
}

