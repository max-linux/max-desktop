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

function getTop(win) {
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
  var index = moTop.curPageIndex + x;
  if (index < 0 || index > moTop.ps.length-1) return;
  var f = getFrame("content");
  if(!f) return;
  moTop.curPageIndex = index;
  f.location = "topics/" + moTop.ps[index];
  if(moTop.contentsWin.ct) moTop.contentsWin.ct.locate(index);   
}

function initNavigate() {
  if (!moTop.curPageIndex) moTop.curPageIndex = -1;

  if (!moTop.contentsWin) {
    moTop.contentsWin = self;
    if (self.ps) {
      moTop.ps = self.ps;
      if (!moTop.pageInited) {
        moTop.pageInited = true;
        var p = moTop.location.href.match(/\?i=(\d+)$/);
        if(p) {
          moTop.curPageIndex = p[1]*1;
          stepPage(0);
          return;
        }
      }
    }
    stepPage(1);
  }
  else if (moTop.curPageIndex < 0) stepPage(1);
  else if (self.ct) self.ct.locate(moTop.curPageIndex);
}
