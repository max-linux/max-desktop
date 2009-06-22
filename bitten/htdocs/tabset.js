function makeTabSet(parentElement) {
  var tabList = document.createElement("ul");
  tabList.className = "tabs";
  var contentDivs = document.createElement("div");

  function makeTab(div) {
    var title = div.firstChild;
    while (title.nodeType != 1) title = title.nextSibling;
    var tabItem = document.createElement("li");
    if (!tabList.childNodes.length) tabItem.className = "active";
    var link = document.createElement("a");
    link.href = "#";
    link.appendChild(title.firstChild);
    tabItem.appendChild(link);

    var contentDiv = document.createElement("div");
    contentDiv.className = "tab-content";
    while (div.childNodes.length) contentDiv.appendChild(div.firstChild);
    if (tabList.childNodes.length) contentDiv.style.display = "none";

    link.onclick = function() {
      var child = contentDivs.firstChild;
      while (child) {
        if (child != contentDiv && child.nodeType == 1) {
          child.style.display = "none";
        }
        child = child.nextSibling;
      }
      var item = tabList.firstChild;
      while (item) {
        if (item.nodeType == 1) {
          item.className = item != tabItem ? "" : "active";
        }
        item = item.nextSibling;
      }
      contentDiv.style.display = "block";
      return false;
    }
    contentDivs.appendChild(contentDiv);
    tabList.appendChild(tabItem);
  }

  var divs = parentElement.getElementsByTagName("div");
  for (var i = 0; i < divs.length; i++) {
    var div = divs[i];
    if (!/\btab\b/.test(div.className)) {
      continue;
    }
    makeTab(div);
  }

  parentElement.appendChild(tabList);
  parentElement.appendChild(contentDivs);
}
