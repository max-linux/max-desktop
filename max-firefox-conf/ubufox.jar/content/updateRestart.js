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
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Alexander Sack <asac@jwsdot.com> - Canonical Ltd.
 *   Arzhel Younsi <xionox@gmail.com>
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

var gAppStartup = Components.classes["@mozilla.org/toolkit/app-startup;1"].getService(Components.interfaces.nsIAppStartup);
var gCategoryManager = Components.classes["@mozilla.org/categorymanager;1"].getService(Components.interfaces.nsICategoryManager);
var intervalID = null;
window.addEventListener('load', scheduleCheckForUpdate, false);

function showRestartNotification()
{
  var strbundle = document.getElementById("ubufox-restart-strings")
  var restartNotificationLabel=strbundle.getString("restartNotificationLabel");
  var restartNotificationButton=strbundle.getString("restartNotificationButton");
  var restartNotificationKey=strbundle.getString("restartNotificationKey");
  var buttons = [{ label: restartNotificationButton, accessKey: restartNotificationKey, callback: restart }];
  
  var num = gBrowser.browsers.length;
  for (var i = 0; i < num; i++) {
    var b = gBrowser.getBrowserAtIndex(i);
    try {
      var notificationBox = gBrowser.getNotificationBox(b);
      var notification = notificationBox.getNotificationWithValue("notification-restart");
      if (!notification)
      {
        notificationBox.appendNotification(restartNotificationLabel, 'notification-restart',"", notificationBox.PRIORITY_WARNING_LOW, buttons);
      }
    } catch(e) {
        Components.utils.reportError(e);
     }
  }
}

function restart()
{
  gAppStartup.quit(gAppStartup.eRestart | gAppStartup.eAttemptQuit);
}

function checkUpdate()
{
  var resReqFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  resReqFile.initWithPath("/var/lib/update-notifier/user.d/firefox-3.0-restart-required");

  if(resReqFile.exists())
  {
    var dateResReq = resReqFile.lastModifiedTime;
    var timestamp = getAndSetTimeBase();
    if(dateResReq > timestamp)
    {
      showRestartNotification();
    }
  }
}

function getAndSetTimeBase () {
  var timestamp = 0;
  try {
    var t = gCategoryManager.getCategoryEntry ("ubufox", "startup-timestamp");
    timestamp = parseInt (t);
  } catch (e) {
    // exception means that there is no such entry; set the initial timestamp
    timestamp = Date.now();
    gCategoryManager.addCategoryEntry ("ubufox", "startup-timestamp", ""+timestamp, false, true);
  }
  return timestamp;
}

function scheduleCheckForUpdate()
{
  getAndSetTimeBase ();
  if (!intervalID)
    intervalID = setInterval("checkUpdate()", 10000);
}
