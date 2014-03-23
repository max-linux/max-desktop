// evitar la denegación de ejecución de plugins no actualizados
// github #492 https://github.com/max-linux/max-desktop/issues/492
pref("extensions.blocklist.enabled",false);
// evitar que deniegue conexiones seguras via proxy a partir de la versión 27 de firefox
pref("security.tls.version.max",1);


// disable PDF internal plugin
// https://github.com/max-linux/max-desktop/issues/552
pref("pdfjs.disabled",true);
//pref("plugin.disable_full_page_plugin_for_types", "application/pdf");
