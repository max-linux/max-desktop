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
 * The Original Code is ubufox
 *
 * The Initial Developer of the Original Code is Canonical Ltd.
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

const Cu = Components.utils;
const Ci = Components.interfaces;

const nsIHttpChannel = Ci.nsIHttpChannel;
const nsIObserver = Ci.nsIObserver;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");

function ufoxHTTPListener() {
  this.headerName = "X-Ubuntu";
  this.headerValue = Services.prefs.getCharPref("extensions.ubufox@ubuntu.com.release");

  Services.obs.addObserver(this, "http-on-modify-request", false);
}

ufoxHTTPListener.prototype = {

  classID: Components.ID("{f0b9df8b-0b9a-4432-9812-45b597e71e26}"),

  observe: function(subject, topic, data)
  {
    if (topic == "http-on-modify-request") {
      var httpChannel = subject.QueryInterface(nsIHttpChannel);
      // We only do this for apt.ubuntu.com for now.
      // Note, this is triggered of every HTTP request, so don't do
      // anything stupid here like comparing lots of strings.
      if (httpChannel.originalURI.host.indexOf("apt.ubuntu.com") != -1) {
        httpChannel.setRequestHeader(this.headerName, this.headerValue, false);
      }
    }
  },

  QueryInterface: XPCOMUtils.generateQI([nsIObserver]),
};

var NSGetFactory = XPCOMUtils.generateNSGetFactory([ufoxHTTPListener]);
