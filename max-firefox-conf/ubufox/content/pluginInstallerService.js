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

var PluginXPIInstallService = {
  
  init: function () 
  {
  },

  pluginPidArray: null,

  startPluginInstallation: function (aPluginXPIUrlsArray,
				     aPluginHashes,
				     aPluginPidArray) {
     this.pluginPidArray = aPluginPidArray;

     var xpiManager = Components.classes["@mozilla.org/xpinstall/install-manager;1"]
                                .createInstance(Components.interfaces.nsIXPInstallManager);
     xpiManager.initManagerWithHashes(aPluginXPIUrlsArray, aPluginHashes,
                                      aPluginXPIUrlsArray.length, this);
  },

  // XPI progress listener stuff
  onStateChange: function (aIndex, aState, aValue)
  {
    // get the pid to return to the wizard
    var pid = this.pluginPidArray[aIndex];
    var errorMsg;

    if (aState == Components.interfaces.nsIXPIProgressDialog.INSTALL_DONE) {
      if (aValue != 0) {
        var xpinstallStrings = document.getElementById("xpinstallStrings");
        try {
          errorMsg = xpinstallStrings.getString("error" + aValue);
        }
        catch (e) {
          errorMsg = xpinstallStrings.getFormattedString("unknown.error", [aValue]);
        }
      }
    }

    gPluginInstaller.pluginXPIInstallationProgress(pid, aState, errorMsg);

  },

  onProgress: function (aIndex, aValue, aMaxValue)
  {
    // get the pid to return to the wizard
    var pid = this.pluginPidArray[aIndex];

    gPluginInstaller.pluginXPIInstallationProgressMeter(pid, aValue, aMaxValue);
  }
}


var AptInstaller = {

  mAptInstallerService: null,
  mAptUrlArray: null,
  mAptPidArray: null,
  mRunning: false,

  install: function(aAptUrlArray,
		    aAptPidArray,
		    aAptInstallerService) {

    this.mAptInstallerService = aAptInstallerService;
    this.mAptUrlArray = aAptUrlArray;
    this.mAptPidArray = aAptPidArray;
    this.mRunning = true;

    //    var thread = Components.classes["@mozilla.org/thread;1"]
    //      .createInstance(Components.interfaces.nsIThread);
    //    thread.init(this, 0, nsIThread.PRIORITY_NORMAL, nsIThread.SCOPE_LOCAL, nsIThread.STATE_UNJOINABLE);
    this.run();
  },

  run: function()
  {
    for (var i = 0; i < this.mAptUrlArray.length; i++) {
      var aptUrl = this.mAptUrlArray[i];
      var aptPid = this.mAptPidArray[i];
      this.mAptInstallerService.onNotifyStart(aptUrl, aptPid);

      var executable =
        Components.classes['@mozilla.org/file/local;1']
       .createInstance(Components.interfaces.nsILocalFile);

      executable.initWithPath("/usr/bin/python");

      if(!executable.exists() || !executable.isExecutable()) {
        window.alert('Unexpected error!');
        this.mAptInstallerService.onNotifyResult(aptUrl, aptPid, -1 );
        continue;
      }

      var procUtil =
        Components.classes['@mozilla.org/process/util;1']
        .createInstance(Components.interfaces.nsIProcess);

      var nsFile = executable.QueryInterface(Components.interfaces.nsIFile);

      procUtil.init(executable);

      var prefBranch = Components.classes["@mozilla.org/preferences-service;1"]
                                .getService(Components.interfaces.nsIPrefBranch);

      var proxyType = prefBranch.getIntPref("network.proxy.type");
      var proxyHost = prefBranch.getCharPref("network.proxy.http");
      var proxyPort = prefBranch.getIntPref("network.proxy.http_port");

      var httpProxy = "";
      if(proxyType > 0 && proxyHost != null && proxyHost.length > 0)
      {
        httpProxy = proxyHost;
        if(proxyPort > 0)
        {
          httpProxy = httpProxy + ":" + proxyPort;
        }
      }

      var args = new Array();
      if(httpProxy.length > 0)
      {
        args = new Array("/usr/bin/apturl", "--http-proxy", httpProxy, aptUrl);
      } else {
        args = new Array("/usr/bin/apturl", aptUrl);
      }
      procUtil.run(true, args, args.length);
      res = procUtil.exitValue;

      this.mAptInstallerService.onNotifyResult(aptUrl, aptPid, res);
    }

    this.mAptInstallerService.onNotifyResult(null, null, -1 );
    mRunning = false;
    return true;
  }
}

var PluginAPTInstallService = {
  
  init: function () 
  {
  },

  pluginPidArray: null,

  startPluginInstallation: function (aPluginAptUrlsArray,
				     aPluginPidArray) {
    AptInstaller.install(aPluginAptUrlsArray, aPluginPidArray, this);
  },

  onNotifyStart: function (aptUrl, aptPid) {
    gPluginInstaller.pluginXPIInstallationProgress(aptPid, 6, null);
  },

  onNotifyResult: function (aptUrl, aptPid, result) {
    if(result > 0) {
      gPluginInstaller.pluginXPIInstallationProgress(aptPid, 7, "Apt Install Failed or Cancelled");
    } else if (result == 0) {
      gPluginInstaller.pluginXPIInstallationProgress(aptPid, 7, null);
    } else {
      gPluginInstaller.pluginXPIInstallationProgress(null, 8, null);
    }
  }
}
