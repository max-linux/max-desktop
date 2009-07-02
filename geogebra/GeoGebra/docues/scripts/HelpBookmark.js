//=======================================================================//
//                                                                       //
//  Macrobject Software Code Library                                     //
//  Copyright (c) 2004-2008 Macrobject Software, All Rights Reserved     //
//  http://www.macrobject.com                                            //
//                                                                       //
//  Warning!!!                                                           //
//      The library can only be used with web help system                //
//      created by Word-2-Web Pro.                                       //
//                                                                       //
//=======================================================================//

function getCookie( name ) {
  var start = document.cookie.indexOf( name + "=" );
  var len = start + name.length + 1;
  if ( ( !start ) && ( name != document.cookie.substring( 0, name.length ) ) ) {
    return "";
  }
  if ( start == -1 ) return "";
  var end = document.cookie.indexOf( ';', len );
  if ( end == -1 ) end = document.cookie.length;
  return unescape( document.cookie.substring( len, end ) );
}

function setCookie( name, value, expires, path, domain, secure ) {
  var today = new Date();
  today.setTime( today.getTime() );
  if ( expires ) {
    expires = expires * 1000 * 60 * 60 * 24;
  }
  else {
    expires = 365 * 1000 * 60 * 60 * 24;
  }
  var expires_date = new Date( today.getTime() + (expires) );
  document.cookie = name+'='+escape( value ) +
    ( ( expires ) ? ';expires='+expires_date.toGMTString() : '' ) + //expires.toGMTString()
    ( ( path ) ? ';path=' + path : '' ) +
    ( ( domain ) ? ';domain=' + domain : '' ) +
    ( ( secure ) ? ';secure' : '' );
}

function moHelpBookmark(pages, titles, pageids, name) 
{
  this.ps = pages;
  this.ts = titles;
  this.pi = pageids;
  this.ls = null;
  this.bmName = name;
  
  this.setListbox = function(listbox)
  {
    this.ls = listbox;
  }

  this.getBookmarks = function()
  {
    var bms = getCookie(this.bmName).match(/\d+/g);
    if (!bms) return [];
    else return bms;
  }
  
  this.addBookmark = function()
  {
    if (!moTop) return;
    if (! getCookie(this.bmName).match(new RegExp('\\b'+this.pi[moTop.curPageIndex]+'\\b'), ''))
      setCookie(this.bmName, getCookie(this.bmName) + ',' + this.pi[moTop.curPageIndex]);
    this.createBookmarks();
  }
  
  this.delBookmark = function()
  {
    if (this.ls.selectedIndex < 0) return; 
    var id = this.ls[this.ls.selectedIndex].pageId;
    setCookie(this.bmName, getCookie(this.bmName).replace(new RegExp(',?\\b'+id+'\\b', 'g'), ''));
    this.createBookmarks();
  }
  
  this.createBookmarks = function()
  {
    this.ls.length = 0;
    var bms = this.getBookmarks()
    for(var i=0; i<bms.length; i++)
    {
      var bmid = bms[i];
      var id = this.findPage(bmid);
      
      var o   = document.createElement("OPTION");
      this.ls[this.ls.length] = o;
      o.value = this.ps[id];
      o.innerHTML = this.ts[id];
      o.pageIndex = id;
      o.pageId = bmid;
    }
  }
  
  this.findPage = function(bmid)
  {
    for (var i= 0; i<this.pi.length; i++)
    {
      if (bmid == this.pi[i])
      {
        return i;
      } 
    }
    return -1;
  }
  
  this.keypress = function()
  {
    if (window.event.keyCode == 13)
      this.display();
  }
  
  this.display = function()
  {
    if (this.ls.selectedIndex < 0) return;
    var item = this.ls[this.ls.selectedIndex];
    var page = item.value;
    if(moTop) {
      getFrame("content").location = "topics/" + page;
      moTop.curPageIndex = item.pageIndex;
    }
  }  
}

var hb = new moHelpBookmark(ps, ts, pi, "Bookmarks_E9AE19FAE54E1A597669663028108237");
