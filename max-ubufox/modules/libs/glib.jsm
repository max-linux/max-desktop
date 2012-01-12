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

var EXPORTED_SYMBOLS = [ "glib" ];

const GLIB_LIBNAME = "glib-2.0";
const GLIB_ABIS = [ 0 ];

const Cu = Components.utils;

Cu.import("resource://gre/modules/ctypes.jsm");
Cu.import("resource://ubufox/libs/ctypes-utils.jsm");

function glib_defines(lib) {
  // Enums

  // Types
  this.gpointer = ctypes.voidptr_t;
  this.gchar = ctypes.char;
  this.gint = ctypes.int;
  this.gint32 = ctypes.int32_t;
  this.guint32 = ctypes.uint32_t;
  this.gulong = ctypes.unsigned_long;
  this.GQuark = this.guint32;
  this.GError = ctypes.StructType("GError",
                                  [{'domain': this.GQuark},
                                   {'code': this.gint},
                                   {'message': this.gchar.ptr}]);
  this.GVariant = ctypes.StructType("GVariant");
  this.GVariantBuilder = ctypes.StructType("GVariantBuilder", [{'x': ctypes.size_t.array(16)}]);
  // XXX: Is this right?
  this.GVariantType = ctypes.char;

  // Templates

  // Functions
  lib.lazy_bind("g_variant_builder_new", this.GVariantBuilder.ptr,
                this.GVariantType.ptr);
  lib.lazy_bind("g_variant_builder_add", ctypes.void_t, this.GVariantBuilder.ptr,
                this.gchar.ptr, "...");
  lib.lazy_bind("g_variant_builder_add_value", ctypes.void_t,
                this.GVariantBuilder.ptr, this.GVariant.ptr);
  lib.lazy_bind("g_variant_builder_end", this.GVariant.ptr,
                this.GVariantBuilder.ptr);
  lib.lazy_bind("g_variant_builder_unref", ctypes.void_t,
                this.GVariantBuilder.ptr);
  lib.lazy_bind("g_variant_builder_init", ctypes.void_t,
                this.GVariantBuilder.ptr, this.GVariantType.ptr);
  lib.lazy_bind("g_variant_builder_clear", ctypes.void_t,
                this.GVariantBuilder.ptr);
  lib.lazy_bind("g_variant_unref", ctypes.void_t, this.GVariant.ptr);
  lib.lazy_bind("g_variant_get_string", this.gchar.ptr, this.GVariant.ptr,
                ctypes.size_t.ptr);
  lib.lazy_bind("g_variant_get", ctypes.void_t, this.GVariant.ptr,
                this.gchar.ptr, "...");
  lib.lazy_bind("g_variant_get_int32", this.gint32, this.GVariant.ptr);
}

if (!glib || !glib.available()) {
  var glib = new ctypes_library(GLIB_LIBNAME, GLIB_ABIS, glib_defines);
}
