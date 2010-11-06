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
 * The Original Code is about:robots
 *
 * The Initial Developer of the Original Code is Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Ryan Flint <rflint@mozilla.com>
 *   Justin Dolske <dolske@mozilla.com>
 *   Johnathan Nightingale <johnath@mozilla.com>
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

Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

/*
var HOMEPAGE_OFFLINE = "file:///usr/share/ubuntu-artwork/home/index.html";
var HOMEPAGE_OFFLINE_TMPL = "/usr/share/ubuntu-artwork/home/locales/index-"
*/
var HOMEPAGE_OFFLINE = "file:///usr/share/xul-ext/max-ubufox/startpage.html";
var HOMEPAGE_OFFLINE_TMPL = "file:///usr/share/xul-ext/max-ubufox/startpage.html";

var HOMEPAGE_ONLINE_PREFIX = "file:///usr/share/xul-ext/max-ubufox/startpage.html#";

function getIsOffline() {
  return false;
  /*
  var prefs = Cc["@mozilla.org/preferences-service;1"]
             .getService(Ci.nsIPrefBranch);
  try {
    return prefs.getBoolPref("browser.offline");
  } catch (e) {
    // in firefox 3.0 browser.offline does not exist in the begginning
    // hence we interpret pref missing as ONLINE
    return false;
  }*/
}

function getUALocale() {
  var prefs = Cc["@mozilla.org/preferences-service;1"]
             .getService(Ci.nsIPrefBranch);

  var userAgentLocale = null;
  try {
    var userAgentLocaleLocalized = null;

    try {
      userAgentLocaleLocalized = prefs.getComplexValue("general.useragent.locale",
                                                       Ci.nsIPrefLocalizedString);
    } catch (e) {}

    if (userAgentLocaleLocalized) {
        userAgentLocale = userAgentLocaleLocalized.toString();
    } else {
        userAgentLocale = prefs.getCharPref("general.useragent.locale");
    }
  } catch (e) { userAgentLocale = "en-US";}

  return userAgentLocale;
}

function get_valid_offlinehomepage() {
   return HOMEPAGE_OFFLINE;
}

function getCurrentSearchEngineName () {
  var searchService = Cc["@mozilla.org/browser/search-service;1"]
                      .getService (Ci.nsIBrowserSearchService);
  var defaultEngine = searchService.currentEngine;
  return defaultEngine.name;
}

function AboutHome() {}
AboutHome.prototype = {
  classDescription: "About Home",
  contractID: "@mozilla.org/network/protocol/about;1?what=home",
  classID: Components.ID("{7a2a7a56-827f-4b38-bdac-31aa7ec2971d}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIAboutModule]),
 
  getURIFlags: function(aURI) {
    return (Ci.nsIAboutModule.ALLOW_SCRIPT |
            Ci.nsIAboutModule.URI_SAFE_FOR_UNTRUSTED_CONTENT);
  },

  newChannel: function(aURI) {
    var ios = Cc["@mozilla.org/network/io-service;1"].
              getService(Ci.nsIIOService);

    var secMan = Cc["@mozilla.org/scriptsecuritymanager;1"].
                 getService(Ci.nsIScriptSecurityManager);
    var principal = secMan.getCodebasePrincipal(aURI);

    if (!getIsOffline()) {
      let searchEngineName = getCurrentSearchEngineName();
      /*let channel = ios.newChannel(HOMEPAGE_ONLINE_PREFIX + "/" + searchEngineName + "/", null, null);*/
      let channel = ios.newChannel(HOMEPAGE_OFFLINE, null, null);
      channel.originalURI = aURI;
      channel.owner = principal;
      return channel;
    }

    let channel = ios.newChannel(HOMEPAGE_OFFLINE, null, null);
//    channel.originalURI = aURI;
    channel.owner = principal;
    return channel;
  }
};

function NSGetModule(compMgr, fileSpec) {
  return XPCOMUtils.generateModule([AboutHome]);
}

