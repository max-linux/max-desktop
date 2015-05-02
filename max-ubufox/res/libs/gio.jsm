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

const { utils: Cu } = Components;

var EXPORTED_SYMBOLS = [ "gio" ];

const GIO_LIBNAME = "gio-2.0";
const GIO_ABIS = [ 0 ];

Cu.import("resource://gre/modules/ctypes.jsm");
Cu.import("resource://ubufox/modules/utils.jsm");
Cu.import("resource://ubufox/libs/gobject.jsm");
Cu.import("resource://ubufox/libs/glib.jsm");

function getString(aWrappee) {
  let res = aWrappee.apply(null, Array.prototype.slice.call(arguments, 1));
  return res.isNull() ? null : res.readString();
}

function getOwnedString(aWrappee) {
  let res;
  try {
    res = aWrappee.apply(null, Array.prototype.slice.call(arguments, 1));
    return res.isNull() ? null : res.readString();
  } finally { glib.g_free(res); }
}

function gio_defines(lib) {
  // Enums
  CTypesUtils.defineEnums(this, "GBusType", -1, [
    "G_BUS_TYPE_STARTER",
    "G_BUS_TYPE_NONE",
    "G_BUS_TYPE_SYSTEM",
    "G_BUS_TYPE_SESSION"
  ]);

  CTypesUtils.defineFlags(this, "GDBusProxyFlags", 0, [
    "G_DBUS_PROXY_FLAGS_NONE",
    "G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES",
    "G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS",
    "G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START"
  ]);

  CTypesUtils.defineFlags(this, "GDBusCallFlags", 0, [
    "G_DBUS_CALL_FLAGS_NONE",
    "G_DBUS_CALL_FLAGS_NO_AUTO_START"
  ]);

  CTypesUtils.defineFlags(this, "GFileMonitorFlags", 0, [
    "G_FILE_MONITOR_NONE",
    "G_FILE_MONITOR_WATCH_MOUNTS",
    "G_FILE_MONITOR_SEND_MOVED"
  ]);

  CTypesUtils.defineEnums(this, "GFileMonitorEvent", 0, [
    "G_FILE_MONITOR_EVENT_CHANGED",
    "G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT",
    "G_FILE_MONITOR_EVENT_DELETED",
    "G_FILE_MONITOR_EVENT_CREATED",
    "G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED",
    "G_FILE_MONITOR_EVENT_PRE_UNMOUNT",
    "G_FILE_MONITOR_EVENT_UNMOUNTED",
    "G_FILE_MONITOR_EVENT_MOVED"
  ]);

  // GIOErrorEnum
  CTypesUtils.defineSimple(this, "G_IO_ERROR_DBUS_ERROR", 36);

  // Types
  CTypesUtils.defineSimple(this, "GDBusInterfaceInfo",
                           ctypes.StructType("GDBusInterfaceInfo"));
  CTypesUtils.defineSimple(this, "GCancellable",
                           ctypes.StructType("GCancellable"));
  CTypesUtils.defineSimple(this, "GAsyncResult",
                           ctypes.StructType("GAsyncResult"));
  CTypesUtils.defineSimple(this, "GDBusProxy",
                           ctypes.StructType("GDBusProxy"));
  CTypesUtils.defineSimple(this, "GFile",
                           ctypes.StructType("GFile"));
  CTypesUtils.defineSimple(this, "GFileMonitor",
                           ctypes.StructType("GFileMonitor"));

  // Templates
  CTypesUtils.defineSimple(this, "GAsyncReadyCallback",
                           ctypes.FunctionType(ctypes.default_abi,
                                               ctypes.void_t,
                                               [gobject.GObject.ptr,
                                                this.GAsyncResult.ptr,
                                                glib.gpointer]).ptr);

  // Functions
  lib.lazy_bind_with_wrapper("g_dbus_proxy_new_for_bus", function(aWrappee,
                                                                  aBusType,
                                                                  aFlags,
                                                                  aInterfaceInfo,
                                                                  aName, aPath,
                                                                  aInterface,
                                                                  aCancellable,
                                                                  aCallback) {
    var ccw = CTypesUtils.wrapCallback(aCallback,
                                       {type: gio.GAsyncReadyCallback,
                                        root: true, singleshot: true});

    try {
      aWrappee(aBusType, aFlags, aInterfaceInfo, aName, aPath, aInterface,
               aCancellable, ccw, null);
    } catch(e) {
      CTypesUtils.unrootCallback(aCallback);
      throw e;
    }
  }, ctypes.void_t, [this.GBusType, this.GDBusProxyFlags,
                     this.GDBusInterfaceInfo.ptr, glib.gchar.ptr,
                     glib.gchar.ptr, glib.gchar.ptr, this.GCancellable.ptr,
                     this.GAsyncReadyCallback, glib.gpointer]);
  lib.lazy_bind("g_dbus_proxy_new_for_bus_finish", this.GDBusProxy.ptr,
                [this.GAsyncResult.ptr, glib.GError.ptr.ptr]);
  lib.lazy_bind_with_wrapper("g_dbus_proxy_call", function(aWrappee, aProxy,
                                                           aMethod, aParams,
                                                           aFlags, aTimeout,
                                                           aCancellable,
                                                           aCallback) {
    var ccw = CTypesUtils.wrapCallback(aCallback,
                                       {type: gio.GAsyncReadyCallback,
                                        root: true, singleshot: true});

    try {
      aWrappee(aProxy, aMethod, aParams, aFlags, aTimeout, aCancellable,
               ccw, null);
    } catch(e) {
      CTypesUtils.unrootCallback(aCallback);
      throw e;
    }
  }, ctypes.void_t, [this.GDBusProxy.ptr, glib.gchar.ptr, glib.GVariant.ptr,
                     this.GDBusCallFlags, glib.gint, this.GCancellable.ptr,
                     this.GAsyncReadyCallback, glib.gpointer]);
  lib.lazy_bind("g_dbus_proxy_call_finish", glib.GVariant.ptr,
                [this.GDBusProxy.ptr, this.GAsyncResult.ptr,
                 glib.GError.ptr.ptr]);
  lib.lazy_bind_with_wrapper("g_dbus_proxy_get_name", getString,
                             glib.gchar.ptr, [this.GDBusProxy.ptr]);
  lib.lazy_bind_with_wrapper("g_dbus_proxy_get_interface_name", getString,
                             glib.gchar.ptr, [this.GDBusProxy.ptr]);
  lib.lazy_bind_with_wrapper("g_dbus_proxy_get_object_path", getString,
                             glib.gchar.ptr, [this.GDBusProxy.ptr]);
  lib.lazy_bind("g_io_error_quark", glib.GQuark);
  lib.lazy_bind_with_wrapper("g_dbus_error_get_remote_error", getOwnedString,
                             glib.gchar.ptr, [glib.GError.ptr]);
  lib.lazy_bind_with_wrapper("g_dbus_proxy_get_name_owner", getOwnedString,
                             glib.gchar.ptr, [this.GDBusProxy.ptr]);
  lib.lazy_bind("g_file_new_for_path", this.GFile.ptr, [glib.gchar.ptr]);
  lib.lazy_bind("g_file_monitor_file", this.GFileMonitor.ptr, [this.GFile.ptr,
                this.GFileMonitorFlags, this.GCancellable.ptr,
                glib.GError.ptr]);
  lib.lazy_bind("g_file_monitor_directory", this.GFileMonitor.ptr,
                [this.GFile.ptr, this.GFileMonitorFlags,
                 this.GCancellable.ptr, glib.GError.ptr]);
  lib.lazy_bind_with_wrapper("g_file_get_path", getOwnedString,
                             glib.gchar.ptr, [this.GFile.ptr]);

  CTypesUtils.defineSimple(this, "G_IO_ERROR", this.g_io_error_quark());

  gobject.createSignal(this.GDBusProxy, "g-signal", ctypes.void_t,
                       [this.GDBusProxy.ptr, glib.gchar.ptr,
                        glib.gchar.ptr, glib.GVariant.ptr,
                        glib.gpointer]);
  gobject.createSignal(this.GFileMonitor, "changed", ctypes.void_t,
                       [this.GFileMonitor.ptr, this.GFile.ptr,
                        this.GFile.ptr, this.GFileMonitorEvent,
                        glib.gpointer]);
}

var gio = new CTypesUtils.newLibrary(GIO_LIBNAME, GIO_ABIS, gio_defines);
