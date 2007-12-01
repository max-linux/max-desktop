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

function hl(n, k)
{
  if(n.hasChildNodes)
  {
    for(var i=0; i<n.childNodes.length; i++)
    {
      if (hl(n.childNodes[i], k)) break;
    }
  }
  if((n.nodeType == 3) && n.nodeValue.match(k))
  {
    n.parentNode.innerHTML = n.parentNode.innerHTML.replace(k, '$1<span class="hl">$2</span>');
    return true;
  }
  return false;
}

function highlight()
{
  var s = moTop.RegexSearchKeyword;
  if (moTop.markKeywords) {
    moTop.markKeywords = false;
    hl(document.body, new RegExp('(</?[^>]+>)|('+s+')', 'gi'));
  }
}
