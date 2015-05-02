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
Components.utils.import("resource://gre/modules/Services.jsm");
Components.utils.import("resource://ubufox/modules/Distro.jsm");

function AboutStartpage() {}
AboutStartpage.prototype = {
  classDescription: "About Startpage",
  contractID: "@mozilla.org/network/protocol/about;1?what=startpage",
  classID: Components.ID("{7a2a7a56-827f-4b38-bdac-31aa7ec2971d}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIAboutModule]),
 
  getURIFlags: function(aURI) {
    return (Ci.nsIAboutModule.ALLOW_SCRIPT |
            Ci.nsIAboutModule.URI_SAFE_FOR_UNTRUSTED_CONTENT);
  },

  newChannel: function(aURI) {
    let secMan = Cc["@mozilla.org/scriptsecuritymanager;1"].
                 getService(Ci.nsIScriptSecurityManager);

    let uri;
    // allow defaults packages to overwrite the homepage
    try {
      uri = Services.io.newURI(Services.prefs.getCharPref("extensions.ubufox@ubuntu.com.custom_homepage"),
                               null, null);
    } catch(e) {
      uri = distro.startpageURI;
    }

    //let channel = Services.io.newChannelFromURI(uri);
    // FOR MAX
    let datauri = Services.io.newURI("chrome://ubufox/content/startpage.html", null, null);
    let channel = Services.io.newChannelFromURI(datauri);
    channel.originalURI = aURI;

    // See https://bugzilla.mozilla.org/show_bug.cgi?id=774585
    channel.owner = "getSimpleCodebasePrincipal" in secMan ?
                    null : secMan.getCodebasePrincipal(aURI);

    return channel;
  }
};

const NSGetFactory = XPCOMUtils.generateNSGetFactory([AboutStartpage]);
