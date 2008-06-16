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

function moCssTree(ps, ts, tl, tn) {

  this.tn = tn;
  this.ps = ps;
  this.ts = ts;
  this.tl = tl;
  
  this.nodeTagName  = 'li';
  this.clsCollapsed = "Collapsed";
  this.clsExpanded  = "Expanded";
  this.clsTopic     = "Topic";
  this.clsCurrent   = "Current";
  this.emptyImg     = "images/~.gif";

  this.curNode   = null;
  this.panel     = null;

  this.getObject = function(id){
   return document.getElementById(id);
  }

  this.expand = function(node) {
    if(node.className == this.clsCollapsed) node.className = this.clsExpanded;
  }
  
  this.toggleNode = function(node) {
    node.className = node.className == this.clsCollapsed ? this.clsExpanded : this.clsCollapsed;
  }

  this.focus = function(node) {
    if ( this.curNode ) this.curNode.className = "";
    node.className = this.clsCurrent;
    this.curNode = node;
  }
  
  this.nodeClick = function(e) {
    var o = null;
    if (window.event) {
      e.cancelBubble = true;
      o = e.srcElement;
    }
    else if (e.stopPropagation) {
      e.stopPropagation();
      o = e.target;
      while(o.nodeType != o.ELEMENT_NODE) o = o.parentNode;
    }
    if (o.tagName.toLowerCase() == "a") {
      this.focus(o);
      if(moTop) moTop.curPageIndex = parseInt(o.getAttribute("mo:id", 2));
    } 
  }

  
  this.locate = function(index) {
    var nodes = this.panel.getElementsByTagName("a");
    if (index < 0 || index > nodes.length-1) return;
    var node = nodes[index];
    this.focus(node);

    while(node != this.panel) { 
      node = node.parentNode;
      if (node.tagName.toLowerCase() == this.nodeTagName) this.expand(node);
    }
  }

  this.createTree = function(panelId) {
    this.panel = this.getObject(panelId);
    var plevel = -1;
    for(var i=0; i<tl.length; i++) {
      var level  = tl[i];
      var nlevel = i+1 < tl.length ? tl[i+1] : -1;
      
      if(level > plevel) document.write("<UL>");
      var li = '<LI class="{class}" onclick="{onclick}">'
        + '<img src="images/o.gif" class="Icon" />'
        + '<A mo:id="{id}" href="topics/{href}" target="content">{title}</a>';
      li = li.replace("{class}",   level < nlevel ? this.clsCollapsed : this.clsTopic)
        .replace("{onclick}", level < nlevel ? "ct.toggleNode(this);ct.nodeClick(event);" : "ct.nodeClick(event);")
        .replace("{id}", i)
        .replace("{href}",  ps[i])
        .replace("{title}", tn[i] + " " + ts[i]);
      document.write(li);
      if(level == nlevel) document.write("</LI>");
      while(level > nlevel) {
        document.write("</LI></UL></LI>");
        nlevel++;
      }
      plevel = level;
    }    
  }
}

var ct = new moCssTree(ps, ts, tl, tn);
