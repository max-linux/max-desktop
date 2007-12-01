//=======================================================================//
//                                                                       //
//  Macrobject Software Code Library                                     //
//  Copyright (c) 2004-2007 Macrobject Software, All Rights Reserved     //
//  http://www.macrobject.com                                            //
//                                                                       //
//  Warning!!!                                                           //
//      The library can only be used with web help system                //
//      created by Word-2-Web Pro.                                       //
//                                                                       //
//=======================================================================//

function moHelpIndex(pages, titles, indexs) 
{
  this.ps = pages;
  this.ts = titles;
  this.ti = indexs;
  this.ls = null;
  
  this.setListbox = function(listbox)
  {
    this.ls = listbox;
  }

  this.createIndex = function()
  {
    this.ls.length = 0;
    for(var i=0; i<ti.length; i++)
    {
      var o   = document.createElement("OPTION");
      o.value = this.ps[ti[i]];
      o.text  = this.ts[ti[i]];
      this.ls[this.ls.length] = o;
    }
  }
  
  this.search = function(keyword)
  {
    if (keyword.length == 0) return;
    keyword = keyword.toLowerCase();
    for (var i= 0; i<ti.length; i++)
    {
      var title = ts[ti[i]].toLowerCase();
      if (title.indexOf(keyword) == 0)
      {
        this.ls[i].selected = true;
        return;
      } 
    }
  }
  
  this.keypress = function()
  {
    if (window.event.keyCode == 13)
      this.display();
  }
  
  this.display = function()
  {
    if (this.ls.selectedIndex < 0) return;
    var page = this.ls[this.ls.selectedIndex].value;
    top.frames["content"].location = "topics/" + page;
    if(moTop) moTop.curPageIndex = this.ti[this.ls.selectedIndex];
  }  
}

var hi = new moHelpIndex(ps, ts, ti);
