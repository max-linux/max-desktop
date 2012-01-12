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
 * The Original Code is ubufox.
 *
 * The Initial Developer of the Original Code is
 * Canonical Ltd.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Alexander Sack <asac@jwsdot.com> - Canonical Ltd.
 *   Arzhel Younsi <xionox@gmail.com>
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

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cu = Components.utils;

Cu.import("resource://gre/modules/Services.jsm");

var EXPORTED_SYMBOLS = [ "UpdateRestartNotifier" ];

["LOG", "WARN", "ERROR"].forEach(function(aName) {
  this.__defineGetter__(aName, function() {
    Components.utils.import("resource://gre/modules/AddonLogging.jsm");

    LogManager.getLogger("ubufox.urn", this);
    return this[aName];
  });
}, this);

var UpdateRestartNotifierPrivate = {
  listeners: [],
  timer: null,
  fired: false,

  init: function URNP_init() {
    if (this.timer) {
      WARN("Already initialized");
      return;
    }

    var startupTime = Date.now();
    this.timer = Cc["@mozilla.org/timer;1"].createInstance(Ci.nsITimer);

    let launcherFile = Cc["@mozilla.org/file/local;1"]
                          .createInstance(Ci.nsILocalFile);
    var launcher = Cc["@mozilla.org/process/environment;1"]
                      .getService(Ci.nsIEnvironment).get("MOZ_APP_LAUNCHER");
    if(!launcher) {
      WARN("Need to set MOZ_APP_LAUNCHER for the restart notifier to work");
      return;
    }

    try {
      // If the launcher is a full path, just get the basename
      launcherFile.initWithPath(launcher);
      launcher = launcherFile.leafName;
    } catch (e) { // initWithPath will throw if the path is relative
    }

    this.timer.initWithCallback(function() {
      let resReqFile = Cc["@mozilla.org/file/local;1"]
                          .createInstance(Ci.nsILocalFile);
      resReqFile.initWithPath("/var/run/" + launcher + "-restart-required");

      if(resReqFile.exists()) {
        let dateResReq = resReqFile.lastModifiedTime;
        if(dateResReq > startupTime) {
          LOG("Update detected - requesting restart");
          UpdateRestartNotifierPrivate.notifyListeners();
        }
      }
    }, 5000, Ci.nsITimer.TYPE_REPEATING_SLACK);

    Services.obs.addObserver(function(aSubject, aTopic, aData) {
      if (aSubject == "xpcom-will-shutdown"){
        Services.obs.removeObserver(arguments.callee, "xpcom-will-shutdown", false);
        if (UpdateRestartNotifierPrivate.timer) {
          UpdateRestartNotifierPrivate.timer.cancel();
          UpdateRestartNotifierPrivate.timer = null;
        }
      }
    }, "xpcom-will-shutdown", false);
  },

  remindListeners: function URNP_remindListeners() {
    this.listeners.forEach(function(listener) {
      try {
        listener.onUpdatedNotify();
      } catch(e) {
        Cu.reportError(e);
      }
    });
  },

  notifyListeners: function URNP_notifyListeners() {
    try {
      this.timer.cancel();
      this.timer.initWithCallback(function() {
        UpdateRestartNotifierPrivate.remindListeners();
      }, 900000, Ci.nsITimer.TYPE_REPEATING_SLACK);
    } catch(e) {
      Cu.reportError(e);
    }

    this.fired = true;

    this.listeners.forEach(function(listener) {
      try {
        listener.onUpdatedNotify();
      } catch(e) {
        Cu.reportError(e);
      }
    });
  },

  addListener: function URNP_addListener(aListener) {
    if (!aListener)
      throw "A listener must be specified";

    if ((aListener.onUpdatedNotify == undefined) ||
        (typeof(aListener.onUpdatedNotify) != "function"))
      throw "Listener must implement onUpdatedNotify";

    if (this.listeners.indexOf(aListener) != -1)
      throw "Trying to register an observer more than once";

    this.listeners.push(aListener);

    if (this.fired)
      aListener.onUpdatedNotify();
  },

  removeListener: function URNP_removeListener(aListener) {
    let index = this.listeners.indexOf(aListener);
    if (index == -1)
      throw "Trying to remove an observer that was never registered";

    this.listeners.splice(index, 1);
  },

  restart: function URNP_restart() {
    if (!this.fired)
      throw "Calling restart with no pending restart request";

    Cc["@mozilla.org/toolkit/app-startup;1"]
       .getService(Ci.nsIAppStartup)
       .quit(Ci.nsIAppStartup.eRestart | Ci.nsIAppStartup.eAttemptQuit);
  }
};

var UpdateRestartNotifier = {
  addListener: function URN_addListener(aListener) {
    UpdateRestartNotifierPrivate.addListener(aListener);
  },

  removeListener: function URN_removeListener(aListener) {
    UpdateRestartNotifierPrivate.removeListener(aListener);
  },

  restart: function URN_restart() {
    UpdateRestartNotifierPrivate.restart();
  }
};

UpdateRestartNotifierPrivate.init();
