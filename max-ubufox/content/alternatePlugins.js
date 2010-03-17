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
 *   Doron Rosenberg <doronr@us.ibm.com>
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

if (!gBrowser)
  var gBrowser = null;
var gAltPluginWizard = null;

altPluginWizard.prototype.pluginUsed = function(pluginElement){

  var pluginsArray = pluginElement.ownerDocument.mimeContent;
  if (!pluginsArray)
    pluginsArray = {};

  var pluginInfo = getPluginInfo(pluginElement);
  pluginsArray[pluginInfo.mimetype] = pluginInfo;
  var thisTab = pluginElement.ownerDocument;
  thisTab.mimeContent = pluginsArray;

  if (gBrowser.selectedBrowser.contentDocument != pluginElement.ownerDocument)
      return;

  var iconAltPlugins = document.getElementById("iconAltPlugins");
  iconAltPlugins.hidden = false;
  var menuAltPlugins = document.getElementById("menuAltPlugins");
  menuAltPlugins.disabled = false;
}

altPluginWizard.prototype.tabSelected = function(aEvent){

  if(gBrowser.selectedBrowser.contentDocument.mimeContent){
    var iconAltPlugins = document.getElementById("iconAltPlugins");
    iconAltPlugins.hidden = false;
    var menuAltPlugins = document.getElementById("menuAltPlugins");
    menuAltPlugins.disabled = false;
  }else{
    var iconAltPlugins = document.getElementById("iconAltPlugins");
    iconAltPlugins.hidden = true;
    var menuAltPlugins = document.getElementById("menuAltPlugins");
    menuAltPlugins.disabled = false;
  }
}

altPluginWizard.prototype.domContentLoaded = function(aEvent){
    var elements = aEvent.target.getElementsByTagName("embed");
    for (let a = 0; a< elements.length; a++) {
	gAltPluginWizard.pluginUsed (elements[a]);
    }
}

function altPluginWizard(){
}

function openPluginFinder(contentMimeArray){
    
  var iconAltPlugins = document.getElementById("iconAltPlugins");
  window.openDialog("chrome://ubufox/content/pluginAlternativeOverlay.xul",
                   "PFSWindow", "chrome,centerscreen,resizable=yes",
                   {plugins: contentMimeArray, browser: gBrowser.selectedBrowser, pluginsOnTab: !iconAltPlugins.hidden});
}


window.addEventListener("load", function() {

  if (!gBrowser)
    gBrowser = document.getElementById("content");  
  gAltPluginWizard = new altPluginWizard();
  gBrowser.tabContainer.addEventListener("TabSelect", gAltPluginWizard.tabSelected, false);
  gBrowser.addEventListener("DOMContentLoaded", gAltPluginWizard.domContentLoaded, false);
  window.removeEventListener("load", arguments.callee, false);

}, false);

