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

var EXPORTED_SYMBOLS = [ "gobject" ];

const GOBJECT_LIBNAME = "gobject-2.0";
const GOBJECT_ABIS = [ 0 ];

Cu.import("resource://gre/modules/ctypes.jsm");
Cu.import("resource://ubufox/modules/utils.jsm");
Cu.import("resource://ubufox/libs/glib.jsm");

var gSignalHandlers = {};
var gSignals = {};

function gobject_defines(lib) {
  // Enums
  CTypesUtils.defineFlags(this, "GConnectFlags", 0, [
    "G_CONNECT_AFTER",
    "G_CONNECT_SWAPPED"
  ]);

  // Types
  CTypesUtils.defineSimple(this, "GObject", ctypes.StructType("GObject"));
  CTypesUtils.defineSimple(this, "GClosure", ctypes.StructType("GClosure"));
  CTypesUtils.defineSimple(this, "GCallback", ctypes.voidptr_t);

  // Templates
  CTypesUtils.defineSimple(this, "GClosureNotify",
                           ctypes.FunctionType(ctypes.default_abi, ctypes.void_t,
                                               [glib.gpointer,
                                                this.GClosure.ptr]).ptr);

  // Functions
  lib.lazy_bind("g_object_ref", glib.gpointer, [glib.gpointer]);
  lib.lazy_bind("g_object_unref", ctypes.void_t, [glib.gpointer]);
  lib.lazy_bind_with_wrapper("g_signal_connect", function(aWrappee, aInstance,
                                                          aSignal, aHandler) {
    if (!aInstance.constructor.targetType) {
      throw Error("Instance must be a pointer type");
    }

    if (!(aInstance.constructor.targetType in gSignals) ||
        !(aSignal in gSignals[aInstance.constructor.targetType])) {
      throw Error("No prototype available for signal");
    }

    var ccw = CTypesUtils.wrapCallback(aHandler,
                                       {type: gSignals[aInstance.constructor.targetType][aSignal],
                                        root: false});
    var id = aWrappee(aInstance, aSignal, ccw, null, null, 0);
    // Root the callback
    gSignalHandlers[id] = ccw;

    return id;
  }, glib.gulong, [glib.gpointer, glib.gchar.ptr, this.GCallback,
                   glib.gpointer, this.GClosureNotify, this.GConnectFlags],
     "g_signal_connect_data");
 
  lib.lazy_bind_with_wrapper("g_signal_handler_disconnect", function(aWrappee,
                                                                     aInstance,
                                                                     aId) {
    aWrappee(aInstance, aId);
    // Unroot the callback
    delete gSignalHandlers[aId];
  }, ctypes.void_t, [glib.gpointer, glib.gulong]);

  Object.defineProperty(this, "createSignal", {
    value: function(aType, aName, aRetType, aArgTypes) {
      if (!aType || "targetType" in aType) {
        throw Error("Must specify a concrete C type");
      }

      if (typeof(aName) != "string") {
        throw Error("Must specify a signal name");
      }

      if (!(aType in gSignals)) {
        gSignals[aType] = {};
      }

      if (aName in gSignals[aType]) {
        throw Error("Cannot redefine signal");
      }

      gSignals[aType][aName] =
        ctypes.FunctionType(ctypes.default_abi, aRetType, aArgTypes).ptr;
    },
    enumerable: true
  });
}

var gobject = new CTypesUtils.newLibrary(GOBJECT_LIBNAME, GOBJECT_ABIS, gobject_defines);
