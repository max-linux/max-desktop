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
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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
const Cu = Components.utils;
const Ci = Components.interfaces;

const nsITimer = Ci.nsITimer;

var EXPORTED_SYMBOLS = [ ];

Cu.import("resource://ubufox/PluginFinder.jsm");
Cu.import("resource://ubufox/libs/gio.jsm");
Cu.import("resource://ubufox/libs/gobject.jsm");
Cu.import("resource://ubufox/libs/glib.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/ctypes.jsm");

["LOG", "WARN", "ERROR"].forEach(function(aName) {
  this.__defineGetter__(aName, function() {
    Components.utils.import("resource://gre/modules/AddonLogging.jsm");

    LogManager.getLogger("ubufox.plugininstaller.apt", this);
    return this[aName];
  });
}, this);

function AptInstallContext(aListener, aPkgName) {
  this.listener = aListener;
  this.pkgname = aPkgName;
  this.running = false;
  this.proxy = null;
  this.transaction = null;
  this.error_code = null;
  this.error_details = null;
}

AptInstallContext.prototype = {
  destroy: function AIC_destroy() {
    if (this.proxy && !this.proxy.isNull()) {
      gobject.g_object_unref(this.proxy);
      this.proxy = null;
    }

    if (this.transaction && !this.transaction.isNull()) {
      gobject.GSignalHandlerDisconnect(this.transaction, this.transactionSigID);
      gobject.g_object_unref(this.transaction);
      this.transaction = null;
    }
  }
};

var PluginInstallerApt = {
  install: function PIA_install(aPluginInfo, aListener) {
    let pkgname = Services.io.newURI(aPluginInfo.location, null, null)
                                     .path.replace(/^([a-zA-Z0-9\-]*).*/, "$1");
    LOG("Starting apt install for " + pkgname);

    var ctxt = new AptInstallContext(aListener, pkgname);

    gio.GDbusProxyNewForBus(gio.G_BUS_TYPE_SYSTEM, gio.G_DBUS_PROXY_FLAGS_NONE,
                            null, "org.debian.apt", "/org/debian/apt",
                            "org.debian.apt", null, function(aObject, aResult) {
      PluginInstallerApt.proxyNewCb(aObject, aResult, ctxt);
    });
  },

  proxyNewCb: function PIA_proxyNewCb(aObject, aResult, aCtxt) {
    LOG("Got aptdaemon proxy for " + aCtxt.pkgname);
    let error = new glib.GError.ptr;
    aCtxt.proxy = gio.g_dbus_proxy_new_for_bus_finish(aResult, error.address());
    if (aCtxt.proxy.isNull() || !error.isNull()) {
      ERROR("Failed to get GDBusProxy for aptdaemon: " +
            error.contents.message.readString());
      this.fail(this.getString("plugininstaller.apt.error.unexpected"), aCtxt);
      return;
    }

    let builder = new glib.GVariantBuilder;
    glib.g_variant_builder_init(builder.address(), "a*");
    glib.g_variant_builder_add(builder.address(), "s",
                               ctypes.char.array()(aCtxt.pkgname).address());
    let inner = glib.g_variant_builder_end(builder.address());
    glib.g_variant_builder_clear(builder.address());

    glib.g_variant_builder_init(builder.address(), "r");
    glib.g_variant_builder_add_value(builder.address(), inner);
    let params = glib.g_variant_builder_end(builder.address());
    glib.g_variant_builder_clear(builder.address());

    gio.GDbusProxyCall(aCtxt.proxy, "InstallPackages", params,
                       gio.G_DBUS_CALL_FLAGS_NONE, -1, null, function(aObject,
                                                                      aResult) {
      PluginInstallerApt.installPkgsCb(aObject, aResult, aCtxt);
    });
  },

  installPkgsCb: function PIA_installPkgsCb(aObject, aResult, aCtxt) {
    LOG("Got response to InstallPackages() for " + aCtxt.pkgname);
    let error = new glib.GError.ptr;
    let res = gio.g_dbus_proxy_call_finish(ctypes.cast(aObject, gio.GDBusProxy.ptr),
                                           aResult, error.address());
    if (res.isNull() || !error.isNull()) {
      ERROR("InstallPackages() method failed: " + error.contents.message.readString());
      this.fail(this.getString("plugininstaller.apt.error.unexpected"), aCtxt);
      return;
    }

    let path = new ctypes.char.ptr;
    glib.g_variant_get(res, "(s)", path.address());

    gio.GDbusProxyNewForBus(gio.G_BUS_TYPE_SYSTEM, gio.G_DBUS_PROXY_FLAGS_NONE,
                            null, "org.debian.apt", path.readString(),
                            "org.debian.apt.transaction", null,
                            function(aObject, aResult) {
      PluginInstallerApt.transactionNewCb(aObject, aResult, aCtxt);
    });

    glib.g_variant_unref(res);
  },

  transactionNewCb: function PIA_transactionNewCb(aObject, aResult, aCtxt) {
    LOG("Got transaction proxy for " + aCtxt.pkgname);
    let error = new glib.GError.ptr;
    aCtxt.transaction = gio.g_dbus_proxy_new_for_bus_finish(aResult, error.address());
    if (aCtxt.transaction.isNull() || !error.isNull()) {
      ERROR("Failed to get GDBusProxy for transaction: " +
            error.contents.message.readString());
      this.fail(this.getString("plugininstaller.apt.error.unexpected"), aCtxt);
      return;
    }

    aCtxt.transactionSigID = gobject.GSignalConnect(aCtxt.transaction, "g-signal",
                                                    function(aProxy, aSender,
                                                             aSignal, aParams,
                                                             aData) {
      PluginInstallerApt.transactionSignalCb(aProxy, aSender, aSignal,
                                             aParams, aCtxt);
    }, ctypes.void_t, [gio.GDBusProxy.ptr, glib.gchar.ptr, glib.gchar.ptr,
                       glib.GVariant.ptr, glib.gpointer]);

    gio.GDbusProxyCall(aCtxt.transaction, "Run", null,
                       gio.G_DBUS_CALL_FLAGS_NONE, -1, null, function(aObject,
                                                                      aResult) {
      PluginInstallerApt.runCb(aObject, aResult, aCtxt);
    });
  },

  transactionSignalCb: function PIA_transactionSignalCb(aProxy, aSender,
                                                        aSignal, aParams, aCtxt) {
    let signal = aSignal.readString();
    LOG("Got signal " + signal + " for " + aCtxt.pkgname);
    if (signal == "PropertyChanged") {
      let namePtr = new ctypes.char.ptr;
      let variant = new glib.GVariant.ptr;
      glib.g_variant_get(aParams, "(sv)", namePtr.address(), variant.address());

      // FIXME: Desperately need some sanity checking in here

      let name = namePtr.readString();
      LOG("Property - " + name);
      if (name == "Progress") {
        if (!aCtxt.running) {
          return;
        }

        let progress = glib.g_variant_get_int32(variant);
        LOG("Progress = " + progress.toString());
        aCtxt.listener.onProgressChanged(progress);

      } else if (name == "ExitState") {
        let exitState = glib.g_variant_get_string(variant, null).readString();
        LOG("ExitState = " + exitState);
        if (exitState == "exit-success") {
          this.finish(aCtxt);
        } else {
          this.fail(this.getErrorMessage(aCtxt), aCtxt);
        }
      } else if (name == "Status") {
        let status = glib.g_variant_get_string(variant, null).readString();
        LOG("Status = " + status);
        if (status == "status-running") {
          aCtxt.running = true;
          aCtxt.listener.onInstallStarted();
        }
      } else if (name == "Error") {
        aCtxt.error_code = new ctypes.char.ptr;
        aCtxt.error_details = new ctypes.char.ptr;
        glib.g_variant_get(variant, "(ss)", aCtxt.error_code.address(),
                           aCtxt.error_details.address());
        ERROR("Error: " + aCtxt.error_code.readString() + " (" +
              aCtxt.error_details.readString() + ")");
      }
    }
  },

  runCb: function PIA_runCb(aObject, aResult, aCtxt) {
    LOG("Got response to Run() for " + aCtxt.pkgname);
    let error = new glib.GError.ptr;
    let res = gio.g_dbus_proxy_call_finish(ctypes.cast(aObject,
                                                       gio.GDBusProxy.ptr),
                                           aResult, error.address());
    if (!error.isNull()) {
      ERROR("Error running transaction: " + error.contents.message.readString()
            + " (domain: " + error.contents.domain + ", code: "
            + error.contents.code + ")");
      let msg = null;
      if (aCtxt.error_code) {
        msg = this.getErrorMessage(aCtxt);
      } else if (error.contents.domain == gio.G_IO_ERROR &&
                 error.contents.code == gio.G_IO_ERROR_DBUS_ERROR) {
        let rerr = gio.g_dbus_error_get_remote_error(error).readString();
        if (rerr == "org.freedesktop.PolicyKit.Error.NotAuthorized") {
          msg = this.getString("plugininstaller.apt.error.unauthorized");
        }
      }

      if (!msg) {
        msg = this.getString("plugininstaller.apt.error.unexpected");
      }

      this.fail(msg, aCtxt);
    }
  },

  finish: function PIA_finish(aCtxt) {
    aCtxt.listener.onInstallFinished();
    aCtxt.destroy();
  },

  fail: function PIA_fail(aError, aCtxt) {
    aCtxt.listener.onInstallFailed(aError);
    aCtxt.destroy();
  },

  getErrorMessage: function PIA_getErrorMessage(aCtxt) {
    let msg = null;
    switch (aCtxt.error_code.readString()) {
    case "error-package-download-failed":
      msg = this.getString("plugininstaller.apt.error.download_failed");
      break;

    case "error-dep-resolution-failed":
      msg = this.getString("plugininstaller.apt.error.dependencies");
      break;

    case "error-no-lock":
      msg = this.getString("plugininstaller.apt.error.lock");
      break;

    case "error-no-package":
      msg = this.getString("plugininstaller.apt.error.does_not_exist");
      break;

    case "error-package-already-installed":
      msg = this.getString("plugininstaller.apt.error.already_installed");
      break;

    case "error-package-manager-failed":
      msg = this.getString("plugininstaller.apt.error.failed_install");
      break;

    case "error-cache-broken":
      msg = this.getString("plugininstaller.apt.error.broken_cache");
      break;

    case "error-package-unauthenticated":
      msg = this.getString("plugininstaller.apt.error.unauthenticated");
      break;

    case "error-incomplete-install":
      msg = this.getString("plugininstaller.apt.error.incomplete_install");
      break;

    case "error-unreadable-package-file":
      msg = this.getString("plugininstaller.apt.error.open_failed");
      break;

    case "error-invalid-package-file":
      msg = this.getString("plugininstaller.apt.error.policy_violation");
      break;

    default:
      msg = this.getString("plugininstaller.apt.error.unexpected");
      break;
    }

    return msg;
  },

  getString: function PIA_getString(aName) {
    if (!this._strbundle) {
      this._strbundle =
        Services.strings.createBundle("chrome://ubufox/locale/plugins.properties");
    }

    return this._strbundle.GetStringFromName(aName);
  }
};

var PluginInstallerIface = {
  types: "apt",

  install: function PII_install(aPluginInfo, aListener) {
    return PluginInstallerApt.install(aPluginInfo, aListener);
  },

  shutdown: function PII_shutdown() {
    PluginFinder.unregisterInstallHandler(this);
  }
};

PluginFinder.registerInstallHandler(PluginInstallerIface);
