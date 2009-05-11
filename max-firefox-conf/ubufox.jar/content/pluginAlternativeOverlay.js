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

var prefBranch = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
var pluginManager = Components.classes["@mozilla.org/plugin/manager;1"].getService(Components.interfaces.nsIPluginManager);

const APP_ICON_ATTR_NAME = "appHandlerIcon";

window.addEventListener("load", showContentList, false);

var gBrowser = document.getElementById("content");
var usedMimeTypes = new Array();
var allHandledMimeTypes = new Array();
var filteredMimeTypes = null;

for (var item in window.arguments[0].plugins){
  usedMimeTypes.push(window.arguments[0].plugins[item].mimetype);  
}

let store = new Object();
for (let i = 0; i < navigator.mimeTypes.length; i++) {
  if(!navigator.mimeTypes.item(i).enabledPlugin)
    continue;
  let mtype = navigator.mimeTypes.item(i).type;
  if(!store[mtype])
    allHandledMimeTypes.push(mtype);
  store[mtype] = "x";
}

var filehintNameMap = null;


function parseFilehintNameText (text)
{
   var map = {};
   var array = new Array();

   var start = 0;
   var next  = 0;
   next = text.indexOf("\n", start);

   while (next  > -1) {
      array.push(text.substring(start, next));
      start = next+1;
      next = text.indexOf("\n", start);
   }
   array.push(text.substring(start));

   var key = null;
   var value = null;
   var line = null;
   var separator = 0;
   var left = null;
   var right = null;
   for (let a = 0; a < array.length; a++) {
      line = array[a];
      separator = line.indexOf(":");
      left = line.substring(0, separator);
      right = line.substring(separator+2);
      if (left == "filehint")
         key = right;
      else if(left == "name") {
         map[key] = right;
      }
   }
   return map;
}

function getFilehintNameMap () {
  if (filehintNameMap)
    return filehintNameMap;

  var url = "http://localhost/~asac/cgi-bin/plugin-finder.py?op=filehint2name&distributionID=9.04";
  try {
    url = prefBranch.getCharPref("pfs.filehint.url");
  } catch (e) {}

  var req = new XMLHttpRequest();
  req.open('GET', url, true); /* 3rd argument, true, marks this as async */
  req.onreadystatechange = function (aEvt) {
    if (req.readyState == 4) {
       if(req.status == 200) {
         filehintNameMap = parseFilehintNameText (req.responseText);
         showContentList();
       }
    }
  };
  req.send(null); 
  return null;
}

if (!filehintNameMap)
  getFilehintNameMap();

function showContentList(){
  var contentName;
  if(!filteredMimeTypes) {
    if(usedMimeTypes.length <= 0)
      filteredMimeTypes = allHandledMimeTypes;
    else
      filteredMimeTypes = usedMimeTypes;
  }
  var _list = document.getElementById("pluginsList");   
  while (_list.hasChildNodes())
    _list.removeChild(_list.lastChild);
  for each (let mimeType in filteredMimeTypes) {
  	if(mimeType != ''){
		let item = document.createElement("richlistitem");
		item.setAttribute("type", mimeType);
		contentName = getDescByMime(mimeType);
		item.setAttribute("typeDescription",contentName);
		item.setAttribute("actionDescription", describePreferredAction(mimeType));
		item.setAttribute(APP_ICON_ATTR_NAME, "plugin");
                item.setAttribute("typeIcon", "moz-icon://goat?size=" + 16 + "&contentType=" + mimeType);
		_list.appendChild(item);
    }
  }
}

function describePreferredAction(mimeType){
  var filehintNameMap = getFilehintNameMap();

  var preferredFileName = null;
  try {
    preferredFileName = prefBranch.getCharPref("modules.plugins.mimetype." + mimeType);
  } catch (e) {}

  var perfectMatchPlugin = null;

  if (preferredFileName) {
    for (let i =0; i < navigator.plugins.length; i++) {
      var plugIn = navigator.plugins.item(i);
      if(preferredFileName == plugIn.filename) {
        perfectMatchPlugin = plugIn;
        break;
      }
    }
  }

  var L = navigator.mimeTypes.length;
  for(var i=0; i<L && perfectMatchPlugin == null; i++) {
     if (navigator.mimeTypes.item(i).type == mimeType)
     {
       perfectMatchPlugin = navigator.mimeTypes.item(i).enabledPlugin;
     }
   }

  if(!perfectMatchPlugin)
    return "...";

  for (betterNameCarrierKey in filehintNameMap) {
     if (perfectMatchPlugin.filename && perfectMatchPlugin.filename.indexOf(betterNameCarrierKey) > -1) {
       return filehintNameMap[betterNameCarrierKey];
     }
  }

  if(perfectMatchPlugin.name.indexOf("Shockwave Flash") >= 0 ||
     perfectMatchPlugin.name.indexOf("Windows Media Player") >= 0)
    return perfectMatchPlugin.description;
  return perfectMatchPlugin.name;
}

function getDescByMime(aMimeType){
  for(let i = 0; i < navigator.mimeTypes.length; i++) {
     let mimeType = navigator.mimeTypes[i];
     if (mimeType.type == aMimeType && mimeType.description != null && mimeType.description.length > 0)
       return mimeType.description + " (" + aMimeType + ")";
   }
  return aMimeType;  
}

function onSelectionChanged() {
  var _list = document.getElementById("pluginsList");
  if (_list.selectedItem)
    _list.setAttribute("lastSelectedType", _list.selectedItem.getAttribute("type"));
}

function onSelectUsedPlugins (e) {
	filteredMimeTypes = usedMimeTypes;
	showContentList();
}

function onSelectAllPlugins (e) {
	filteredMimeTypes = allHandledMimeTypes;
	showContentList();
}
   
var gApplicationsPane = {  

  rebuildActionsMenu: function() {
    var filehintNameMap = getFilehintNameMap();
    var _list = document.getElementById("pluginsList");
    var typeItem = _list.selectedItem;
    var menu = document.getAnonymousElementByAttribute(typeItem, "class", "actionsMenu");
    var menuPopup = menu.menupopup;
    var pluginArrayLength = navigator.plugins.length;
    
    while (menuPopup.hasChildNodes())
      menuPopup.removeChild(menuPopup.lastChild);

    var menuItem = null;
    for each (let mimeType in filteredMimeTypes) {
      for (var i = 0; i < pluginArrayLength; i++) {
        var plugin = navigator.plugins[i];
        for (var j = 0; j < plugin.length; j++) {
          if(plugin.item(j).type == mimeType && typeItem.type == mimeType){  
            menuItem = document.createElement("menuitem");
            var pluginName = plugin.name;
            if(pluginName.indexOf("Shockwave Flash") >= 0)
              pluginName = plugin.description;
            for (betterNameCarrierKey in filehintNameMap) {
               if (plugin.filename && plugin.filename.indexOf(betterNameCarrierKey) > -1) {
                  pluginName = filehintNameMap[betterNameCarrierKey];
               }
            }
            menuItem.setAttribute("label", pluginName);
            menuItem.setAttribute("id", pluginName);
            menuItem.ubufoxPluginFilename = plugin.filename;
            menuItem.setAttribute(APP_ICON_ATTR_NAME, "plugin");
            menuPopup.appendChild(menuItem);
          }
        }
      }
    }
    var strbundle = document.getElementById("ubufox-alt-strings")
    var search = strbundle.getString("ubufox.altplugins.search");

    menuItem = document.createElement("menuitem");
    menuItem.setAttribute("id", "install-plugin-handler");
    menuItem.setAttribute("label", search);
    menuItem.type = typeItem.type;
    menuPopup.appendChild(menuItem);
  },
  
  onSelectAction: function(aActionItem) {
    var _list = document.getElementById("pluginsList");
    var typeItem = _list.selectedItem;

    var id = aActionItem.getAttribute ("id");
    if (id == "install-plugin-handler") {
      var pluginInfoArray = {};
      var fakePluginInfo = new Object();
      fakePluginInfo.mimetype = typeItem.type;
      fakePluginInfo.pluginsPage = "";
      pluginInfoArray[typeItem.type] = fakePluginInfo;

      window.openDialog("chrome://mozapps/content/plugins/pluginInstallerWizard.xul",
                 "PFSWindow", "chrome,centerscreen,resizable=yes,width=600,height=600",
                 {plugins: pluginInfoArray, browser: gBrowser});

      pluginManager.reloadPlugins(true);
      showContentList();
    }
    else {
      prefBranch.setCharPref("modules.plugins.mimetype." + typeItem.type, aActionItem.ubufoxPluginFilename);
      pluginManager.reloadPlugins(true);
      showContentList();
    }
    return;
  }
};

