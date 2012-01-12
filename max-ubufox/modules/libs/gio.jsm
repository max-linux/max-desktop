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

var EXPORTED_SYMBOLS = [ "gio" ];

const GIO_LIBNAME = "gio-2.0";
const GIO_ABIS = [ 0 ];

const Cu = Components.utils;

Cu.import("resource://gre/modules/ctypes.jsm");
Cu.import("resource://ubufox/libs/ctypes-utils.jsm");
Cu.import("resource://ubufox/libs/gobject.jsm");
Cu.import("resource://ubufox/libs/glib.jsm");

["LOG", "WARN", "ERROR"].forEach(function(aName) {
  this.__defineGetter__(aName, function() {
    Components.utils.import("resource://gre/modules/AddonLogging.jsm");

    LogManager.getLogger("ubufox.gio", this);
    return this[aName];
  });
}, this);

var asyncCbHandlers = {};
var asyncCbIdSerial = 0;

function AsyncCbData(cb, ccb) {
  this.cb = cb,
  this.ccb = ccb;
}

function newAsyncCbId() {
  while (asyncCbHandlers[++asyncCbIdSerial]) {}
  return asyncCbIdSerial;
}

function gio_defines(lib) {
  // Enums
  // GBusType
  this.GBusType = ctypes.int;
  this.G_BUS_TYPE_STARTER = -1;
  this.G_BUS_TYPE_NONE = 0;
  this.G_BUS_TYPE_SYSTEM = 1;
  this.G_BUS_TYPE_SESSION = 2;

  // GDBusProxyFlags
  this.GDBusProxyFlags = ctypes.int;
  this.G_DBUS_PROXY_FLAGS_NONE = 0;
  this.G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES = (1<<0);
  this.G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS = (1<<1);
  this.G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START = (1<<2);

  // GDBusCallFlags
  this.GDBusCallFlags = ctypes.int;
  this.G_DBUS_CALL_FLAGS_NONE = 0;
  this.G_DBUS_CALL_FLAGS_NO_AUTO_START = (1<<0);

  // GIOErrorEnum
  this.G_IO_ERROR_DBUS_ERROR = 36;

  // Types
  this.GDBusInterfaceInfo = ctypes.StructType("GDBusInterfaceInfo");
  this.GCancellable = ctypes.StructType("GCancellable");
  this.GAsyncResult = ctypes.StructType("GAsyncResult");
  this.GDBusProxy = ctypes.StructType("GDBusProxy");

  // Templates
  this.GAsyncReadyCallback = ctypes.FunctionType(ctypes.default_abi,
                                                 ctypes.void_t,
                                                 [gobject.GObject.ptr,
                                                  this.GAsyncResult.ptr,
                                                  glib.gpointer]).ptr;

  // Functions
  lib.lazy_bind("g_dbus_proxy_new_for_bus", ctypes.void_t, this.GBusType,
                this.GDBusProxyFlags, this.GDBusInterfaceInfo.ptr,
                glib.gchar.ptr, glib.gchar.ptr, glib.gchar.ptr,
                this.GCancellable.ptr, this.GAsyncReadyCallback, glib.gpointer);
  lib.lazy_bind("g_dbus_proxy_new_for_bus_finish", this.GDBusProxy.ptr,
                this.GAsyncResult.ptr, glib.GError.ptr.ptr);
  lib.lazy_bind("g_dbus_proxy_call", ctypes.void_t, this.GDBusProxy.ptr,
                glib.gchar.ptr, glib.GVariant.ptr, this.GDBusCallFlags,
                glib.gint, this.GCancellable.ptr, this.GAsyncReadyCallback,
                glib.gpointer);
  lib.lazy_bind("g_dbus_proxy_call_finish", glib.GVariant.ptr,
                this.GDBusProxy.ptr, this.GAsyncResult.ptr, glib.GError.ptr.ptr);
  lib.lazy_bind("g_io_error_quark", glib.GQuark);
  lib.lazy_bind("g_dbus_error_get_remote_error", glib.gchar.ptr, glib.GError.ptr);

  // Helpers to work around jsctypes limitations
  this.GDbusProxyNewForBus = function(aType, aFlags, aInfo, aName,
                                      aPath, aInterface, aCancellable,
                                      aCallback) {
    let cb = function(aObject, aResult, aData) {
      try {
        aCallback(aObject, aResult);
      } catch(e) {
        Cu.reportError(e);
      }
      delete asyncCbHandlers[id];
    };
    let ccb = gio.GAsyncReadyCallback(cb);

    var id = newAsyncCbId();
    asyncCbHandlers[id] = new AsyncCbData(cb, ccb);

    gio.g_dbus_proxy_new_for_bus(aType, aFlags, aInfo, aName, aPath,
                                 aInterface, aCancellable, ccb, null);
  };

  this.GDbusProxyCall = function(aProxy, aMethod, aParams, aFlags, aTimeout,
                                 aCancellable, aCallback) {
    let cb = function(aObject, aResult, aData) {
      try {
        aCallback(aObject, aResult);
      } catch(e) {
        Cu.reportError(e);
      }
      delete asyncCbHandlers[id];
    };
    let ccb = gio.GAsyncReadyCallback(cb);

    var id = newAsyncCbId();
    asyncCbHandlers[id] = new AsyncCbData(cb, ccb);

    gio.g_dbus_proxy_call(aProxy, aMethod, aParams, aFlags, aTimeout,
                          aCancellable, ccb, null);
  };

  this.G_IO_ERROR = this.g_io_error_quark();
}

if (!gio || !gio.available()) {
  var gio = new ctypes_library(GIO_LIBNAME, GIO_ABIS, gio_defines);
}
