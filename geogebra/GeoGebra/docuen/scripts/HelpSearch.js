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

function moSearchEngine(pages, titles, keywords, indexs)
{
  this.ps = pages;
  this.ts = titles;
  this.ks = keywords;
  this.is = indexs;
  this.ls = null;
  this.sr = null;
  this.any = false;
  this.last = [];
  this.inLast = false;
  this.titlesOnly = false;
  
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
      this.ls[this.ls.length] = o;
      o.value = this.ps[result[i]];
      o.innerHTML = this.ts[result[i]];
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
      if (this.inLast) sr = this.AND(sr, this.last);
      this.last = sr;
      if (!sr)
      {
        alert("no matches found");
        return;
      }
      if (moTop)
      {
        moTop.SearchKeyword = s;
        moTop.RegexSearchKeyword = '';
        for (var i=0; i<ss.length; i++) {
          if (i > 0) moTop.RegexSearchKeyword += '|';
          var re = /[\u0100-\uFFFF]/;
          if (re.test(ss[i])) moTop.RegexSearchKeyword += ss[i];
          else moTop.RegexSearchKeyword += '\\b' + ss[i] + '\\b';
        }
      }
      this.createResultList(sr);
    }
  }
  
  this.doSearch = function(ss)
  {
    var sr;
    for (var si=0; si<ss.length; si++)
    {
      var sr2;
      if (this.titlesOnly)
        sr2 = this.matchArray(ts, ss[si]);
      else if (ss[si].toString().match(/[*?]/))
        sr2 = this.matchArray(ks, ss[si]);
      else
        sr2 = this.findArray(ks, ss[si]);
      if (!this.any && sr2.length == 0)
        return null;
      else
        if (si == 0)
          sr = sr2;
        else
          if (this.any)
            sr = this.OR(sr, sr2);
          else
            sr = this.AND(sr, sr2);
      if (!this.any && sr.length == 0) return null;
    }
    return sr;
  }
  
  this.matchArray = function(array, item)
  {
    var result = [];
    var re;
    if (this.titlesOnly)
      re = new RegExp('(^|\\W)' + item + '(\\W|$)', 'i');
    else
      re = new RegExp('^' + item + '$', 'i');
    for(var i=0; i<array.length; i++)
      if(array[i].match(re))
      {
        if (this.titlesOnly)
          result[result.length] = i
        else
          result = this.OR(result, is[i]);
      }
    return result;
  }
  
  this.findArray = function(array, item)
  {
    var ki = this.findIndex(array, item);
    if(ki < 0)
      return [];
    else
      return is[ki];
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
    var re = /[\*\?\w\u00C0-\u00D6\u00D8-\u00F6\u00F8-\u00FF\u0160\u0178\u017D\u0192\u02C6]{2,}/g;
    var w;
    while ( w = re.exec(s) )
    {
      var word = w.toString();
      word = word.replace(/(?=[*?])/g, "[\\w\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u00FF\\u0160\\u0178\\u017D\\u0192\\u02C6]");
      ss[ss.length] = word;
    }
    return ss;
  }
  
  this.AND = function(a, b)
  {
    var result=[];
    for(var ai=0; ai<a.length; ai++)
      if(this.findIndex(b, a[ai]) >= 0) result[result.length] = a[ai];
    return result;
  }
  
  this.OR = function(b, a)
  {
    var result=[];
    for(var bi=0; bi<b.length; bi++)
      result[result.length] = b[bi];
    for(var ai=0; ai<a.length; ai++)
      if(this.findIndex(result, a[ai]) < 0) result[result.length] = a[ai];
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
    if(moTop) {
      getFrame("content").location = "topics/" + page;
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
