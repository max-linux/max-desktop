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

function moSearchEngine(pages, titles, keywords, indexs)
{
  this.ps = pages;
  this.ts = titles;
  this.ks = keywords;
  this.is = indexs;
  this.ls = null;
  this.sr = null;
  
  this.setListbox = function(listbox)
  {
    this.ls = listbox;
  }
  
  this.createResultList = function(result)
  {
    this.sr = result;
    this.ls.length = 0;
    for(var i=0; i<result.length; i++)
    {
      var o   = document.createElement("OPTION");
      o.value = this.ps[result[i]];
      o.text  = this.ts[result[i]];
      this.ls[this.ls.length] = o;
    }
  }
  
  this.search = function(s)
  {
    var ss = this.parse(s);
    if (ss.length == 0)
    {
      alert("cannot search the phrase");
      return;
    }
    else
    {
      var sr = this.doSearch(ss);
      if (!sr)
      {
        alert("no matches found");
        return;
      }
      if (moTop)
      {
        moTop.SearchKeyword = s;
        moTop.RegexSearchKeyword = ss.join('|');
      }
      this.createResultList(sr);
    }
  }
  
  this.doSearch = function(ss)
  {
    var sr;
    for(var si=0; si<ss.length; si++)
    {
      var ki = this.findIndex(ks, ss[si]);
      if(ki < 0)
        return null;
      else
        if (si == 0)
          sr = is[ki];
        else
          sr = this.AND(sr, is[ki]);
      if (sr.length == 0) return null;
    }
    return sr;
  }
  
  this.findIndex = function(array, item)
  {
    var result = -1;
    for(var i=0; i<array.length; i++)
      if(item == array[i])
      {
        result = i;
        break;
      }
    return result; 
  }
  
  this.parse = function(s)
  {
    s = s.toLowerCase();
    var ss = [];
    var re = /[\w\d]+/g;
    var w;
    while ( w = re.exec(s) ) ss[ss.length] = w;
    return ss;
  }
  
  this.AND = function(a, b)
  {
    var result=[];
    for(var ai=0; ai<a.length; ai++)
      if(this.findIndex(b, a[ai]) >= 0) result[result.length] = a[ai];
    return result;
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
    if(moTop) {
      moTop.curPageIndex = this.sr[this.ls.selectedIndex];
      moTop.markKeywords = true;
    }
  }
  
  this.init = function(textbox) {
    if (moTop && moTop.SearchKeyword != null) {
      textbox.value = moTop.SearchKeyword;
      this.search(moTop.SearchKeyword);
    }
  }  
}

var se = new moSearchEngine(ps, ts, ks, is);
