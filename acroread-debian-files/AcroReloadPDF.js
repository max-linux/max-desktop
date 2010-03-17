////////////////////////////////////////////////////////////////////////
//
// AcroReloadPDF.js, version 20090123
//
// * folder level JavaScript, to be put to:
//
//   $HOME/.adobe/Acrobat/8.0/JavaScripts
//   (version number might need to be adjusted)
//
// * adds item ``Reload'' to the ``File'' menu of Adobe Reader
// * reloads the current document and restores page number and zoom
//   state
//
// © Alexander Grahn, 2009
//
// This material is subject to the LaTeX Project Public License. See
//   http://www.ctan.org/tex-archive/help/Catalogue/licenses.lppl.html
// for the details of that license.
//
////////////////////////////////////////////////////////////////////////

reloadCurrentDoc=app.trustedFunction(function(currentDoc){
  app.beginPriv();
  var currentDocView=currentDoc.viewState;
  var currentDocPath=currentDoc.path;
  currentDoc.closeDoc();
  currentDoc=app.openDoc(currentDocPath);
  currentDoc.viewState=currentDocView;
  app.endPriv();
});

app.addMenuItem({
  cName:   "reloadCurDoc",
  cUser:   "Reloa&dD",
  cParent: "File",
  cExec:   "reloadCurrentDoc(event.target);",
  cEnable: "event.rc = (event.target != null);",
  nPos:    "Open",
  bPrepend: true
});
