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

if (!com) var com = {};
if (!com.ubuntu) com.ubuntu = {};

(function() {
  const { classes: Cc, utils: Cu } = Components;

  Cu.import("resource://gre/modules/Services.jsm");
  Cu.import("resource://gre/modules/XPCOMUtils.jsm");
  Cu.import("resource://gre/modules/PopupNotifications.jsm");
  Cu.import("resource://ubufox/modules/Distro.jsm");

  var UpdateRestartListener = {
    restartRequested: false,

    addNotificationToBrowser: function URL_addNotificationToBrowser(aBrowser,
                                                                    aRaised) {
      if (this.notificationStyle == "popup") {
        PopupNotifications.show(aBrowser, "ubufox-restart-request",
                                this.restartNotificationLabel,
                                "ubufox-restart-notification-icon",
                                {label: this.restartNotificationButton,
                                 accessKey: this.restartNotificationKey,
                                 callback: (function() {
          gBrowser.tabContainer.removeEventListener("TabOpen",
                                                    this.onTabOpen,
                                                    false);
          gBrowser.browsers.forEach(function(browser) {
            let notification = PopupNotifications.getNotification("ubufox-restart-request",
                                                                  browser);
            if (notification) {
              PopupNotifications.remove(notification);
            }
          });

          Cc["@mozilla.org/toolkit/app-startup;1"].getService(Ci.nsIAppStartup)
                                                  .quit(Ci.nsIAppStartup.eRestart |
                                                        Ci.nsIAppStartup.eAttemptQuit);
        }).bind(this)},
                                [], {dismissed: aRaised ? false : true,
                                     timeout: 8.64e15});
      } else if (this.notificationStyle == "infobar") {
        let notificationBox = gBrowser.getNotificationBox(aBrowser);
        let notification = notificationBox
                           .getNotificationWithValue("notification-restart");
        if (!notification) {
          notificationBox.appendNotification(this.oldRestartNotificationLabel,
                                             "notification-restart", "",
                                             notificationBox.PRIORITY_WARNING_LOW,
                                             [{ label: this.restartNotificationButton,
                                                accessKey: this.restartNotificationKey,
                                                callback: (function() {
            gBrowser.tabContainer.removeEventListener("TabOpen",
                                                      this.onTabOpen,
                                                      false);
            gBrowser.browsers.forEach(function(browser) {
              let notificationBox = gBrowser.getNotificationBox(browser);
              let notification = notificationBox
                                 .getNotificationWithValue("notification-restart");
              if (notification) {
                notificationBox.removeNotification(notification);
              }
            });

            if (this.intervalId) {
              window.clearInterval(this.intervalId);
            }

            Cc["@mozilla.org/toolkit/app-startup;1"].getService(Ci.nsIAppStartup)
                                                    .quit(Ci.nsIAppStartup.eRestart |
                                                          Ci.nsIAppStartup.eAttemptQuit);
          }).bind(this)}]);
        }
      }
    },

    onTabOpen: function URL_onTabOpen(aEvent) {
      window.setTimeout(function() {
        UpdateRestartListener.addNotificationToBrowser(gBrowser.getBrowserForTab(aEvent.target),
                                                       false);
      }, 0);
    },

    displayNotifications: function URL_displayNotifications() {
      gBrowser.browsers.forEach(function(browser) {
        this.addNotificationToBrowser(browser,
                                      browser == gBrowser.selectedBrowser ?
                                       true : false);
      }, this);
    },

    requestRestart: function URL_requestRestart() {
      if (!this.restartRequested) {
        gBrowser.tabContainer.addEventListener("TabOpen", this.onTabOpen, false);
        this.displayNotifications();

        if (this.notificationStyle == "infobar") {
          this.intervalId = window.setInterval(this.displayNotifications.bind(this),
                                               900000);
        }

        this.restartRequested = true;
      }
    },

    onAppUpdated: function URL_onAppUpdated() {
      this.requestRestart();
    },

    onPluginsUpdated: function URL_onPluginsUpdated() {
      if (this.notificationStyle == "popup") {
        this.requestRestart();
      }
    }
  };

  XPCOMUtils.defineLazyGetter(UpdateRestartListener,
                              "notificationStyle",
                              function() {
    try {
      return Services.prefs.getCharPref("extensions.ubufox.update-restart-notification-style");
    } catch(e) {
      return distro.updateRestartNotificationStyle;
    }
  });

  XPCOMUtils.defineLazyGetter(UpdateRestartListener,
                              "oldRestartNotificationLabel",
                              function() {
    return document.getElementById("ubufox-restart-strings")
                   .getString("oldRestartNotificationLabel");
  });

  XPCOMUtils.defineLazyGetter(UpdateRestartListener,
                              "restartNotificationLabel",
                              function() {
    return document.getElementById("ubufox-restart-strings")
                   .getFormattedString("restartNotificationLabel",
                                       [document.getElementById("bundle_brand")
                                                .getString("brandShortName")]);
  });

  XPCOMUtils.defineLazyGetter(UpdateRestartListener,
                              "restartNotificationButton",
                              function() {

    return document.getElementById("ubufox-restart-strings")
                   .getString("restartNotificationButton");
  });

  XPCOMUtils.defineLazyGetter(UpdateRestartListener,
                              "restartNotificationKey",
                              function() {
    return document.getElementById("ubufox-restart-strings")
                   .getString("restartNotificationKey");
  });

  this.Ubufox = {
    reportBug: function() {
      distro.reportBug();
    },

    help: function() {
      openUILinkIn(distro.helpURL, "tab");
    }
  };

  // Override an existing function with a new function, chaining up to
  // the old function
  function overrideExistingFunction(aObject, aName, aReplacement) {
    var old = aObject[aName];
    aObject[aName] = function() {
      try {
        aReplacement.apply(aObject, arguments);
      } catch(e) {
        Cu.reportError(e);
      }
      old.apply(aObject, arguments);
    }
  }

  overrideExistingFunction(window, "buildHelpMenu", function() {
    document.getElementById("ubufox-reportbug").setAttribute("hidden",
                                                             !distro.canReportBug);
  });

  addEventListener("load", function() {
    try {
      removeEventListener("load", arguments.callee, false);
      Cc["@ubuntu.com/update-notifier;1"].getService()
                                         .wrappedJSObject
                                         .addListener(UpdateRestartListener);
      gInitialPages.push("about:startpage");
    } catch(e) {
      Cu.reportError(e);
    }
  }, false);

  addEventListener("unload", function() {
    removeEventListener("unload", arguments.callee, false);
    // Don't remove this call, ever. Without this we will leak the
    // document as it is in the scope of the callback, which
    // UpdateRestartNotifier is holding on to
    Cc["@ubuntu.com/update-notifier;1"].getService()
                                       .wrappedJSObject
                                       .removeListener(UpdateRestartListener);
  }, false);

}).call(com.ubuntu);
