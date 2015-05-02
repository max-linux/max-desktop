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

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;

Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/FileUtils.jsm");
Cu.import("resource://gre/modules/ctypes.jsm");
Cu.import("resource://ubufox/modules/utils.jsm");
Cu.import("resource://ubufox/libs/gio.jsm");
Cu.import("resource://ubufox/libs/gobject.jsm");
Cu.import("resource://ubufox/libs/glib.jsm");

const NS_XPCOM_CURRENT_PROCESS_DIR        = "XCurProcD";
const NS_GRE_DIR                          = "GreD";
const NS_USER_PLUGINS_DIR                 = "UserPlugins";
const NS_APP_PLUGINS_DIR                  = "APlugns";
const NS_SYSTEM_PLUGINS_DIR               = "SysPlugins";
const XRE_SYS_LOCAL_EXTENSION_PARENT_DIR  = "XRESysLExtPD";
const XRE_SYS_SHARE_EXTENSION_PARENT_DIR  = "XRESysSExtPD";
const XRE_EXTENSIONS_DIR_LIST             = "XREExtDL";

addLogger(this, "update-notifier");

function UpdateNotifier() {
  LOG("Starting");
  this.wrappedJSObject = this;

  var listeners = [];
  var watches = {};
  var timers = {};
  var dispatched = [];
  var plugins = {};

  /**
   * Add a listener to receive update notifications
   * Calling this inside a notification may produce undefined results
   *
   * @param  aListener
   *         The listener object
   */
  this.addListener = function UN_addListener(aListener) {
    if (!aListener) {
      throw new Error("A listener must be specified");
    }

    if (typeof(aListener) != "object") {
      throw new Error("Invalid listener");
    }

    if (listeners.indexOf(aListener) != -1) {
      throw new Error("Trying to register an observer more than once");
    }

    listeners.push(aListener);

    for (let event in dispatched) {
      try {
        if (event in aListener) {
          aListener[event].apply(aListener, dispatched[event]);
        }
      } catch(e) {
        ERROR("Listener threw", e);
      }
    }
  };

  /**
   * Remove a listener and don't receive any more notifications.
   * Calling this inside a notification may produce undefined results
   *
   * @param  aListener
   *         The listener to remove
   */
  this.removeListener = function UN_removeListener(aListener) {
    let index = listeners.indexOf(aListener);
    if (index == -1) {
      throw new Error("Trying to remove an observer that was never registered");
    }

    listeners.splice(index, 1);
  };

  function dispatchNotification(aEvent) {
    if (dispatched.indexOf(aEvent) != -1) {
      return;
    }

    dispatched.push(aEvent);

    listeners.forEach(function(listener) {
      try {
        if (aEvent in listener) {
          listener[aEvent].apply(listener);
        }
      } catch(e) {
        ERROR("Listener threw", e);
      }
    });
  }

  function handleAppUpdated() {
    LOG("Application change detected");
    dispatchNotification("onAppUpdated");
  }

  function handlePluginsChanged() {
    LOG("Plugin change detected");
    let ph = Cc["@mozilla.org/plugin/host;1"].getService(Ci.nsIPluginHost);

    try {
      ph.reloadPlugins(false);
    } catch(e) {
      LOG("nsIPluginHost.reloadPlugins failed. No changes?");
      return;
    }

    // Here's how this works:
    // - We iterate over the updated list of plugin tags from the plugin host
    // - For each plugin tag, we check if we have old data for that plugin,
    //   looking up by path
    // - We then pass over and new plugin data that hasn't been paired with
    //   old data, and see if it matches any old data by name. This is to handle
    //   plugin upgrades where the install path changes. We do this in
    //   a separate pass in case there is more than one plugin with the same
    //   name. In this case, we can still get a match if only one of those
    //   is updated.
    // - Obviously, if a plugin update changes its path and name, we detect
    //   that as a new install
    // - For each plugin, we compare the newest mtime with the recorded mtime
    //   from the old plugin data (if it exists). If the old data exists and the
    //   mtime is different, then the plugin was updated
    let newPlugins = {};
    ph.getPluginTags().forEach(function(pluginTag) {
      LOG("Processing plugin \"" + pluginTag.name + "\" at " +
          pluginTag.fullpath);

      if (pluginTag.fullpath in plugins) {
        var oldPluginData = plugins[pluginTag.fullpath];
        delete plugins[pluginTag.fullpath];
      }

      if (oldPluginData) {
        LOG("Found old data for plugin with the same path (\"" +
            oldPluginData.name + "\", lastModifiedTime " +
            oldPluginData.lastModifiedTime + ")");
      }

      if (!(pluginTag.fullpath in newPlugins)) {
        let plugin = {
          "pluginData": {
            "path": pluginTag.fullpath,
            "name": pluginTag.name
          },
          "oldPluginData": oldPluginData
        };

        newPlugins[pluginTag.fullpath] = plugin;

        try {
          plugin.pluginData.lastModifiedTime = (new FileUtils.File(pluginTag.fullpath))
                                                             .lastModifiedTime;
          LOG("Updated lastModifiedTime: " + plugin.pluginData
                                                   .lastModifiedTime);
        } catch(e) {
          LOG("Plugin at path " + pluginTag.fullpath + " was not found");
          delete newPlugins[pluginTag.fullpath];
        }
      }
    });

    let pluginPathsByName = {};
    for (let path in plugins) {
      pluginPathsByName[plugins[path].name] = path;
    }

    // Try to match new data to old data by name, for plugins
    // where the install path changed
    for each (let plugin in newPlugins) {
      if (plugin.oldPluginData) {
        continue;
      }

      if (plugin.pluginData.name in pluginPathsByName) {
        let path = pluginPathsByName[plugin.pluginData.name];
        plugin.oldPluginData = plugins[path];
        delete plugins[path];

        LOG("Found old data for plugin \"" + plugin.pluginData.name +
            "\". Old path: " + path + ", old lastModifiedTime: " +
            plugin.oldPluginData.lastModifiedTime + ", new path: " +
            plugin.pluginData.path);
      }
    }

    Object.keys(plugins).forEach(function(path) {
      LOG("Plugin \"" + plugins[path].name + "\" at " +
          plugins[path].path + " was removed");
      delete plugins[path];
    });

    let foundUpdated = false;
    for each (let plugin in newPlugins) {
      if (plugin.oldPluginData &&
          plugin.oldPluginData.lastModifiedTime !=
          plugin.pluginData.lastModifiedTime) {
        LOG("Plugin \"" + plugin.pluginData.name + "\" at " +
            plugin.pluginData.path + " was updated");
        foundUpdated = true;     
      } else if (!plugin.oldPluginData) {
        LOG("Plugin \"" + plugin.pluginData.name + "\" at " +
            plugin.pluginData.path + " was newly installed");
      } else {
        LOG("Plugin \"" + plugin.pluginData.name + "\" at " +
            plugin.pluginData.path + " is unchanged");
      }

      plugins[plugin.pluginData.path] = plugin.pluginData;
    }

    if (foundUpdated) {
      dispatchNotification("onPluginsUpdated");
    }
  }

  function handleAddonsChanged() {
    // We don't know how to handle addon changes yet
  }

  function handler(aFile, aEventType, aIsDir) {
    if (aEventType == gio.GFileMonitorEventEnums.G_FILE_MONITOR_EVENT_CHANGED ||
        aEventType == gio.GFileMonitorEventEnums.G_FILE_MONITOR_EVENT_DELETED ||
        aEventType == gio.GFileMonitorEventEnums.G_FILE_MONITOR_EVENT_CREATED ||
        aEventType == gio.GFileMonitorEventEnums.G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED) {
      let path = gio.g_file_get_path(aFile);
      let type;
      if (!(path in watches) && aIsDir) {
        type = watches[(new FileUtils.File(path)).parent.path].type;
      } else {
        type = watches[path].type;
      }

      if (!type) {
        ERROR("Got event for a path not in our watch list: " + path);
        return;
      }

      // Wait for the location to settle before dispatching the notification
      if (!(type in timers)) {
        timers[type] = Cc["@mozilla.org/timer;1"].createInstance(Ci.nsITimer);
        timers[type].initWithCallback(function() {
          delete timers[type];
          ((new Object({"app"    : handleAppUpdated,
                        "plugins": handlePluginsChanged,
                        "addons" : handleAddonsChanged}))[type])();
        }, 10000, Ci.nsITimer.TYPE_ONE_SHOT);
      } else {
        timers[type].delay = 10000;
      }
    }
  }

  function watchOneFile(aFile, aType) {
    if (!aFile.exists() || !aFile.isFile()) {
      return;
    }

    aFile.normalize();

    let file = gio.g_file_new_for_path(aFile.path);
    if (file.isNull()) {
      ERROR("OOM: Failed to get GFile for path " + aFile.path);
      return;
    }

    let mon = gio.g_file_monitor_file(file,
                                      gio.GFileMonitorFlagsFlags.G_FILE_MONITOR_NONE,
                                      null, null);
    gobject.g_object_unref(file);
    if (mon.isNull()) {
      ERROR("Failed to create monitor for path " + aFile.path);
      return;
    }

    let id = gobject.g_signal_connect(mon, "changed",
                                      function(aMonitor, aFile, aOtherFile,
                                               aEventType) {
      handler(aFile, aEventType, false);    
    });
    watches[aFile.path] = {monitor: mon, sig_id: id, type: aType};
    LOG("Watching " + aFile.path);
  }

  function watchOneDir(aKey, aPaths, aType) {
    let nsifile;
    if (aKey instanceof Ci.nsIFile) {
      nsifile = aKey;
      aPaths.forEach(function(path) {
        nsifile.append(path);
      });
    } else {
      nsifile = FileUtils.getDir(aKey, aPaths);
    }
    if (!nsifile.exists() || !nsifile.isDirectory()) {
      return;
    }

    nsifile.normalize();

    if (nsifile.path in watches) {
      return;
    }

    let file = gio.g_file_new_for_path(nsifile.path);
    if (file.isNull()) {
      ERROR("OOM: Failed to get GFile for path " + nsifile.path);
      return;
    }

    let mon = gio.g_file_monitor_directory(file,
                                           gio.GFileMonitorFlagsFlags.G_FILE_MONITOR_NONE,
                                           null, null);
    gobject.g_object_unref(file);
    if (mon.isNull()) {
      ERROR("Failed to create monitor for path " + nsifile.path);
      return;
    }

    let id = gobject.g_signal_connect(mon, "changed",
                                      function(aMonitor, aFile, aOtherFile,
                                               aEventType) {
      handler(aFile, aEventType, true);
    });
    watches[nsifile.path] = {monitor: mon, sig_id: id, type: aType};
    LOG("Watching " + nsifile.path);
  }

  function watchList(aKey, aType, aFilterList) {
    let e = Services.dirsvc.get(aKey, Ci.nsISimpleEnumerator);
    let files = [];
    while (e.hasMoreElements()) {
      files.push(e.getNext().QueryInterface(Ci.nsIFile));
    }

    if (aFilterList) {
      files = files.filter(function(aValue, aIndex, aArray) {
        if (aFilterList.indexOf(aValue.parent.path) != -1) {
          return true;
        }

        return false;
      });
    }

    files.forEach(function(file) {
      if (!(file.exists())) {
        return;
      }

      if (file.isDirectory()) {
        watchOneDir(file, [], aType);
      } else {
        watchOneFile(file, aType);
      }
    });
  }

  Services.obs.addObserver(function(aSubject, aTopic, aData) {
    if (aSubject == "xpcom-shutdown") {
      Services.obs.removeObserver(arguments.callee, "xpcom-shutdown", false);
      for (let path in watches) {
        gobject.g_signal_handler_disconnect(watches[path].monitor,
                                            watches[path].sig_id);
        gobject.g_object_unref(watches[path].monitor);
      }
    }
  }, "xpcom-shutdown", false);

  var idle = Cc["@mozilla.org/timer;1"].createInstance(Ci.nsITimer);
  idle.initWithCallback(function() {
    let kungFuDeathGrip = idle;
    watchOneDir(NS_XPCOM_CURRENT_PROCESS_DIR, [], "app");
    watchOneDir(NS_GRE_DIR, [], "app");
    watchOneDir(NS_APP_PLUGINS_DIR, [], "plugins");
    watchOneDir(NS_SYSTEM_PLUGINS_DIR, [], "plugins");
    //watchOneDir(XRE_SYS_LOCAL_EXTENSION_PARENT_DIR,
    //            [Services.appinfo.ID], "addons");
    //watchOneDir(XRE_SYS_SHARE_EXTENSION_PARENT_DIR,
    //            [Services.appinfo.ID], "addons");
    //watchOneDir(NS_XPCOM_CURRENT_PROCESS_DIR,
    //            ["extensions"], "addons");
    //watchList(XRE_EXTENSIONS_DIR_LIST, "addons",
    //          [FileUtils.getDir(XRE_SYS_LOCAL_EXTENSION_PARENT_DIR,
    //                            [Services.appinfo.ID]).path,
    //           FileUtils.getDir(XRE_SYS_SHARE_EXTENSION_PARENT_DIR,
    //                            [Services.appinfo.ID]).path,
    //           FileUtils.getDir(NS_XPCOM_CURRENT_PROCESS_DIR,
    //                            ["extensions"]).path]);

    idle.initWithCallback(function() {
      let kungFuDeathGrip = idle;
      // Would use the AddonManager here, but it doesn't expose full paths.
      // We use a delay here to avoid starting the plugin host at startup
      // if it isn't otherwise needed
      LOG("Initializing plugin data");
      Cc["@mozilla.org/plugin/host;1"].getService(Ci.nsIPluginHost)
                                      .getPluginTags()
                                      .forEach(function(aPluginTag) {
        try {
          plugins[aPluginTag.fullpath] = {
            "path": aPluginTag.fullpath,
            "name": aPluginTag.name,
            "lastModifiedTime": (new FileUtils.File(aPluginTag.fullpath))
                                              .lastModifiedTime
          };
          LOG("Found plugin \"" + aPluginTag.name + "\" at " +
              aPluginTag.fullpath + ", lastModifiedTime: " +
              plugins[aPluginTag.fullpath].lastModifiedTime);
        } catch(e) {
          delete plugins[aPluginTag.fullpath];
          LOG("Plugin \"" + aPluginTag.name + "\" doesn't exist at " +
              aPluginTag.fullpath);
        }
      });
    }, 8000, Ci.nsITimer.TYPE_ONE_SHOT);

  }, 0, Ci.nsITimer.TYPE_ONE_SHOT);
}

UpdateNotifier.prototype = {
  classDescription: "Update Notifier",
  classID: Components.ID("{799a4700-2085-40e8-853a-05dcf8e95e41}"),
  contractID: "@ubuntu.com/update-notifier;1",
  QueryInterface: XPCOMUtils.generateQI([Ci.nsISupports])
};

const NSGetFactory = XPCOMUtils.generateNSGetFactory([UpdateNotifier]);
