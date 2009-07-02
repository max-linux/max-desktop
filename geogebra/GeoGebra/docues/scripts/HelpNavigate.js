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

function getTop(win) {
  if (win.__mo_web_help_top__) return win;
  if (win == top) return top;
  if (win.parent != null) return getTop(win.parent);
}

function getFrame(name, frame)
{
  name = name.toLowerCase();
  if (!frame) frame = moTop;
  var frames = frame.frames;
  if(frames.length == 0) return null;

  var result = null;
  for(var i=0; i<frames.length; i++)
  {
    var f = frames[i];
    var n = null;
    try { n = f.name.toLowerCase(); } catch (e) { continue; }

    if(n == name) result = frames[i];
    else result = getFrame(name, f);
    if (result) return result;
  }
  return null;
}

var moTop = null;
if (!moTop) moTop = getTop(self);

function stepPage(x) {
  if (!moTop.ps) return;
  x = parseInt(x);
  var index = parseInt(moTop.curPageIndex) + x;
  if (index < 0 || index > moTop.ps.length-1) return;
  if(x < 0) goPage(index, -1)
  else goPage(index);
}

function findPageIndex(name) {
  if (!moTop.ps) return -1;
  name = decodeURIComponent(name);
  for(var i=0; i<moTop.ps.length; i++) {
    if (name == moTop.ps[i]) return i;
  }
  return -1;
}

function goPage(index, direction) {
  if (direction == null) direction = 1;

  var f = getFrame("content");
  if(!f) return;
  moTop.curPageIndex = index;
  if(moTop.ps[index] == '#' || moTop.ps[index] == 'javascript:void(0)') {
    stepPage(direction);
    return;
  }
  else {
    var href;
    if (f == window) href = moTop.ps[index];
    else href = "topics/" + moTop.ps[index];
    if (moTop.goBookmark) {
      href = href  + moTop.goBookmark;
      moTop.goBookmark = null;
    }
    f.location = href;
  }
  locate(index);
}

function locate(index) {
  if (!index.toString().match(/^\d+$/)) 
    index = findPageIndex(index);
  if (index < 0) return;

  if(moTop.ct) moTop.ct.locate(index);
  moTop.curPageIndex = index;
}

function initNavigate() {
  moTop.ct = self.ct;
  if(!moTop.ps && self.ps) moTop.ps = self.ps;
  if(!moTop.tl && self.tl) moTop.tl = self.tl;
  if(!moTop.ts && self.ts) moTop.ts = self.ts;

  if (!moTop.curPageIndex) moTop.curPageIndex = -1;

  if (!moTop.pageInited) {
    moTop.pageInited = true;
    if ( checkSearch() ) return;

    var bm = moTop.location.href.match(/#[^\/\\]+$/);
    if (bm) moTop.goBookmark = bm[0];
    var p = moTop.location.href.match(/\?i=(\d+)(#|$)/);
    if(p) {
      goPage( parseInt(p[1]) );
      return;
    }
    else {
      p = moTop.location.href.match(/\?n=([^?&#]+)(#|$)/);
      if(p) {
        goPage( findPageIndex(p[1]) );
        return;
      } 
    }
  }

  if (moTop.curPageIndex < 0) goPage(0);
  else locate(moTop.curPageIndex);
}

function printPage() {
  var p = getFrame('Content');
  if (!window.opera) {
    p.focus();
    p.print();
  }
  else {
    window.open(p.location + '?print');
  }
}

function checkPage(redir) {
  if (parent == self) {
    if (location.href.match(/\?print$/)) {
      if (!redir) {
        print();
        close();
      }
    }
    else if (redir) {
      var bm = self.location.href.match(/#[^\/\\]+$/);
      if (bm) { redir = redir + bm[0]; }
      location.href = redir;
    }
  }
}

function checkSearch() {
  var kw = moTop.location.href.match(/\?search=([^\/\\]+)$/);
  if (kw) {
    moTop.SearchKeyword = kw[1];
    var f = getFrame("left");
    if(!f) return false;
    f.location.href="HelpSearch.html";
    return true;
  }
  else return false;
}


function writeBreadcrumbs(index, start, spliter, right2left) {
  if (!moTop.tl || !moTop.ps || !moTop.ts || index < 0) return;
  var parent = getParent(index);
  if (parent == 0 && moTop.tl[parent] == moTop.tl[index]) parent = -1;
  if (!right2left) writeBreadcrumbs(parent, start, spliter, right2left);
  if (index != start && right2left) document.write(spliter);
  if (index == start || moTop.ps[index] == '#' || moTop.ps[index] == 'javascript:void(0)')
    document.write(moTop.ts[index]);
  else
    document.write('<a href="javascript:stepPage('+ (index - start) +')">'+ moTop.ts[index] +'</a>');
  if (index != start && !right2left) document.write(spliter);
  if (right2left) writeBreadcrumbs(parent, start, spliter, right2left);
}

function getParent(index) {
  var level = moTop.tl[index];
  while(moTop.tl[index] >= level && index > 0) index--;
  return index;
}