/* ***** BEGIN LICENSE BLOCK *****
 *   Version: MPL 1.1/GPL 2.0/LGPL 2.1
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
 * The Original Code is distro-mods.
 *
 * The Initial Developer of the Original Code is
 * Canonical Ltd.
 * Portions created by the Initial Developer are Copyright (C) 2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

Components.utils.import("resource://gre/modules/Services.jsm");
Components.utils.import("resource://ubufox/UpdateRestartNotifier.jsm");

if (!com) var com = {};
if (!com.ubuntu) com.ubuntu = {};

(function() {
  const Ci = Components.interfaces;
  const Cc = Components.classes;
  const Cu = Components.utils;

  var UpdateRestartListener = {
    updated: false,
    _buttons: null,
    _restartNotificationLabel: null,

    get buttons() {
      if (!this._buttons) {
        let bundle = document.getElementById("ubufox-restart-strings");
        let restartNotificationButton = bundle.getString("restartNotificationButton");
        let restartNotificationKey = bundle.getString("restartNotificationKey");

        this._buttons = [{ label: restartNotificationButton,
                           accessKey: restartNotificationKey,
                           callback: UpdateRestartNotifier.restart }];
      }

      return this._buttons;
    },

    get restartNotificationLabel() {
      if (!this._restartNotificationLabel) {
        let bundle = document.getElementById("ubufox-restart-strings");
        this._restartNotificationLabel = bundle.getString("restartNotificationLabel");
      }

      return this._restartNotificationLabel;
    },

    addNotificationToBrowser: function URL_addNotificationToBrowser(browser) {
      let notificationBox = gBrowser.getNotificationBox(browser);
      let notification = notificationBox
                         .getNotificationWithValue("notification-restart");
      if (!notification) {
        notificationBox.appendNotification(this.restartNotificationLabel,
                                           "notification-restart", "",
                                           notificationBox.PRIORITY_WARNING_LOW,
                                           this.buttons);
      }
    },

    onUpdatedNotify: function URL_onUpdatedNotify() {
      if (!this.updated) {
        this.updated = true;

        gBrowser.tabContainer.addEventListener("TabOpen", function(aEvent) {
          UpdateRestartListener.addNotificationToBrowser(gBrowser
                                                         .getBrowserForTab(aEvent.target));
        }, false);
      }

      gBrowser.browsers.forEach(function(browser) {
        UpdateRestartListener.addNotificationToBrowser(browser);
      });
    }
  };

  this.Ubufox = {
    openPluginFinder: function() {
      let contentMimeArray = {};
      let pluginsOnTab = false;
      let elements = gBrowser.selectedBrowser.contentDocument
                                             .getElementsByTagName("embed");
      for (let a = 0; a < elements.length; a++) {
        let element = elements[a];
        let pluginInfo = getPluginInfo(element);
        contentMimeArray[pluginInfo.mimetype] = pluginInfo;
        pluginsOnTab = true;
      }
      window.openDialog("chrome://ubufox/content/pluginAlternativeOverlay.xul",
                       "PFSWindow", "chrome,centerscreen,resizable=yes",
                       {plugins: contentMimeArray,
                        browser: gBrowser.selectedBrowser,
                        pluginsOnTab: pluginsOnTab});
    },

    reportBug: function() {
      let executable = Cc["@mozilla.org/file/local;1"]
                          .createInstance(Ci.nsILocalFile);

      executable.initWithPath("/usr/bin/ubuntu-bug");

      if(!executable.exists () || !executable.isExecutable())
        alert('Unexpected error!');

      let procUtil = Cc["@mozilla.org/process/util;1"]
                        .createInstance(Ci.nsIProcess);

      procUtil.init(executable);

      let pkgname = Cc["@mozilla.org/xre/app-info;1"]
                    .getService(Ci.nsIXULAppInfo).name.toLowerCase()
      if (!pkgname) {
          pkgname = "firefox";
      }
      let args = new Array(pkgname);

      procUtil.run(false, args, args.length);
    },

    help: function() {
      let codename = Services.prefs.getCharPref("extensions.ubufox.codename");
      let url = "https://launchpad.net/distros/ubuntu/" + codename + "/+sources/firefox/+gethelp";
      openUILinkIn(url, "tab");
    },

    translate: function() {
      let codename = Services.prefs.getCharPref("extensions.ubufox.codename");
      let url = "https://launchpad.net/distros/ubuntu/" +
                codename + "/+sources/firefox/+translate";
      openUILinkIn(url, "tab");
    },
  };

  addEventListener("load", function() {
    try {
      window.removeEventListener("load", arguments.callee, false);
      UpdateRestartNotifier.addListener(UpdateRestartListener);
      let item = document.getElementById("ubufox-helptranslate");
      item.hidden = true;
    } catch(e) {
      Cu.reportError(e);
    }
  }, false);

  addEventListener("unload", function() {
    removeEventListener("unload", arguments.callee, false);
    // Don't remove this call, ever. Without this we will leak the
    // document as it is in the scope of the callback, which
    // UpdateRestartNotifier is holding on to
    UpdateRestartNotifier.removeListener(UpdateRestartListener);
  }, false);

}).call(com.ubuntu);
