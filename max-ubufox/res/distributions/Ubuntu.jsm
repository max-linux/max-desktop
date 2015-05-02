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

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;

Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/FileUtils.jsm");
Cu.import("resource://ubufox/modules/Distro.jsm");
Cu.import("resource://ubufox/modules/utils.jsm");

addLogger(this, "distro.ubuntu");

var EXPORTED_SYMBOLS = [ "DistroImpl" ];

const HOMEPAGE_BASE_URI = "http://start.ubuntu.com/";
const HOMEPAGE_PATH = "%VERSION%/%SEARCH_PROVIDER%";

// This allows us to map search plugins to a startpage search provider
const HOMEPAGE_PROVIDER_TABLE = [
  {"field": "searchForm", "regex": /.*search\.yahoo\.com\//, "provider": "Yahoo"},
  {"field": "searchForm", "regex": /.*google\.com\//, "provider": "Google"},
];

// This allows us to specify additional query parameters for
// a search provider
const HOMEPAGE_PROVIDER_QUERY_MAP = {
  "Google": "sourceid=hp"
};

// The default startpage search provider
const HOMEPAGE_DEFAULT_PROVIDER = "Google";

function apportExists() {
  try {
    let executable = new FileUtils.File("/usr/bin/ubuntu-bug");
    return executable.isExecutable();
  } catch(e) {
    return false;
  }
}

function getProviderForCurrentSearchEngine() {

  function buildReturnParams(aProvider) {
    return {"name": aProvider,
            "query": HOMEPAGE_PROVIDER_QUERY_MAP[aProvider]};
  }

  let searchEngine = Services.search.currentEngine;
  try {
    for each (let entry in HOMEPAGE_PROVIDER_TABLE) {
      if (searchEngine[entry.field].match(entry.regex) ==
          searchEngine[entry.field]) {
        return buildReturnParams(entry.provider);
      }
    }
  } catch(e) { ERROR("Failed to build parameters for search engine", e); }

  return buildReturnParams(HOMEPAGE_DEFAULT_PROVIDER);
}

var DistroImpl = {
  get canReportBug() {
    let dist_id;
    try {
      dist_id = Services.prefs.getCharPref("distribution.id");
    } catch(e) { }

    return apportExists() && dist_id == "canonical";
  },

  reportBug: function Ubuntu_reportBug() {
    if (!apportExists()) {
      throw new Error("Apport must be installed in order to report a bug");
    }

    let dist_id;
    try {
      dist_id = Services.prefs.getCharPref("distribution.id");
    } catch(e) { }

    if (dist_id != "canonical") {
      throw new Error("Don't know how to report a bug for non-Ubuntu builds");
    }

    let executable = new FileUtils.File("/usr/bin/ubuntu-bug");

    let procUtil = Cc["@mozilla.org/process/util;1"].createInstance(Ci.nsIProcess);
    procUtil.init(executable);

    let pkgname = Cc["@mozilla.org/xre/app-info;1"]
                  .getService(Ci.nsIXULAppInfo).name.toLowerCase()
    if (!pkgname) {
        pkgname = "firefox";
    }
    let args = new Array(pkgname);

    procUtil.run(false, args, args.length);
  },

  get startpageURI() {
    try {
      if (Services.prefs.getCharPref("distribution.id") != "canonical") {
        return Services.io.newURI("about:home", null, null);
      }
    } catch(e) {
      return Services.io.newURI("about:home", null, null);
    }

    let provider = getProviderForCurrentSearchEngine();

    let baseuri = Services.io.newURI(HOMEPAGE_BASE_URI, null, null);

    let path = HOMEPAGE_PATH;
    path = path.replace(/%VERSION%/, distro.version.replace(/^([^\.]*)\.([^\.]*).*/, "$1.$2"));
    path = path.replace(/%SEARCH_PROVIDER%/, provider.name);

    let query = "";
    if (provider.query != null) {
      query += "?" + provider.query;
    }

    return Services.io.newURI(path + "/" + query, null, baseuri);
  },

  get updateRestartNotificationStyle() {
    let parts = distro.version.split(".");
    if (Number(parts[0]) > 12 || (Number(parts[0]) == 12 && Number(parts[1] == 10))) {
      return "popup";
    }

    return "infobar";
  }
};
