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

var EXPORTED_SYMBOLS = [ "gobject" ];

const GOBJECT_LIBNAME = "gobject-2.0";
const GOBJECT_ABIS = [ 0 ];

const Cu = Components.utils;

Cu.import("resource://gre/modules/ctypes.jsm");
Cu.import("resource://ubufox/libs/ctypes-utils.jsm");
Cu.import("resource://ubufox/libs/glib.jsm");

["LOG", "WARN", "ERROR"].forEach(function(aName) {
  this.__defineGetter__(aName, function() {
    Components.utils.import("resource://gre/modules/AddonLogging.jsm");

    LogManager.getLogger("ubufox.gobject", this);
    return this[aName];
  });
}, this);

var signalHandlers = {};

function GSignalCbData(cb, ccb) {
  this.cb = cb,
  this.ccb = ccb;
}

function gobject_defines(lib) {
  // Enums
  // GConnectFlags
  this.GConnectFlags = ctypes.int;
  this.G_CONNECT_AFTER = (1<<0);
  this.G_CONNECT_SWAPPED = (1<<1);

  // Types
  this.GObject = ctypes.StructType("GObject");
  this.GClosure = ctypes.StructType("GClosure");
  this.GCallback = ctypes.voidptr_t;

  // Templates
  this.GClosureNotify = ctypes.FunctionType(ctypes.default_abi, ctypes.void_t,
                                            [glib.gpointer, this.GClosure.ptr]).ptr;

  // Functions
  lib.lazy_bind("g_object_unref", ctypes.void_t, glib.gpointer);
  lib.lazy_bind("g_signal_connect_data", glib.gulong, glib.gpointer,
                glib.gchar.ptr, this.GCallback, glib.gpointer,
                this.GClosureNotify, this.GConnectFlags);
  lib.lazy_bind("g_signal_handler_disconnect", ctypes.void_t, glib.gpointer,
                glib.gulong);

  // Helpers
  this.g_signal_connect = function(aInstance, aSignal, aHandler, aData) {
    return gobject.g_signal_connect_data(aInstance, aSignal, aHandler, aData, null, 0);
  };

  this.GSignalConnect = function(aInstance, aSignal, aHandler, aRetType, aArgTypes) {
    let cb = function() {
      try {
        LOG("Signal arguments length: " + arguments.length);
        return aHandler.apply(null, Array.prototype.slice.call(arguments, 0));
      } catch(e) {
        Cu.reportError(e);
      }
    }
    let ccb = ctypes.FunctionType(ctypes.default_abi, aRetType, aArgTypes).ptr(cb);

    let id = gobject.g_signal_connect(aInstance, aSignal, ccb, null);
    signalHandlers[id] = new GSignalCbData(cb, ccb);

    return id;
  };

  this.GSignalHandlerDisconnect = function(aInstance, aId) {
    gobject.g_signal_handler_disconnect(aInstance, aId);
    delete signalHandlers[aId];
  }
}

if (!gobject || !gobject.available()) {
  var gobject = new ctypes_library(GOBJECT_LIBNAME, GOBJECT_ABIS, gobject_defines);
}
